# $Id: Makefile,v 1.16 2007/06/05 15:46:38 cnepveu Exp $
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
OBJ=$(OBJDIR)/tsp_local.o
OBJLINK=$(OBJDIR)/*.o
LDFLAGS=-L../../gw6cconfig -lcrypto -lpthread -lgw6cconfig -lstdc++ -lm
BIN=../../bin
TSPC=gw6c

CFLAGS=-O2 -Wall -I../.. -I../../include -I../../platform/$(target)
CC=gcc

all: $(OBJ) $(OBJLINK) $(BIN)/$(TSPC)

$(OBJDIR)/tsp_local.o:tsp_local.c
	$(CC) $(CFLAGS) -c tsp_local.c -o $(OBJDIR)/tsp_local.o $(DEFINES)

$(BIN)/$(TSPC): $(OBJ) $(OBJLINK)
	$(CC) $(CFLAGS) -o $(BIN)/$(TSPC) $(OBJLINK) $(LDFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(BIN)/$(TSPC)

install: all
