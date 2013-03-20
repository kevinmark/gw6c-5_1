#
# $Id: Makefile,v 1.22 2007/05/11 20:11:12 cnepveu Exp $
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
#

OBJ_DIR=objects
BIN_DIR=bin
LIB_DIR=lib
INC_DIR=gw6cconfig
C_COMPILER=$(CROSS_COMPILE)gcc -c
COMPILER=$(CROSS_COMPILE)g++ -c
LINKER=$(CROSS_COMPILE)g++
C_LINKER=$(CROSS_COMPILE)gcc
RANLIB=$(CROSS_COMPILE)ranlib
ARCHIVER=$(CROSS_COMPILE)ar
CPP_FLAGS=-I. -O2 -Wall -DNDEBUG $(ADDITIONAL_CPPFLAGS) $(HAP6_DEFINES)
C_FLAGS=-I. -O2 -Wall -DNDEBUG $(ADDITIONAL_CPPFLAGS) $(HAP6_DEFINES)
TARGET=$(LIB_DIR)/libgw6cconfig.a
LD_FLAGS=-O2 -L$(LIB_DIR) -lgw6cconfig $(ADDITIONAL_LDFLAGS)


.PHONY=all export clean

#
# GNU make - BSD make workaround
#   Here, we're specifying additional switches for WINDOWS targets.
#   The reason it's made this way is because BSD and GNU make don't agree
#   on conditional makefile instructions(e.g.: "ifdef", ".ifdef").
#
all:
	@if [ "$(target)" = "windows" ]; then \
	  $(MAKE) target=$(target) ADDITIONAL_CPPFLAGS=-DWIN32 ADDITIONAL_LDFLAGS=-lws2_32 all-real ;\
	else \
	  $(MAKE) target=$(target) all-real ;\
	fi

all-real: $(TARGET) $(BIN_DIR) test_targets

export:
	@if [ "$(target)" = "windows" ]; then \
	  $(MAKE) target=$(target) ADDITIONAL_CPPFLAGS=-DWIN32 ADDITIONAL_LDFLAGS=-lws2_32 export-real ;\
	else \
	  $(MAKE) target=$(target) export-real ;\
	fi

export-real: $(TARGET)
	@if [ -z "$(exportdir)"  -o  ! -d "$(exportdir)" ]; then \
	  echo ""; \
	  echo "You must specify an export directory(i.e.: exportdir=the_export_dir) in which the"\
	       "Gateway6 Client configuration library will be installed."; \
	  echo ""; \
	  exit 1; \
	fi
	mkdir -p $(exportdir)/gw6cconfig
	cp $(TARGET) $(exportdir)/gw6cconfig
	cp $(INC_DIR)/*.h $(exportdir)/gw6cconfig


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

OBJS=$(OBJ_DIR)/namevalueparser.o \
	$(OBJ_DIR)/config.o \
	$(OBJ_DIR)/namevalueconfig.o \
	$(OBJ_DIR)/gw6cvalidation.o \
	$(OBJ_DIR)/gw6cconfig.o \
	$(OBJ_DIR)/gw6c_c_wrapper.o \
	$(OBJ_DIR)/gw6cuistrings.o \
	$(OBJ_DIR)/hap6devicemappingconfig.o \
	$(OBJ_DIR)/hap6_devmap_c_wrap.o

$(OBJ_DIR)/namevalueparser.o: src/namevalueparser.cc $(INC_DIR)/namevalueparser.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/namevalueparser.o src/namevalueparser.cc

$(OBJ_DIR)/config.o: src/config.cc $(INC_DIR)/config.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/config.o src/config.cc

$(OBJ_DIR)/namevalueconfig.o: src/namevalueconfig.cc $(INC_DIR)/namevalueconfig.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/namevalueconfig.o src/namevalueconfig.cc

$(OBJ_DIR)/gw6cvalidation.o: src/gw6cvalidation.cc $(INC_DIR)/gw6cvalidation.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/gw6cvalidation.o src/gw6cvalidation.cc

$(OBJ_DIR)/gw6cconfig.o: src/gw6cconfig.cc $(INC_DIR)/gw6cconfig.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/gw6cconfig.o src/gw6cconfig.cc

$(OBJ_DIR)/gw6c_c_wrapper.o: src/gw6c_c_wrapper.cc $(INC_DIR)/gw6c_c_wrapper.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/gw6c_c_wrapper.o src/gw6c_c_wrapper.cc

$(OBJ_DIR)/gw6cuistrings.o: src/gw6cuistrings.c $(INC_DIR)/gw6cuistrings.h
	$(C_COMPILER) $(C_FLAGS) -o $(OBJ_DIR)/gw6cuistrings.o src/gw6cuistrings.c

$(OBJ_DIR)/hap6devicemappingconfig.o:src/hap6devicemappingconfig.cc $(INC_DIR)/hap6devicemappingconfig.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/hap6devicemappingconfig.o src/hap6devicemappingconfig.cc

$(OBJ_DIR)/hap6_devmap_c_wrap.o: src/hap6_devmap_c_wrap.cc $(INC_DIR)/hap6_devmap_c_wrap.h
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/hap6_devmap_c_wrap.o src/hap6_devmap_c_wrap.cc

$(OBJ_DIR)/test_namevalue.o: src/test_namevalue.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/test_namevalue.o src/test_namevalue.cc

$(OBJ_DIR)/test_c_wrapper.o: src/test_c_wrapper.c
	$(C_COMPILER) $(C_FLAGS) -o $(OBJ_DIR)/test_c_wrapper.o src/test_c_wrapper.c

$(OBJ_DIR)/test_hap6.o: src/test_hap6.cc
	$(COMPILER) $(CPP_FLAGS) -o $(OBJ_DIR)/test_hap6.o src/test_hap6.cc

$(OBJ_DIR)/test_c_hap6_wrap.o: src/test_c_hap6_wrap.c
	$(C_COMPILER) $(C_FLAGS) -o $(OBJ_DIR)/test_c_hap6_wrap.o src/test_c_hap6_wrap.c

test_targets: $(TARGET) $(OBJ_DIR)/test_namevalue.o $(OBJ_DIR)/test_c_wrapper.o $(OBJ_DIR)/test_hap6.o $(OBJ_DIR)/test_c_hap6_wrap.o
	$(LINKER) -o $(BIN_DIR)/test_namevalue $(OBJ_DIR)/test_namevalue.o $(LD_FLAGS)
	$(C_LINKER) -o $(BIN_DIR)/test_c_wrapper $(OBJ_DIR)/test_c_wrapper.o $(LD_FLAGS) -lstdc++
	$(LINKER) -o $(BIN_DIR)/test_hap6 $(OBJ_DIR)/test_hap6.o $(LD_FLAGS)
	$(C_LINKER) -o $(BIN_DIR)/test_c_hap6_wrap $(OBJ_DIR)/test_c_hap6_wrap.o $(LD_FLAGS) -lstdc++
	

$(TARGET): $(OBJ_DIR) $(OBJS) $(LIB_DIR)
	$(ARCHIVER) cru $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

clean:
	@echo -n "Cleaning up workspace... "
	@rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	@echo "done."
