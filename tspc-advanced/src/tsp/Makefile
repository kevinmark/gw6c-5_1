# $Id: Makefile,v 1.21 2007/11/28 17:27:35 cnepveu Exp $
#
# Copyright (c) 2001-2007 Hexago Inc. All rights reserved.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../../Mk/mk-$(target).mk

OBJDIR=../../objs
OBJ=$(OBJDIR)/tsp_auth.o $(OBJDIR)/tsp_cap.o $(OBJDIR)/tsp_client.o $(OBJDIR)/tsp_net.o $(OBJDIR)/tsp_setup.o $(OBJDIR)/tsp_auth_passdss.o $(OBJDIR)/tsp_lease.o $(OBJDIR)/tsp_redirect.o $(OBJDIR)/tsp_tun_mgt.o
BIN=../bin

CFLAGS=-O2 -Wall -I../.. -I../../include -I../../platform/$(target) $(extraincludedir) -I$(OPENSSL_INCDIR)
CC=$(CROSS_COMPILE)gcc

all: $(OBJ)

$(OBJDIR)/tsp_auth.o:tsp_auth.c
	$(CC) $(CFLAGS) -c tsp_auth.c -o $(OBJDIR)/tsp_auth.o $(DEFINES)
$(OBJDIR)/tsp_cap.o:tsp_cap.c
	$(CC) $(CFLAGS) -c tsp_cap.c -o $(OBJDIR)/tsp_cap.o $(DEFINES)
$(OBJDIR)/tsp_client.o:tsp_client.c
	$(CC) $(CFLAGS) -c tsp_client.c -o $(OBJDIR)/tsp_client.o $(DEFINES)
$(OBJDIR)/tsp_net.o:tsp_net.c
	$(CC) $(CFLAGS) -c tsp_net.c -o $(OBJDIR)/tsp_net.o $(DEFINES)
$(OBJDIR)/tsp_setup.o:tsp_setup.c
	$(CC) $(CFLAGS) -c tsp_setup.c -o $(OBJDIR)/tsp_setup.o $(DEFINES)
$(OBJDIR)/tsp_auth_passdss.o:tsp_auth_passdss.c
	$(CC) $(CFLAGS) -c tsp_auth_passdss.c -o $(OBJDIR)/tsp_auth_passdss.o $(DEFINES)
$(OBJDIR)/tsp_lease.o:tsp_lease.c
	$(CC) $(CFLAGS) -c tsp_lease.c -o $(OBJDIR)/tsp_lease.o $(DEFINES)
$(OBJDIR)/tsp_redirect.o:tsp_redirect.c
	$(CC) $(CFLAGS) -c tsp_redirect.c -o $(OBJDIR)/tsp_redirect.o $(DEFINES)
$(OBJDIR)/tsp_tun_mgt.o:tsp_tun_mgt.c
	$(CC) $(CFLAGS) -c tsp_tun_mgt.c -o $(OBJDIR)/tsp_tun_mgt.o $(DEFINES)

clean:
	rm -f $(OBJDIR)/tsp_auth.o $(OBJDIR)/tsp_cap.o $(OBJDIR)/tsp_client.o $(OBJDIR)/tsp_net.o $(OBJDIR)/tsp_setup.o $(OBJDIR)/tsp_lease.o $(OBJDIR)/tsp_redirect.o $(OBJDIR)/tsp_tun_mgt.o

install: all

