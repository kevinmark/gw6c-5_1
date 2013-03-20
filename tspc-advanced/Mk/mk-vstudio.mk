# $Id: mk-vstudio.mk,v 1.3 2007/06/14 14:21:06 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code
#

subdirs=conf template
ifname=2
ifname_tun=tunv6
ifname_v4v6=hextun
conf_template=windows
template=windows.cmd
install_template=$(installdir)/template

# ALL SOURCE IS BUILT FROM Visual Studio
