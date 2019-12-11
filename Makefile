# SPDX-License-Identifier:	BSD-3-Clause
#
# Copyright 2019 NXP
#
MAKEFLAGS += --warn-undefined-variables
EXTRA_CFLAGS ?=

ifeq ($(CROSS_COMPILE),)
$(error CROSS_COMPILE is not set!)
endif

# IPC_UIO_DIR needed to include ipc-uio.h
ifeq ($(IPC_UIO_DIR),)
$(error IPC_UIO_DIR is not set!)
endif
$(info IPC_UIO_DIR = $(IPC_UIO_DIR))

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
RM := rm -f

includes := -I./common -I./hw -I./os -I$(IPC_UIO_DIR)
CFLAGS += -Wall -g $(includes) #-DDEBUG
CFLAGS += $(EXTRA_CFLAGS)

lib_name = libipc-shm.a

# path to ipc-shm-uio.ko needed for auto-inserting the module
IPC_UIO_MODULE_PATH ?= ./ipc-shm-uio.ko

ko_basename := $(notdir $(IPC_UIO_MODULE_PATH))
ko_name := $(shell echo $(ko_basename) | tr '-' '_' | sed -e 's/.ko//')
CFLAGS += -DIPC_UIO_MODULE_PATH=\"$(IPC_UIO_MODULE_PATH)\"
CFLAGS += -DIPC_UIO_MODULE_NAME=\"$(ko_name)\"

# object file list
objs = common/ipc-shm.o common/ipc-queue.o os/ipc-os.o

%.o: %.c
	@echo 'Building lib file: $<'
	$(CC) -c $(CFLAGS) -o $@ $<
	@echo ' '

$(lib_name): $(objs)
	@echo 'Building target: $@'
	$(AR) rcs $(lib_name) $(objs)
	@echo ' '

clean:
	$(RM) $(objs) $(lib_name)

.PHONY: clean
