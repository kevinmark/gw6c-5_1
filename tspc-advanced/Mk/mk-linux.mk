# $Id: mk-linux.mk,v 1.10 2007/05/22 16:31:54 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2006.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
# Linux particulars.
#

DEFINES=-Dlinux
COPY=cp
TSPC=gw6c
install_bin=$(installdir)/bin
install_template=$(installdir)/template
install_man=$(installdir)/man
subdirs=src/net src/lib src/tsp src/xml platform/unix-common platform/linux template conf man 
ifname=sit1
ifname_tun=tun
ifname_v4v6=sit0
conf_template=linux
