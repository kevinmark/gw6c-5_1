# $Id: Makefile,v 1.32 2007/05/23 16:46:11 cnepveu Exp $
#
#  Copyright (c) 2007 Hexago Inc. All rights reserved.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include Mk/mk-$(target).mk

GW6CMSG_DIR   :=$(PWD)/../gw6c-messaging
GW6CCFG_DIR   :=$(PWD)/../gw6c-config


all: pre-checks check-gw6cconfig check-gw6cmessaging
	@for dir in ${subdirs}; do \
	  (cd $$dir && $(MAKE) all) \
	  || case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

pre-checks:
	@if [ -z "$(target)" ]; then \
	    echo ; \
	    echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"; \
	    echo "You must supply the target type!"; \
	    echo "make all target=target-type"; \
	    echo ; \
	    echo "where target-type is one of the following:"; \
	    echo "   freebsd      for FreeBSD (NAT traversal support)."; \
	    echo "   linux        for Linux (NAT traversal support)."; \
	    echo "   netbsd       for NetBSD (No NAT support)."; \
            echo "   darwin       for Mac OS X darwin (NAT traversal support)."; \
	    echo "   solaris      for Solaris (No NAT support)."; \
	    echo "   openbsd      for OpenBSD (No NAT support)."; \
	    echo "   windows      (Deprecated, still supported)."; \
	    echo "   vstudio      for Windows XP/Vista (NAT traversal support)."; \
	    echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"; \
	    echo ; \
	    exit 1;\
	fi
	@if [ ! -f Mk/mk-$(target).mk ]; then \
	    echo "incomplete target setup for $(target) - no Mk/mk-$(target).mk"; exit 1;\
	fi
	@if [ ! "$(target)" = "vstudio" ]; then \
	  if [ ! -f template/$(target).sh -a ! -f template/$(target).bat ] ;\
	  then \
	      echo "incomplete target setup for $(target) - no template/$(target).sh"; exit 1;\
	  fi \
	fi
	mkdir -p bin
	mkdir -p objs
	
	
check-gw6cconfig:
	@if [ ! -d gw6cconfig -a ! "$(target)" = "vstudio" ] ;\
	then \
	    echo "Building gw6c-config module in ../gw6c-config/"; \
	    $(MAKE) -C ../gw6c-config target=$(target) exportdir=../tspc-advanced export ; \
	fi

check-gw6cmessaging:
	@if [ ! -d gw6cmessaging -a ! "$(target)" = "vstudio" ] ;\
	then \
	    echo "Building gw6c-messaging module in ../gw6c-messaging/"; \
	    $(MAKE) -C ../gw6c-messaging target=$(target) exportdir=../tspc-advanced export ; \
	fi

install: all
	@if [ -z "$(installdir)" ]; then \
	    echo ; \
	    echo "You must specify the install directory"; \
	    echo "ex.: make install target=os-type installdir=/usr/local/gw6c"; \
	    echo ; \
	    exit 1;\
	fi
	@INSTALL=cp
	mkdir -p $(installdir)
	@for dir in ${subdirs}; do \
	  (cd $$dir && $(MAKE) install) \
	  || case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

	@mkdir -p $(install_bin)
	@$(COPY) bin/$(TSPC) $(install_bin)
	@$(COPY) bin/gw6c.conf.sample $(install_bin)

	@if [ ! -f $(install_bin)/gw6c.conf ]; then \
		$(COPY) $(install_bin)/gw6c.conf.sample $(install_bin)/gw6c.conf; \
	fi

clean: pre-checks
	@for dir in ${subdirs}; do \
          (cd $$dir && $(MAKE) clean) \
          || case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
        done && test -z "$$fail"
	/bin/rm -rf bin/gw6c bin/gw6c.conf.sample objs gw6cmessaging gw6cconfig

cleanall: clean
	$(MAKE) -C ${GW6CMSG_DIR} clean
	$(MAKE) -C ${GW6CCFG_DIR} clean

