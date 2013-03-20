// **************************************************************************
// $Id: servent.cc,v 1.10 2007/06/22 18:34:42 cnepveu Exp $
//
// Copyright (c) 2007 Hexago Inc. All rights reserved.
// 
//   LICENSE NOTICE: You may use and modify this source code only if you
//   have executed a valid license agreement with Hexago Inc. granting
//   you the right to do so, the said license agreement governing such
//   use and modifications.   Copyright or other intellectual property
//   notices are not to be removed from the source code.
//
// Description:
//   Implementation of the Servent class.
//
// Author: Charles Nepveu
//
// Creation Date: November 2006
// __________________________________________________________________________
// **************************************************************************
#include <gw6cmessaging/servent.h>
#include <gw6cmessaging/debugdefs.h>
#include <stddef.h>
#include <string.h>


#ifdef WIN32
#include <windows.h>
#define SLEEP(X)    Sleep(X)              // Milliseconds
#else
#include <unistd.h>
#define SLEEP(X)    usleep((X) * 1000)    // Microseconds
#endif

#define MUTEX_TIMEOUT     15000           // 15 seconds.

typedef unsigned char   BYTE;
typedef unsigned short  WORD;


// --------------------------------------------------------------------------
// Packet:
//   Packets are the raw data units sent through this IPC implementation.
//   They are formed up of a header, and the user data.
//   This implementation keeps the size of individual packets under 1024.
//
//   The maximum of user data transferable is 63 * 1020 = 64260 bytes.
//   - 63 possible fragments of user data.
//   - 1020 bytes of user data per packet MAX.
//
struct PACKET_HEADER          // 32 bit packet header.
{
  BYTE        _header  : 4;   // Packet header: fixed value: 1010
  BYTE        _msgtype : 6;   // Message type: 0x00 to 0x3F
  BYTE        _seq     : 6;   // Sequence number: 0-63
  BYTE        _fragseq : 6;   // Fragment sequence number: 0-63
  WORD        _datalen :10;   // User data length: 0-1023 (1020)
};

struct PACKET                 // Maximum length of packet: 1024 bytes.
{
  PACKET_HEADER  _header;     // 32 bit packet header.
  BYTE           _data[0];    // Data label.
};

#define PKT_HEADER_FIXEDVALUE         0x0A    // Hexa for binary 1010.
// Values 0x00-0x1F are RESERVED.
#define PKT_MSGTYPE_NONFRAGDATA       0x20    // Non-fragmented data
#define PKT_MSGTYPE_FRAGDATAFIRST     0x21    // Data, first fragment
#define PKT_MSGTYPE_FRAGDATA          0x22    // Data, fragment
#define PKT_MSGTYPE_FRAGDATALAST      0x23    // Data, last fragment
// Values 0x24-0x3F for future protocol expansion.

// User data max length(2^10): limited by PACKET_HEADER::_datalen
#define PKT_USERDATA_MAX              1020    // Maximum user data per packet
#define MAX_USERDATA                  64260

// Prototypes
size_t MakePacket( PACKET** ppPkt, const void* pvData, const WORD nDataSize, 
                   int eMsgType, WORD nSeqNum, WORD nFragSeqNum );
size_t ExtractPacketData( const PACKET* pPkt, BYTE* pUserData, int* eMsgType,
                          WORD* nSeqNum, WORD* nFragSeqNum );
void FreePacket( PACKET** pPkt );
void PrintPacket( const PACKET* pPkt );       // DEBUG ONLY.

// --------------------------------------------------------------------------


namespace gw6cmessaging
{
// --------------------------------------------------------------------------
// Function : Servent constructor
//
// Description:
//   Will initialize a new Servent object.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
Servent::Servent( void ) :
  m_pIPCServent(NULL),
  m_nTtlBytesRead(0),
  m_nTtlBytesWritten(0),
  m_pSemIPCMutex(NULL)
{
  // This mutex is created to serialize IO operations on the IPC Servent.
  // For thread safety.
  m_pSemIPCMutex = new Semaphore();
  assert( m_pSemIPCMutex != NULL );
}


// --------------------------------------------------------------------------
// Function : Servent destructor
//
// Description:
//   Will clean-up space allocated during object lifetime.
//
// Arguments: (none)
//
// Return values: (N/A)
//
// --------------------------------------------------------------------------
Servent::~Servent( void )
{
  if( m_pSemIPCMutex != NULL )
  {
    delete m_pSemIPCMutex;
    m_pSemIPCMutex = NULL;
  }
}


// --------------------------------------------------------------------------
// Function : Initialize
//
// Description:
//   Will initialize the IPCServent object referenced passed, and keep a copy
//   of that object for future use.
//
// Arguments: (none)
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   Any other value is an error.
//
// --------------------------------------------------------------------------
error_t Servent::Initialize( IPCServent* pIPCServent )
{
  error_t retCode;
  assert( pIPCServent != NULL );
  

  // Initialize the IPCServent.
  m_pIPCServent = pIPCServent;
  if( (retCode = m_pIPCServent->Initialize()) != GW6CM_UIS__NOERROR )
    m_pIPCServent = NULL;

  // Return initialization return code.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : WaitReady       [ Run in another thread ]
//
// Description:
//   This function will wait `ulWaitms' miliseconds, or until the
//   IPCServent signals the ready state.
//   Usually, the IPC Servent signal its ready state when it is ready
//   to accept connections(IPCServer) or connect to a server(IPCClient).
//
// Arguments:
//   ulWaitms: unsigned long [IN], The time in miliseconds to wait for the 
//                                 ready state to be signalled.
//
// Return values:
//   Returns true if the ready state is signalled before the timeout.
//
// --------------------------------------------------------------------------
bool Servent::WaitReady( unsigned long ulWaitms )
{
  unsigned long wait = 0;

  // Wait until the ipc servent is set (see Initialize).
  while( m_pIPCServent == NULL  &&  ( (wait < ulWaitms) || (ulWaitms==0))  )
  {
    SLEEP(25);
    wait += 25;
  }

  // Check on it.
  if( m_pIPCServent != NULL )
    return m_pIPCServent->WaitReady( ulWaitms - wait );

  return false;
}


// --------------------------------------------------------------------------
// Function : ReadData
//
// Description:
//   Will acquire IO mutex object and perform the read operation. 
//   Increments counter of bytes received.
//
// Arguments:
//   GW6CM_UIS_IOWAITTIMEOUT: Failed acquiring IO mutex. Might be another 
//   thread is doing a lengthy IO operation. Or a bug. 
//   (see _ReadData)
//
// Return values:
//   (see _ReadData)
//
// --------------------------------------------------------------------------
error_t Servent::ReadData( void* pvReadBuffer, const size_t nBufferSize, size_t& nRead )
{
  error_t retCode;

  // ---------------------------------------
  // Acquire mutex handle for IO operation.
  // ---------------------------------------
  if( m_pSemIPCMutex->WaitAndLock( MUTEX_TIMEOUT ) == 0 )
  {
    // Perform IO operation.
    retCode = _ReadData( pvReadBuffer, nBufferSize, nRead );

    // Release mutex.
    m_pSemIPCMutex->ReleaseLock();

    // Add the number of bytes received to the total.
    m_nTtlBytesRead += ( retCode == GW6CM_UIS__NOERROR ) ? nRead : 0;
  }
  else
    retCode = GW6CM_UIS_IOWAITTIMEOUT;

  return retCode;
}

// --------------------------------------------------------------------------
// Function : WriteData
//
// Description:
//   Will acquire IO mutex object and perform the write operation. 
//   Increments counter of bytes sent.
//
// Arguments:
//   (see _WriteData)
//
// Return values:
//   GW6CM_UIS_IOWAITTIMEOUT: Failed acquiring IO mutex. Might be another 
//   thread is doing a lengthy IO operation. Or a bug. 
//   (see _WriteData)
//
// --------------------------------------------------------------------------
error_t Servent::WriteData( const void* pvData, const size_t nDataSize, size_t& nWritten )
{
  error_t retCode;

  // ---------------------------------------
  // Acquire mutex handle for IO operation.
  // ---------------------------------------
  if( m_pSemIPCMutex->WaitAndLock( MUTEX_TIMEOUT ) == 0 )
  {
    // Perform IO operation.
    retCode = _WriteData( pvData, nDataSize, nWritten );

    // Release mutex.
    m_pSemIPCMutex->ReleaseLock();

    // Add the number of bytes sent to the total.
    m_nTtlBytesWritten += ( retCode == GW6CM_UIS__NOERROR ) ? nWritten : 0;
  }
  else
    retCode = GW6CM_UIS_IOWAITTIMEOUT;

  return retCode;
}


// --------------------------------------------------------------------------
// Function : CanRead
//
// Description:
//   Will verify if data is available for reception. This indicates that
//   a ReadData call should not block.
//
// Arguments:
//   bCanRead: boolean [OUT], Indicates if we can perform a non-blocking
//             read operation.
//
// Return values:
//   GW6CM_UIS__NOERROR: Indicates success.
//   Any other value is an error code.
//
// --------------------------------------------------------------------------
error_t Servent::CanRead( bool& bCanRead )
{
  error_t retCode = GW6CM_UIS__NOERROR;
  assert( m_pIPCServent );

  bCanRead = false;

  // ---------------------------------------
  // Acquire mutex handle for IO operation.
  // ---------------------------------------
  if( m_pSemIPCMutex->WaitAndLock( MUTEX_TIMEOUT ) == 0 )
  {
    // Delegate to the IPC layer.
    retCode = m_pIPCServent->CanRead( bCanRead );

    // Release mutex.
    m_pSemIPCMutex->ReleaseLock();
  }

  return retCode;
}


// --------------------------------------------------------------------------
// Function : CanWrite
//
// Description:
//   Will verify if a write operation will block or not.
//   NOTE: This function may not be reliable. Verify if read buffer is empty
//         as an alternative.
//
// Arguments:
//   bCanWrite: boolean [OUT], Indicates if we can perform a non-blocking
//              write operation.
//
// Return values:
//   GW6CM_UIS__NOERROR: Indicates success.
//   Any other value is an error code.
//
// --------------------------------------------------------------------------
error_t Servent::CanWrite( bool& bCanWrite )
{
  error_t retCode = GW6CM_UIS__NOERROR;
  assert( m_pIPCServent );

  bCanWrite = false;

  // ---------------------------------------
  // Acquire mutex handle for IO operation.
  // ---------------------------------------
  if( m_pSemIPCMutex->WaitAndLock( MUTEX_TIMEOUT ) == 0 )
  {
    // Delegate to the IPC layer.
    retCode = m_pIPCServent->CanWrite( bCanWrite );

    // Release mutex.
    m_pSemIPCMutex->ReleaseLock();
  }

  return retCode;
}


#define INVALID_SEQNUM 99
// --------------------------------------------------------------------------
// Function : _ReadData         [ PRIVATE ]
//
// Description:
//   Will attempt to read data from the IPC.
//   This function should not block if a Servent::CanRead() has been tested
//   before invoking this function.
//
// Arguments:
//   pvReadBuffer: void* [OUT], The receiving buffer.
//   nBufferSize: int [IN], The size in bytes allocated for read at the
//                receive buffer.
//   nRead: size_t [OUT], The number of bytes read from the IPC.
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   GW6CM_UIS_BADPACKET: Received bad data.
//   GW6CM_UIS_READBUFFERTOOSMALL: not enough buffer size to receive entire message.
//   GW6CM_UIS_PACKETSNOTORDERED: Message fragments are disordered.
//   GW6CM_UIS_IPCDESYNCHRONIZED: Major error. De-synchronization detected. Re-initiate connection.
//
// --------------------------------------------------------------------------
error_t Servent::_ReadData( void* pvReadBuffer, const size_t nBufferSize, size_t& nRead )
{
  size_t nBytesRead;                    // Number of bytes read from the IPC channel (packet length).
  size_t nDataLen;                      // User data length extracted from packet.
  WORD   nSeqNum, nLastSeqNum;          // Message sequence number: 0-63.
  WORD   nFragSeqNum, nLastFragSeqNum;  // Fragment sequence number: 0-63.
  int    nMessageType = 0;              // Message type (see packet info).
  BYTE   pRecvBuf[offsetof(PACKET, _data) + PKT_USERDATA_MAX];// Buffer for received packet.
  BYTE   pUDataBuf[PKT_USERDATA_MAX];   // User data extracted from packet.
  error_t retCode;                      // Return code.
  bool   bCanRead;                      // Indicates if read operation will block or not.


  assert( m_pIPCServent != NULL );


  // Initialize local variables.
  nLastFragSeqNum = nLastSeqNum = INVALID_SEQNUM;
  nRead = 0;    // Reset total message bytes counter.
  do
  {
    // --------------------------------------
    // Verify if data is available for read.
    // --------------------------------------
    if( (retCode = m_pIPCServent->CanRead( bCanRead )) != GW6CM_UIS__NOERROR )
    {
      // Error checking if we can read.
      return retCode;
    }
    if( !bCanRead )
    {
      SLEEP(33);       // Sleep 33 milliseconds.
      continue;
    }


    // Reset number of user data extracted from packet.
    nDataLen = 0;

    // ----------------------------------
    // Read packet from the IPC channel.
    // ----------------------------------
    retCode = m_pIPCServent->Read( (void*)pRecvBuf, sizeof(pRecvBuf), nBytesRead );
    if( retCode == GW6CM_UIS__NOERROR  &&  nBytesRead > 0 )
    {
      // --------------------------------------------------------------------
      // Extract user data from packet, and put it into temporary user data 
      // buffer.
      // --------------------------------------------------------------------
      nDataLen = ExtractPacketData( (PACKET*)pRecvBuf, pUDataBuf, &nMessageType, &nSeqNum, &nFragSeqNum );
      if( nDataLen == 0 )
      {
        // Bad packet or invalid data.
        return GW6CM_UIS_BADPACKET;
      }


      // ----------------------------------------------------
      // Verify that message sequence number hasn't changed.
      // ----------------------------------------------------
      if( nLastSeqNum != INVALID_SEQNUM  &&  nSeqNum != nLastSeqNum )
      {
        // Message sequence number change (meaning we've got another message).
        // This should NOT happen in this logic.
        // End of earlier message should have been detected before changing
        // message.
        // This is a serious error, as this packet data will be lost and
        // a de-synchronization will now occur forever...
        return GW6CM_UIS_IPCDESYNCHRONIZED;
      }
      else
        nLastSeqNum = nSeqNum;


      // ------------------------------------------------------------
      // Verify that message fragment sequence number is sequential.
      // ------------------------------------------------------------
      if( nLastFragSeqNum != INVALID_SEQNUM  &&  nFragSeqNum != (nLastFragSeqNum + 1) )
      {
        // Message fragments are not ordered.
        // ASSERTION: Pipe IPCs are assumed fifo. If using an IPC 
        //            other than pipe, correct this logic to allow
        //            packet fragment re-ordering.
        return GW6CM_UIS_PACKETSNOTORDERED;
      }
      else
        nLastFragSeqNum = nFragSeqNum;


      // --------------------------------------------------------------------
      // Verify that user data read won't overflow the provided read buffer.
      // --------------------------------------------------------------------
      if( (nRead + nDataLen) > nBufferSize )
      {
        // User did not provide enough buffer size to receive entire message.
        return GW6CM_UIS_READBUFFERTOOSMALL;
      }


      // -----------------------------------------
      // Append user data to the provided buffer.
      // -----------------------------------------
      memcpy( ((BYTE*)pvReadBuffer) + nRead, pUDataBuf, nDataLen );


      // Increase the number of user data bytes received.
      nRead += nDataLen;
    }

  }
  while( retCode == GW6CM_UIS__NOERROR  &&
         nMessageType != PKT_MSGTYPE_NONFRAGDATA  &&  
         nMessageType != PKT_MSGTYPE_FRAGDATALAST );


  // Read operation finished.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : _WriteData         [ PRIVATE ]
//
// Description:
//   Will send user data over IPC.
//   ATTENTION: This function WILL block if the outgoing buffer is full.
//              It will block until the other side of the pipe has read 
//              enough to free the write buffer.
//              In this implementation, you should wait until the read buffer
//              of this end of the pipe is empty, before trying a write
//              operation. (THREADS)
//
// Arguments:
//   pvData: void* [IN], The user data.
//   nDataSize: int [IN], The size in bytes of the user data.
//
// Return values:
//   GW6CM_UIS__NOERROR: Operation successful.
//   GW6CM_UIS_SENDBUFFERTOOBIG: user data is too big.
//
// --------------------------------------------------------------------------
error_t Servent::_WriteData( const void* pvData, const size_t nDataSize, size_t& nWritten )
{
  PACKET* pPkt = NULL;
  size_t nSent;                   // Number of bytes sent by the Write method. (this includes the packet header).
  size_t nToSend;                 // Number of bytes that will be sent in the next packet.
  size_t nPacketLen;              // Size of packet (user data + header).
  static WORD nSeqNum=0;          // Message sequence number: 0-63.
  WORD   nFragSeqNum;             // Fragment sequence number: 0-63.
  int    nMessageType;            // Message type (see packet info).
  error_t retCode=GW6CM_UIS__NOERROR; // Return code
  bool   bCanWrite;               // Indicates if a read operation is possible.


  assert( m_pIPCServent != NULL );
  // Simple check to verify that user data is not too big.
  if( nDataSize > MAX_USERDATA )
    return GW6CM_UIS_SENDBUFFERTOOBIG;


  nWritten = 0;                     // Reset total number of bytes sent.
  nFragSeqNum = 0;                  // Reset fragment sequence number.
  while( nWritten < nDataSize )
  {
    // ------------------------------------------
    // Check if we can write to the IPC channel.
    // ------------------------------------------
    if( (retCode = m_pIPCServent->CanWrite( bCanWrite )) != GW6CM_UIS__NOERROR )
    {
      // Error checking if we can write.
      return retCode;
    }
    if( !bCanWrite )
    {
      SLEEP(33);       // Sleep 33 milliseconds.
      continue;
    }
  

    // ------------------------------------------------------
    // Compute the size of user data to send in this packet.
    // ------------------------------------------------------
    nToSend = nDataSize - nWritten;
    if( nToSend > PKT_USERDATA_MAX )
    {
      // Need to fragment data.
      nToSend = PKT_USERDATA_MAX;
      nMessageType = (nFragSeqNum==0) ? PKT_MSGTYPE_FRAGDATAFIRST : PKT_MSGTYPE_FRAGDATA;
    }
    else
      nMessageType = (nFragSeqNum==0) ? PKT_MSGTYPE_NONFRAGDATA : PKT_MSGTYPE_FRAGDATALAST;


    // ----------------------------------------------
    // Build the packet (User data + Packet header).
    // ----------------------------------------------
    nPacketLen = MakePacket( &pPkt, (void*)(((BYTE*)pvData) + nWritten), nToSend, nMessageType, nSeqNum, nFragSeqNum );
    assert( pPkt != NULL );


    // ------------------------------------
    // Send the packet in the IPC channel.
    // ------------------------------------
    retCode = m_pIPCServent->Write( (void*)pPkt, nPacketLen, nSent );
    if( retCode != GW6CM_UIS__NOERROR  ||  nSent != nPacketLen )
    {
      // Error, OR
      // Failed to send entire data. Might be broken pipe.
      return retCode;
    }
    else
      nWritten += nToSend;    // Add the nToSend. NOT THE nSent (because that one contains packet header length).


    // ----------------------------------
    // Free the allocated packet memory.
    // ----------------------------------
    FreePacket( &pPkt );


    // ------------------------------------
    // Increment fragment sequence number.
    // ------------------------------------
    if( ++nFragSeqNum > 63 )
    {
      // Cannot send more than 63 packets (i.e.: message fragments).
      // Should never get here, because of initial check. assert here.
      assert(false);
      return GW6CM_UIS_SENDBUFFERTOOBIG;
    }
  }

  // ----------------------------------
  // Increment static message seq num.
  // ----------------------------------
  nSeqNum = (nSeqNum + 1) % 64;


  // Return last Write retCode.
  return retCode;
}


// --------------------------------------------------------------------------
// Function : GetServentInfo
//
// Description:
//   This function will fill up the passed SERVENT_INFO with information
//   on the running Servent object.
//
// Arguments:
//   pObj: SERVENT_INFO* [IN,OUT], A SERVENT_INFO object reference.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void Servent::GetServentInfo( PSERVENT_INFO pObj )
{
  assert( pObj != NULL );

  pObj->nTtlBytesRead    = m_nTtlBytesRead;
  pObj->nTtlBytesWritten = m_nTtlBytesWritten;
}

} // namespace


// --------------------------------------------------------------------------
// Function : MakePacket
//
// Description:
//   Will create a new packet.
//
// Arguments:
//   pPkt: PACKET* [OUT], A new allocated PACKET.
//
// Return values:
//   Size of new packet. (HEADER + USER DATA sizes).
//
// --------------------------------------------------------------------------
size_t MakePacket( PACKET** ppPkt, const void* pvData, const WORD nDataSize, 
                   int eMsgType, WORD nSeqNum, WORD nFragSeqNum )
{
  size_t nPacketSz;

  // Sanity checks.
  assert( nDataSize <= PKT_USERDATA_MAX );
  assert( *ppPkt == NULL );


  // Compute the size of the packet (Header + user data).
  nPacketSz = offsetof(PACKET, _data) + nDataSize;
  *ppPkt = (PACKET*) new BYTE[nPacketSz];


  // Make the packet header
  (*ppPkt)->_header._header  = PKT_HEADER_FIXEDVALUE;
  (*ppPkt)->_header._msgtype = eMsgType;
  (*ppPkt)->_header._seq     = nSeqNum;
  (*ppPkt)->_header._fragseq = nFragSeqNum;
  (*ppPkt)->_header._datalen = nDataSize;

  // Copy user data in packet.
  memcpy( (*ppPkt)->_data, pvData, nDataSize );

  // DEBUG:
  //  cout << "Make packet - ";
  //  PrintPacket( *ppPkt );

  // Return the total size of packet (header + user data length).
  return nPacketSz;
}


// --------------------------------------------------------------------------
// Function : ExtractPacketData
//
// Description:
//   Will extract user data from packet, and append it to the buffered
//   user data. Will make sanity checks before copying.
//
// Arguments:
//   pPkt: PACKET* [IN], A PACKET.
//   pUserData: BYTE* [IN,OUT], pointer to writeable user data.
//
// Return values:
//   size of user data extracted from packet.
//
// --------------------------------------------------------------------------
size_t ExtractPacketData( const PACKET* pPkt, BYTE* pUserData, int* eMsgType,
                          WORD* nSeqNum, WORD* nFragSeqNum )
{
  // DEBUG:
  //  cout << "Extract packet - ";
  //  PrintPacket( pPkt );

  // ---------------------------------------
  // Make sanity checks on received packet.
  // ---------------------------------------

  if( pPkt->_header._header != PKT_HEADER_FIXEDVALUE )
  {
    assert(false);
    return 0;
  }

  *eMsgType = pPkt->_header._msgtype;   
  if( *eMsgType < PKT_MSGTYPE_NONFRAGDATA  ||  *eMsgType > PKT_MSGTYPE_FRAGDATALAST )
  {
    assert( false );
    return 0;
  }

  *nSeqNum = pPkt->_header._seq;
  if( *nSeqNum > 63 )
  {
    assert( false );
    return 0;
  }

  *nFragSeqNum = pPkt->_header._fragseq;
  if( *nFragSeqNum > 63 )
  {
    assert( false );
    return 0;
  }

  if( pPkt->_header._datalen > PKT_USERDATA_MAX )
  {
    assert( false );
    return 0;
  }


  // Copy user data from packet to buffered user data.
  memcpy( (void*)pUserData, pPkt->_data, pPkt->_header._datalen );


  // Return the size of the user of data copied to the buffer.
  return pPkt->_header._datalen;
}


// --------------------------------------------------------------------------
// Function : FreePacket
//
// Description:
//   Will free a packet created by the MakePacket function.
//
// Arguments:
//   pPkt: PACKET* [IN], An allocated PACKET.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void FreePacket( PACKET** ppPkt )
{
  BYTE* pBytes = (BYTE*)(*ppPkt);

  assert( pBytes != NULL );
  delete [] pBytes;

  *ppPkt = NULL;
}


// --------------------------------------------------------------------------
// Function : PrintPacket       [ DEBUG ONLY }
//
// Description:
//   Will print a packet header and data.
//
// Arguments:
//   pPkt: PACKET* [IN], A valid PACKET.
//
// Return values: (none)
//
// --------------------------------------------------------------------------
void PrintPacket( const PACKET* pPkt )
{
  assert( pPkt != NULL );
  DBG_PRINT( "Printing packet..." );
  DBG_PRINT( "  Header         : 0x" << setbase(16) << (WORD) pPkt->_header._header );
  DBG_PRINT( "  Packet type    : 0x" << setbase(16) << (WORD) pPkt->_header._msgtype );
  DBG_PRINT( "  Packet Seq num : 0x" << setbase(16) << (WORD) pPkt->_header._seq );
  DBG_PRINT( "  Frag Seq num   : 0x" << setbase(16) << (WORD) pPkt->_header._fragseq );
  DBG_PRINT( "  User data len  : 0x" << setbase(16) << (WORD) pPkt->_header._datalen );
  DBG_PRINT( "  Message Data   : "   << pPkt->_data );
}
