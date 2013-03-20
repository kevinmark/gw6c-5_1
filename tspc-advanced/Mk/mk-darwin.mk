# $Id: mk-darwin.mk,v 1.9 2007/11/28 17:27:03 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2006.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
#  Mac OS X Darwin particulars.

DEFINES=
COPY=cp
TSPC=gw6c
install_bin=$(installdir)/bin
install_template=$(installdir)/template
install_man=$(installdir)/man
subdirs=src/net src/lib src/tsp src/xml platform/unix-common platform/darwin template conf man
ifname=gif0
ifname_tun=tun0
ifname_v4v6=
conf_template=darwin
