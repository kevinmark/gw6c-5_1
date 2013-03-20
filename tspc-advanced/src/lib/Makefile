# $Id: Makefile,v 1.17 2007/04/25 19:31:45 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2005,2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../../Mk/mk-$(target).mk

OBJDIR=../../objs
OBJ=$(OBJDIR)/base64.o $(OBJDIR)/cli.o $(OBJDIR)/config.o $(OBJDIR)/lib.o $(OBJDIR)/log.o $(OBJDIR)/md5c.o $(OBJDIR)/cnfchk.o $(OBJDIR)/buffer.o $(OBJDIR)/bufaux.o $(OBJDIR)/version.o $(OBJDIR)/dns.o $(OBJDIR)/deque.o
HAP6_OBJ=$(OBJDIR)/hap6.o
BIN=../bin

CFLAGS=-O2 -Wall -I../.. -I../../include -I../../platform/$(target) $(extraincludedir) -I$(OPENSSL_INCDIR)
CC=$(CROSS_COMPILE)gcc

all: $(OBJ) $(HAP6_OBJ)

$(OBJDIR)/base64.o:base64.c
	$(CC) $(CFLAGS) -c base64.c -o $(OBJDIR)/base64.o $(DEFINES)
$(OBJDIR)/cli.o:cli.c
	$(CC) $(CFLAGS) -c cli.c -o $(OBJDIR)/cli.o $(DEFINES)
$(OBJDIR)/config.o:config.c
	$(CC) $(CFLAGS) -c config.c -o $(OBJDIR)/config.o $(DEFINES)
$(OBJDIR)/lib.o:lib.c
	$(CC) $(CFLAGS) -c lib.c -o $(OBJDIR)/lib.o $(DEFINES)
$(OBJDIR)/log.o:log.c
	$(CC) $(CFLAGS) -c log.c -o $(OBJDIR)/log.o $(DEFINES)
$(OBJDIR)/md5c.o:md5c.c
	$(CC) $(CFLAGS) -c md5c.c -o $(OBJDIR)/md5c.o $(DEFINES)
$(OBJDIR)/cnfchk.o:cnfchk.c
	$(CC) $(CFLAGS) -c cnfchk.c -o $(OBJDIR)/cnfchk.o $(DEFINES)
$(OBJDIR)/buffer.o:buffer.c
	$(CC) $(CFLAGS) -c buffer.c -o $(OBJDIR)/buffer.o $(DEFINES)
$(OBJDIR)/bufaux.o:bufaux.c
	$(CC) $(CFLAGS) -c bufaux.c -o $(OBJDIR)/bufaux.o $(DEFINES)
$(OBJDIR)/version.o:version.c
	$(CC) $(CFLAGS) -c version.c -o $(OBJDIR)/version.o $(DEFINES)
$(OBJDIR)/dns.o:dns.c
	$(CC) $(CFLAGS) -c dns.c -o $(OBJDIR)/dns.o $(DEFINES)
$(OBJDIR)/deque.o:deque.c
	$(CC) $(CFLAGS) -c deque.c -o $(OBJDIR)/deque.o $(DEFINES)


$(OBJDIR)/hap6.o:
	@if [ ! -z "$(HAP6_DEFINES)" ] && [ -f hap6.c ]; then \
		echo $(CC) $(CFLAGS) -c hap6.c -o $(OBJDIR)/hap6.o $(DEFINES) ;\
		$(CC) $(CFLAGS) -c hap6.c -o $(OBJDIR)/hap6.o $(DEFINES) ;\
	fi

clean:
	rm -f $(OBJDIR)/base64.o $(OBJDIR)/cli.o $(OBJDIR)/config.o $(OBJDIR)/lib.o $(OBJDIR)/log.o $(OBJDIR)/md5c.o $(OBJDIR)/cnfchk.o $(OBJDIR)/buffer.o $(OBJDIR)/bufaux.o $(OBJDIR)/version.o $(OBJDIR)/dns.o $(OBJDIR)/deque.o
	rm -f $(HAP6_OBJ)

install: all
