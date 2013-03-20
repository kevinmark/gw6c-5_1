# $Id: mk-openwrt.mk,v 1.4 2007/05/22 16:31:55 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
# Gumstix particulars.
#

DEFINES=-Dlinux -DNO_OPENSSL
COPY=cp
TSPC=gw6c
install_bin=$(installdir)/bin
install_template=$(installdir)/template
install_man=$(installdir)/man
subdirs=src/net src/lib src/tsp src/xml platform/unix-common platform/openwrt template conf man 
ifname=sit1
ifname_tun=tun
ifname_v4v6=sit0
conf_template=openwrt
tsp_dir=/
