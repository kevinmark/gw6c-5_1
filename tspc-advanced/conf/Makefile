# $Id: Makefile,v 1.14 2007/05/22 16:31:57 cnepveu Exp $
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../Mk/mk-$(target).mk

BIN=../bin

all: gw6c.conf.sample 

.PHONY: gw6c.conf.sample 
gw6c.conf.sample:
	@echo Generating basic configuration file
	@if [ -f $(BIN)/gw6c.conf.sample ]; then \
	   rm -f $(BIN)/gw6c.conf.sample; \
	fi
	@if [ -z "$(configdir)" ]; then \
	 if [ -z "$(installdir)" ]; then \
	   sed -e "s+@ifname_v4v6@+$(ifname_v4v6)+" -e "s+@ifname_tun@+$(ifname_tun)+" -e "s+@ifname@+$(ifname)+" -e "s+@conf_template@+$(conf_template)+" -e "s+@conf_gw6_dir@+`cd ..;pwd`+"  gw6c.conf.in>$(BIN)/gw6c.conf.sample; \
	 else \
	   sed -e "s+@ifname_v4v6@+$(ifname_v4v6)+" -e "s+@ifname_tun@+$(ifname_tun)+" -e "s+@ifname@+$(ifname)+" -e "s+@conf_template@+$(conf_template)+" -e "s+@conf_gw6_dir@+$(installdir)+" gw6c.conf.in >$(BIN)/gw6c.conf.sample; \
	 fi \
	else \
	 sed -e "s+@ifname_v4v6@+$(ifname_v4v6)+" -e "s+@ifname_tun@+$(ifname_tun)+" -e "s+@ifname@+$(ifname)+" -e "s+@conf_template@+$(conf_template)+" -e "s+@conf_gw6_dir@+$(configdir)+" gw6c.conf.in >$(BIN)/gw6c.conf.sample; \
	fi
	chmod 700 $(BIN)/gw6c.conf.sample

clean:
	rm -f $(BIN)/gw6c.conf.sample

install: all
