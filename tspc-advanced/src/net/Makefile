# $Id: Makefile,v 1.21 2007/04/25 19:31:48 cnepveu Exp $
#
# Copyright (c) 2001-2007 Hexago Inc. All rights reserved
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../../Mk/mk-$(target).mk

OBJDIR=../../objs
OBJ=$(OBJDIR)/net.o $(OBJDIR)/net_rudp.o $(OBJDIR)/net_rudp6.o $(OBJDIR)/net_tcp.o $(OBJDIR)/net_udp.o $(OBJDIR)/net_ka.o $(OBJDIR)/net_cksm.o $(OBJDIR)/net_tcp6.o $(OBJDIR)/net_echo_request.o
BIN=../bin

CFLAGS=-O2 -Wall -I../../include -I../../platform/$(target)
CC=$(CROSS_COMPILE)gcc

all: $(OBJ) 

$(OBJDIR)/net.o:net.c
	$(CC) $(CFLAGS) -c net.c -o $(OBJDIR)/net.o $(DEFINES)
$(OBJDIR)/net_rudp.o:net_rudp.c
	$(CC) $(CFLAGS) -c net_rudp.c -o $(OBJDIR)/net_rudp.o $(DEFINES)
$(OBJDIR)/net_tcp.o:net_tcp.c
	$(CC) $(CFLAGS) -c net_tcp.c -o $(OBJDIR)/net_tcp.o $(DEFINES)
$(OBJDIR)/net_udp.o:net_udp.c
	$(CC) $(CFLAGS) -c net_udp.c -o $(OBJDIR)/net_udp.o $(DEFINES)
$(OBJDIR)/net_ka.o:net_ka.c
	$(CC) $(CFLAGS) -c net_ka.c -o $(OBJDIR)/net_ka.o $(DEFINES)
$(OBJDIR)/net_cksm.o:net_cksm.c
	$(CC) $(CFLAGS) -c net_cksm.c -o $(OBJDIR)/net_cksm.o $(DEFINES)
$(OBJDIR)/net_tcp6.o:net_tcp6.c
	$(CC) $(CFLAGS) -c net_tcp6.c -o $(OBJDIR)/net_tcp6.o $(DEFINES)
$(OBJDIR)/net_rudp6.o:net_rudp6.c
	$(CC) $(CFLAGS) -c net_rudp6.c -o $(OBJDIR)/net_rudp6.o $(DEFINES)
$(OBJDIR)/net_echo_request.o:net_echo_request.c
	$(CC) $(CFLAGS) -c net_echo_request.c -o $(OBJDIR)/net_echo_request.o $(DEFINES)

clean:
	rm -f $(OBJDIR)/net.o $(OBJDIR)/net_rudp.o $(OBJDIR)/net_rudp6.o $(OBJDIR)/net_tcp.o $(OBJDIR)/net_udp.o $(OBJDIR)/net_ka.o $(OBJDIR)/net_cksm.o $(OBJDIR)/net_tcp6.o $(OBJDIR)/net_echo_request.o 

install: all

