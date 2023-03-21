# SPDX-License-Identifier:	BSD-3-Clause
#
# Copyright 2019,2021-2023 NXP
#

# The following variables must be defined by caller:
#  CROSS_COMPILE: cross compiler path and prefix
#  KERNELDIR    : Linux kernel source code location
#  PLATFORM     : platform to build for
# Optional parameters:
#  POLLING      : set yo 'yes' to disable tx interrupt if
#                 remote sample application use polling

MAKEFLAGS += --warn-undefined-variables
EXTRA_CFLAGS ?=
EXTRA_LDFLAGS ?=
.DEFAULT_GOAL := all
PLATFORM_FLAVOR ?= s32g2

ifeq ($(CROSS_COMPILE),)
$(error CROSS_COMPILE is not set!)
endif

platforms := S32V234 S32GEN1
ifeq ($(filter-out $(PLATFORM),$(platforms)),$(platforms))
    $(error Set PLATFORM variable to a supported platform: '$(platforms)')
endif

ifeq ($(PLATFORM_FLAVOR),s32g2)
ccflags-y += -DPLATFORM_FLAVOR_s32g2
endif
ifeq ($(PLATFORM_FLAVOR),s32r45)
ccflags-y += -DPLATFORM_FLAVOR_s32r45
endif
ifeq ($(PLATFORM_FLAVOR),s32g3)
ccflags-y += -DPLATFORM_FLAVOR_s32g3
endif

POLLING ?= no
ifeq ($(POLLING),yes)
CFLAGS += -DPOLLING
endif

CC := $(CROSS_COMPILE)gcc
RM := rm -rf

elf_name := ipc-shm-sample.elf
libipc_dir ?= $(shell pwd)/..

CFLAGS += -Wall -g -I$(libipc_dir)/common -DCONFIG_SOC_$(PLATFORM) #-DDEBUG
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += -L$(libipc_dir) -lipc-shm -lpthread -lrt
LDFLAGS += $(EXTRA_LDFLAGS)

# object file list
objs = sample.o
objs += ipcf_Ip_Cfg_$(PLATFORM_FLAVOR).o

%.o: %.c
	@echo 'Building app file: $<'
	$(CC) -c $(CFLAGS) -o $@ $<
	@echo ' '

$(elf_name): $(objs) libipc
	@echo 'Building target: $@'
	$(CC) -o $(elf_name) $(objs) $(LDFLAGS)
	@echo 'Finished building target: $@'
	@echo ' '

libipc:
	$(MAKE) -C $(libipc_dir)

all: $(elf_name)

clean:
	$(MAKE) -C $(libipc_dir) $@
	$(RM) $(objs) $(elf_name)
	@echo ' '

.PHONY: all clean libipc
