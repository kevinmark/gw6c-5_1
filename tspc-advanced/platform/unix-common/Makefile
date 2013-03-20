# $Id: Makefile,v 1.8 2007/11/28 17:27:23 cnepveu Exp $
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

OS_UNAME=os_uname.h
OBJDIR=../../objs
OBJ=$(OBJDIR)/unix-main.o $(OBJDIR)/unix-echo-request.o
BIN=../bin

CFLAGS=-O2 -Wall -I../../include -I../../platform/$(target)
CC=$(CROSS_COMPILE)gcc

all: $(OBJ)

$(OS_UNAME):
	@echo > $(OS_UNAME)
	@echo \#define OS_UNAME_INFO \"`uname -a`\" >> $(OS_UNAME)
	@echo >> $(OS_UNAME)

$(OBJDIR)/unix-main.o:unix-main.c $(OS_UNAME)
	$(CC) $(CFLAGS) -c unix-main.c -o $(OBJDIR)/unix-main.o $(DEFINES)

$(OBJDIR)/unix-echo-request.o:unix-echo-request.c
	$(CC) $(CFLAGS) -c unix-echo-request.c -o $(OBJDIR)/unix-echo-request.o $(DEFINES)

clean:
	rm -f $(OBJDIR)/unix-main.o $(OBJDIR)/unix-echo-request.o $(OS_UNAME)

install: all
