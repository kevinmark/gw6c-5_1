# $Id: Makefile,v 1.10 2007/06/14 14:21:13 cnepveu Exp $
#
#  This source code copyright (c) Hexago Inc. 2005,2007.
#
#  LICENSE NOTICE: You may use and modify this source code only if you
#  have executed a valid license agreement with Hexago Inc. granting
#  you the right to do so, the said license agreement governing such
#  use and modifications.   Copyright or other intellectual property
#  notices are not to be removed from the source code.
#

include ../Mk/mk-$(target).mk

all:

install: all
	@echo Installing templates
	@if [ $(template) ]; then ( \
		mkdir -p ../$(install_template); \
		for template_var in README $(template); do \
	   	$(COPY) $${template_var} ../$(install_template)/$${template_var}; \
		done; ) \
	else ( \
		mkdir -p $(install_template); \
		for template_var in README $(target).sh; do \
                $(COPY) $${template_var} $(install_template)/$${template_var}; \
                done; ) \
	fi

clean: 
	@echo
