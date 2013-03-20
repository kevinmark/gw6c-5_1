# $Id: Makefile,v 1.18 2007/04/25 19:31:33 cnepveu Exp $
#
# This source code copyright (c) Hexago Inc. 2002-2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../../Mk/mk-$(target).mk

OBJDIR=../../objs
OBJ=$(OBJDIR)/tsp_local.o $(OBJDIR)/tsp_tun.o
OBJLINK=$(OBJDIR)/*.o
LDFLAGS=-L../../gw6cconfig -lcrypto -lpthread -lgw6cconfig -lstdc++
BIN=../../bin
TSPC=gw6c

CFLAGS=-O2 -Wall -I../.. -I../../include -I../../platform/$(target) -L$(CRYPTO_LIBDIR)
CC=$(CROSS_COMPILE)gcc

all: $(OBJ) $(wildcard $(OBJLINK)) $(BIN)/$(TSPC)

$(OBJDIR)/tsp_local.o:tsp_local.c
	$(CC) $(CFLAGS) -c tsp_local.c -o $(OBJDIR)/tsp_local.o $(DEFINES)
$(OBJDIR)/tsp_tun.o:tsp_tun.c
	$(CC) $(CFLAGS) -c tsp_tun.c -o $(OBJDIR)/tsp_tun.o $(DEFINES)

$(BIN)/$(TSPC): $(OBJ) $(wildcard $(OBJLINK))
	$(CC) $(CFLAGS) -o $(BIN)/$(TSPC) $(OBJLINK) $(LDFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(BIN)/$(TSPC)

install: all
