# $Id: Makefile,v 1.5 2007/05/11 20:09:15 cnepveu Exp $
#
#  Copyright (c) 2007 Hexago Inc. All rights reserved.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../Mk/mk-$(target).mk

all:

install: all
	@echo Installing man pages
	mkdir -p $(install_man)/man5
	mkdir -p $(install_man)/man8
	$(COPY) man5/gw6c.conf.5 $(install_man)/man5
	$(COPY) man8/gw6c.8 $(install_man)/man8
	@echo To view man pages run :
	@echo    man -M $(install_man) gw6c
	@echo    man -M $(install_man) gw6c.conf

clean: 
	@echo
