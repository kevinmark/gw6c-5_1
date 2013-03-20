/*
---------------------------------------------------------------------------
 $Id: net_tcp6.h,v 1.7 2007/05/23 19:19:28 cnepveu Exp $
---------------------------------------------------------------------------
* This source code copyright (c) Hexago Inc. 2002-2004,2007.
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License (GPL) Version 2, 
* June 1991 as published by the Free  Software Foundation.
* 
* This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY;  without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License 
* along with this program; see the file GPL_LICENSE.txt. If not, write 
* to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
* MA 02111-1307 USA
---------------------------------------------------------------------------
*/


#ifndef _NET_TCP6_H_
#define _NET_TCP6_H_

extern SOCKET   NetTCP6Connect   (char *, unsigned short);
extern int      NetTCP6Close     (SOCKET);

extern int      NetTCP6ReadWrite (SOCKET, char *, int, char *, int);

extern int      NetTCP6Write     (SOCKET, char *, int);
extern int      NetTCP6Printf    (SOCKET, char *, int, char *, ...);

extern int      NetTCP6Read      (SOCKET, char *, int);

#endif
