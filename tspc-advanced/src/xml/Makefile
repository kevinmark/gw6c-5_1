# $Id: Makefile,v 1.13 2007/04/25 19:31:53 cnepveu Exp $
#
# Copyright (c) 2001-2005,2007 Hexago Inc. All rights reserved.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../../Mk/mk-$(target).mk

OBJDIR=../../objs
OBJ=$(OBJDIR)/xmlparse.o $(OBJDIR)/xml_req.o $(OBJDIR)/xml_tun.o
BIN=../bin

CFLAGS=-O2 -Wall -I../../include -I../../platform/$(target)
CC=$(CROSS_COMPILE)gcc

all: $(OBJ) 

$(OBJDIR)/xmlparse.o:xmlparse.c
	$(CC) $(CFLAGS) -c xmlparse.c -o $(OBJDIR)/xmlparse.o $(DEFINES)
$(OBJDIR)/xml_req.o:xml_req.c
	$(CC) $(CFLAGS) -c xml_req.c -o $(OBJDIR)/xml_req.o $(DEFINES)
$(OBJDIR)/xml_tun.o:xml_tun.c
	$(CC) $(CFLAGS) -c xml_tun.c -o $(OBJDIR)/xml_tun.o $(DEFINES)

clean:
	rm -f $(OBJDIR)/xmlparse.o $(OBJDIR)/xml_req.o $(OBJDIR)/xml_tun.o

install: all
