#!/bin/sh -x
#
# $Id: openbsd.sh,v 1.11 2007/12/14 19:07:11 cnepveu Exp $
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

if [ -z $TSP_VERBOSE ]; then
   TSP_VERBOSE=0
fi

KillProcess()
{
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo killing $*
      fi
   fi
   PID=`ps axww | grep $1 | grep -v grep | awk '{ print $1;}'`
   if [ ! -z "${PID}" ]; then
      kill $PID
   fi
}

Display()
{
   if [ -z $TSP_VERBOSE ]; then
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
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo $*
      fi
   fi
   $* # Execute command
   if [ $? -ne 0 ]; then
      echo "Error while executing $1"
      echo "   Command: $*"
      exit 1
   fi
}

ExecNoCheck()
{
   if [ ! -z $TSP_VERBOSE ]; then
      if [ $TSP_VERBOSE -ge 2 ]; then
         echo $*
      fi
   fi
   $* # Execute command
}

# Programs localization

Display 1 "--- Start of configuration script. ---"
Display 1 "Script: " `basename $0`

ifconfig=/sbin/ifconfig
route=/sbin/route
rtadvd=/usr/sbin/rtadvd
sysctl=/usr/sbin/sysctl

if [ -z $TSP_HOME_DIR ]; then
   echo "TSP_HOME_DIR variable not specified!;"
   exit 1
fi

if [ ! -d $TSP_HOME_DIR ]; then
   echo "Error : directory $TSP_HOME_DIR does not exist"
   exit 1
fi
#

if [ -z $TSP_HOST_TYPE ]; then
   echo Error: TSP_HOST_TYPE not defined.
   exit 1
fi

#################################
# Run tunnel destruction script.
#################################
if [ X"${TSP_OPERATION}" = X"TSP_TUNNEL_TEARDOWN" ]; then

  Display 1 Tunnel tear down starting...


  # Router deconfiguration.
  if [ X"${TSP_HOST_TYPE}" = X"router" ]; then

    # Kill router advertisement daemon
    KillProcess rtadvd
    
    # Remove IPv6 configuration of home interface (PREFIX::1).
    ExecNoCheck $ifconfig $TSP_HOME_INTERFACE inet6 $TSP_PREFIX::1 delete

    # Remove blackhole
    if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
      ExecNoCheck $route delete -net -inet6 $TSP_PREFIX:: -prefixlen $TSP_PREFIXLEN ::1
    fi
  fi

  # Delete default IPv6 route
  ExecNoCheck $route delete -inet6 default

  # Delete tunnel interface IPv6 configuration
  list=`$ifconfig $TSP_TUNNEL_INTERFACE | grep inet6 | awk '{print $2}' | grep -v '^fe80'`
  for ipv6address in $list
  do 
    ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE inet6 $ipv6address delete
  done

  # Deconfiguration of tunnel
  ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE deletetunnel
  
  # Try to destroy the tunnel interface, if possible.
  ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE destroy


  Display 1 Tunnel tear down completed.

  exit 0
fi

##########################
# Tunnel creation script.
##########################
if [ X"${TSP_HOST_TYPE}" = X"host" ] || [ X"${TSP_HOST_TYPE}" = X"router" ]; then
   #
   # Configured tunnel config (IPv4)
   Display 1 Setting up interface $TSP_TUNNEL_INTERFACE
# Delete first the any previous tunnel

   ExecNoCheck $ifconfig $TSP_TUNNEL_INTERFACE deletetunnel
   Exec $ifconfig $TSP_TUNNEL_INTERFACE giftunnel $TSP_CLIENT_ADDRESS_IPV4 $TSP_SERVER_ADDRESS_IPV4

   #
   # Configured tunnel config (IPv6)

   # Check if the interface already has an IPv6 configuration
   list=`$ifconfig $TSP_TUNNEL_INTERFACE | grep inet6 | awk '{print $2}' | grep -v '^fe80'`
   for ipv6address in $list
   do 
     Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 $ipv6address delete
   done

   Exec $ifconfig $TSP_TUNNEL_INTERFACE inet6 $TSP_CLIENT_ADDRESS_IPV6 $TSP_SERVER_ADDRESS_IPV6 prefixlen $TSP_TUNNEL_PREFIXLEN alias

   #
   # Default route
   Display 1 Adding default route to $TSP_SERVER_ADDRESS_IPV6

   # Delete first any default IPv6 route, and set the new one.
   ExecNoCheck $route delete -inet6 default
   Exec $route add -inet6 default $TSP_SERVER_ADDRESS_IPV6
fi

# Router configuration if required
if [ X"${TSP_HOST_TYPE}" = X"router" ]; then

  Display 1 "Router configuration"

  # Kernel configuration. Allow IPv6 forwarding.
  Exec $sysctl -w net.inet6.ip6.forwarding=1
  Exec $sysctl -w net.inet6.ip6.accept_rtadv=0   # routed must disable any router advertisement incoming

  # Blackhole the remaining prefix portion that is not routed.
  if [ X"${TSP_PREFIXLEN}" != X"64" ]; then
    ExecNoCheck $route add -net -inet6 $TSP_PREFIX:: -prefixlen $TSP_PREFIXLEN ::1
  fi

  # Add prefix::1 inet6 address to advertising interface.
  Exec $ifconfig $TSP_HOME_INTERFACE inet6 $TSP_PREFIX::1 prefixlen 64 alias

  # Router advertisement startup
  KillProcess rtadvd
  Exec $rtadvd $TSP_HOME_INTERFACE
fi

Display 1 "--- End of configuration script. ---"

exit 0
