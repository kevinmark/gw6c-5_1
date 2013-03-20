#
# $Id: Makefile,v 1.29 2007/05/11 20:10:59 cnepveu Exp $
#
#  Copyright (c) 2007 Hexago Inc. All rights reserved.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#
# Author: Charles Nepveu
#


OBJ_DIR=objects
BIN_DIR=bin
LIB_DIR=lib
INC_DIR=gw6cmessaging
C_COMPILER=$(CROSS_COMPILE)gcc -c -O2 -DNDEBUG
COMPILER=$(CROSS_COMPILE)g++ -c -O2 -DNDEBUG 
LINKER=$(CROSS_COMPILE)g++ -O2
C_LINKER=$(CROSS_COMPILE)gcc -O2
RANLIB=$(CROSS_COMPILE)ranlib
ARCHIVER=$(CROSS_COMPILE)ar
CPP_FLAGS=-I. -Wall $(ADDITIONAL_CPPFLAGS) $(HAP6_DEFINES)
C_FLAGS=-I. -Wall $(HAP6_DEFINES)
TEST_IPCLAYER=$(BIN_DIR)/test_ipclayer
TEST_MESSAGING=$(BIN_DIR)/test_messaging
TEST_GW6CEMUL8OR=$(BIN_DIR)/test_gw6cemulator
TARGET=$(LIB_DIR)/libgw6cmessaging.a
LD_FLAGS=-L$(LIB_DIR) -lgw6cmessaging $(ADDITIONAL_LDFLAGS)

WIN_ADDITIONAL_OBJS=$(OBJ_DIR)/pipeserver.o \
	$(OBJ_DIR)/pipeclient.o \
	$(OBJ_DIR)/pipeio.o \
	$(OBJ_DIR)/threadwrapper.o \
	$(OBJ_DIR)/semaphore.o



.PHONY=all export clean

#
# GNU make - BSD make workaround
#   Here, we're specifying additional switches for WINDOWS targets.
#   The reason it's made this way is because BSD and GNU make don't agree
#   on conditional makefile instructions(e.g.: "ifdef", ".ifdef").
#
all:
	@if [ "$(target)" = "windows" ]; then \
	  $(MAKE) target=$(target) ADDITIONAL_CPPFLAGS=-DWIN32 ADDITIONAL_OBJS="$(WIN_ADDITIONAL_OBJS)" all-real ;\
	else \
	  $(MAKE) target=$(target) ADDITIONAL_LDFLAGS="-lrt -lpthread" all-real ;\
	fi

all-real: $(TARGET) $(BIN_DIR) $(TEST_IPCLAYER) $(TEST_MESSAGING) $(TEST_GW6CEMUL8OR)

export:
	@if [ "$(target)" = "windows" ]; then \
	  $(MAKE) target=$(target) ADDITIONAL_CPPFLAGS=-DWIN32 ADDITIONAL_OBJS="$(WIN_ADDITIONAL_OBJS)" export-real ;\
	else \
	  $(MAKE) target=$(target) ADDITIONAL_LDFLAGS="-lrt -lpthread" export-real ;\
	fi

export-real: $(TARGET)
	@if [ -z "$(exportdir)"  -o  ! -d "$(exportdir)" ]; then \
	  echo ""; \
	  echo "You must specify an export directory(i.e.: exportdir=the_export_dir) in which the"\
	       "Gateway6 Client messaging library will be installed."; \
	  echo ""; \
	  exit 1; \
	fi
	mkdir -p $(exportdir)/gw6cmessaging
	cp $(TARGET) $(exportdir)/gw6cmessaging
	cp $(INC_DIR)/*.h $(exportdir)/gw6cmessaging


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

OBJS=$(ADDITIONAL_OBJS) \
	$(OBJ_DIR)/ipcserver.o \
	$(OBJ_DIR)/ipcclient.o \
	$(OBJ_DIR)/servent.o \
	$(OBJ_DIR)/message.o \
	$(OBJ_DIR)/messagesender.o \
	$(OBJ_DIR)/communicationsmgr.o \
	$(OBJ_DIR)/clientmsgtranslator.o \
	$(OBJ_DIR)/clientmsgsender.o \
	$(OBJ_DIR)/servermsgtranslator.o \
	$(OBJ_DIR)/servermsgsender.o \
	$(OBJ_DIR)/clientmessengerimpl.o \
	$(OBJ_DIR)/guimessengerimpl.o \
	$(OBJ_DIR)/gw6c_c_wrapper.o \
	$(OBJ_DIR)/gw6cuistrings.o


$(OBJ_DIR)/ipcserver.o: src/ipcserver.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/ipcserver.o src/ipcserver.cc

$(OBJ_DIR)/ipcclient.o: src/ipcclient.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/ipcclient.o src/ipcclient.cc

$(OBJ_DIR)/pipeserver.o: src/$(target)/pipeserver.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/pipeserver.o src/$(target)/pipeserver.cc

$(OBJ_DIR)/pipeclient.o: src/$(target)/pipeclient.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/pipeclient.o src/$(target)/pipeclient.cc

$(OBJ_DIR)/pipeio.o: src/$(target)/pipeio.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/pipeio.o src/$(target)/pipeio.cc

$(OBJ_DIR)/servent.o: src/servent.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/servent.o src/servent.cc

$(OBJ_DIR)/threadwrapper.o: src/$(target)/threadwrapper.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/threadwrapper.o src/$(target)/threadwrapper.cc

$(OBJ_DIR)/semaphore.o: src/$(target)/semaphore.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/semaphore.o src/$(target)/semaphore.cc

$(OBJ_DIR)/message.o: src/message.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/message.o src/message.cc

$(OBJ_DIR)/messagesender.o: src/messagesender.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/messagesender.o src/messagesender.cc

$(OBJ_DIR)/communicationsmgr.o: src/communicationsmgr.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/communicationsmgr.o src/communicationsmgr.cc

$(OBJ_DIR)/clientmsgtranslator.o: src/clientmsgtranslator.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/clientmsgtranslator.o src/clientmsgtranslator.cc

$(OBJ_DIR)/clientmsgsender.o: src/clientmsgsender.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/clientmsgsender.o src/clientmsgsender.cc

$(OBJ_DIR)/servermsgtranslator.o: src/servermsgtranslator.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/servermsgtranslator.o src/servermsgtranslator.cc

$(OBJ_DIR)/servermsgsender.o: src/servermsgsender.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/servermsgsender.o src/servermsgsender.cc

$(OBJ_DIR)/clientmessengerimpl.o: src/clientmessengerimpl.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/clientmessengerimpl.o src/clientmessengerimpl.cc

$(OBJ_DIR)/guimessengerimpl.o: src/guimessengerimpl.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/guimessengerimpl.o src/guimessengerimpl.cc

$(OBJ_DIR)/gw6c_c_wrapper.o: src/gw6c_c_wrapper.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/gw6c_c_wrapper.o src/gw6c_c_wrapper.cc

$(OBJ_DIR)/gw6cuistrings.o: src/gw6cuistrings.c
	$(C_COMPILER) $(C_FLAGS) -o $(OBJ_DIR)/gw6cuistrings.o src/gw6cuistrings.c

$(OBJ_DIR)/test_ipclayer.o: src/test_ipclayer.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/test_ipclayer.o src/test_ipclayer.cc

$(OBJ_DIR)/test_messaginglayer.o: src/test_messaginglayer.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/test_messaginglayer.o src/test_messaginglayer.cc

$(OBJ_DIR)/test_gw6cemulator.o: src/test_gw6cemulator.c
	$(C_COMPILER) $(C_FLAGS) -o $(OBJ_DIR)/test_gw6cemulator.o src/test_gw6cemulator.c


$(TARGET): $(OBJ_DIR) $(OBJS) $(LIB_DIR)
	$(ARCHIVER) cru $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)


$(TEST_IPCLAYER): $(TARGET) $(OBJ_DIR)/test_ipclayer.o
	$(LINKER) -o $(TEST_IPCLAYER) $(OBJ_DIR)/test_ipclayer.o $(LD_FLAGS)

$(TEST_MESSAGING): $(TARGET) $(OBJ_DIR)/test_messaginglayer.o
	$(LINKER) -o $(TEST_MESSAGING) $(OBJ_DIR)/test_messaginglayer.o $(LD_FLAGS)

$(TEST_GW6CEMUL8OR): $(TARGET) $(OBJ_DIR)/test_gw6cemulator.o
	$(LINKER) -o $(TEST_GW6CEMUL8OR) $(OBJ_DIR)/test_gw6cemulator.o $(LD_FLAGS)

clean:
	@echo -n "Cleaning up workspace... "
	@rm -f $(OBJS) $(TARGET)
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	@echo "done."
