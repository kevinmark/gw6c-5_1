#!/bin/sh
#
# $Id: solaris.sh,v 1.9 2007/11/28 17:27:38 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2005.
#
# LICENSE NOTICE: You may use and modify this source code only if you
# have executed a valid license agreement with Hexago Inc. granting
# you the right to do so, the said license agreement governing such
# use and modifications.   Copyright or other intellectual property
# notices are not to be removed from the source code.
#

LANGUAGE=C

if [ -z "$TSP_VERBOSE" ]; then
   TSP_VERBOSE=0
fi

KillProcess()
{
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo killing $*
      fi
   fi
   /bin/pkill $1
}

Display()
{
   if [ -z "$TSP_VERBOSE" ]; then
      return;
   fi
   if [ $TSP_VERBOSE -lt $1 ]; then
      return;
   fi
   shift
   echo $*
}

Exec()
{
   if [ ! -z "$TSP_VERBOSE" ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo $*
      fi
   fi
   $*
   ret=$?
   if [ $ret -ne 0 ]; then
      echo "Error while executing $1"
      echo "   Command: $*"
      exit 1
   fi
   echo "Command $1 succeed"
}

# Program localization 

Display 1 "--- Start of configuration script. ---"
Display 1 "Script: " `basename $0`

ifconfig=/sbin/ifconfig
route=/usr/sbin/route
inetinit=/etc/init.d/inetinit
ndpd=/usr/lib/inet/in.ndpd
ndd=/usr/sbin/ndd
ripngd=/usr/lib/inet/in.ripngd

ndpdconfigfilename=ndpd.conf
ndpdconfigfile=$TSP_HOME_DIR/$ndpdconfigfilename

if [ -z "$TSP_HOME_DIR" ]; then
   echo "TSP_HOME_DIR variable not specified!;"
   exit 1
fi

if [ ! -d $TSP_HOME_DIR ]; then
   echo "Error : directory $TSP_HOME_DIR does not exist"
   exit 1
fi
#

if [ -z "$TSP_HOST_TYPE" ]; then
   echo Error: TSP_HOST_TYPE not defined.
   exit 1
fi


#################################
# Run tunnel destruction script.
#################################
if [ X"${TSP_OPERATION}" = X"TSP_TUNNEL_TEARDOWN" ]; then

  Display 1 Tunnel tear down starting...

  # Unplumb and disable tunnel interface
  $ifconfig $TSP_TUNNEL_INTERFACE inet6 unplumb 2>/dev/null

  # Remove INET6 default route
  $route delete -inet6 default $TSP_SERVER_ADDRESS_IPV6 2>/dev/null


  # Disable neighbor discovery.
  if [ X"${TSP_HOST_TYPE}" = X"router" ]; then
    if [ -f /usr/lib/inet/in.ndpd ]; then
      KillProcess $ndpd
    fi
  fi

  Display 1 Tunnel tear down completed.

  exit 0
fi


##########################
# Tunnel creation script.
##########################
if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then
   #
   # Configured tunnel config (IPv6) 
   Display 1 Setting up interface $TSP_TUNNEL_INTERFACE
   $ifconfig $TSP_TUNNEL_INTERFACE inet6 unplumb 2>/dev/null
   Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 plumb 

   Display 1 Configuring IPv4 addresses
   Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 tsrc $TSP_CLIENT_ADDRESS_IPV4 tdst $TSP_SERVER_ADDRESS_IPV4 up
   Display 1 Configuring IPv6 addresses
   Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 addif $TSP_CLIENT_ADDRESS_IPV6 $TSP_SERVER_ADDRESS_IPV6 up 
   # Default route 
   Display 1 Configuring route 
   $route delete -inet6 default $TSP_SERVER_ADDRESS_IPV6 2>/dev/null
   Display 1 "Adding default route"
   Exec $route add -inet6 default $TSP_SERVER_ADDRESS_IPV6
fi

# Host only configuration
if [ X"${TSP_HOST_TYPE}" = X"host" ]; then
   Display 1 "Host configuration"
   Exec $ndd -set /dev/ip ip6_forwarding 0
   Exec $ndd -set /dev/ip ip6_send_redirects 0
   Exec $ndd -set /dev/ip ip6_ignore_redirect 0
fi

# Router configuration if required
if [ X"${TSP_HOST_TYPE}" = X"router" ]; then
   Display 1 "Router configuration"
   Display 1 "Kernel setup"
   Exec $ndd -set /dev/ip ip6_forwarding 1
   Exec $ndd -set /dev/ip ip6_send_redirects 1
   Exec $ndd -set /dev/ip ip6_ignore_redirect 1
   if [ -f /usr/lib/inet/in.ndpd ]; then
     Display 1 Creating new $ndpdconfigfile
     echo "##### ndpd.conf made by Gateway6 Client ####" > "$ndpdconfigfile"
     echo "ifdefault AdvSendAdvertisements true" >> "$ndpdconfigfile"
     echo "prefix $TSP_PREFIX::/64 $TSP_HOME_INTERFACE" >> "$ndpdconfigfile"
     if [ -f $ndpdconfigfile ]; then
       Display "Starting IPv6 neighbor discovery."
       KillProcess $ndpd
       Exec $ndpd -f $ndpdconfigfile
     else
       echo "Error : file $ndpdconfigfile not found"
       exit 1
     fi
   fi
   Display 1 Configuring address $TSP_PREFIX::1 on interface $TSP_HOME_INTERFACE
   Exec $ifconfig $TSP_HOME_INTERFACE inet6 addif $TSP_PREFIX::1/64 up 
fi

Display 1 "--- End of configuration script. ---"

exit 0
