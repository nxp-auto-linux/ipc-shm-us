.. SPDX-License-Identifier: BSD-3-Clause

==========================================================
IPCF Shared Memory User-space Sample Application for Linux
==========================================================

:Copyright: 2018-2019 NXP

Overview
========
This sample application demonstrates a ping-pong message communication with an 
RTOS application, using the user-space shared memory driver.

The sample app initializes the shared memory driver and sends messages to the
remote app, waiting for a reply after each message is sent. When a reply is
received from remote app, it wakes up and sends another message.

Prerequisites
=============
 - EVB board for S32V234 silicon, cut 2.0, maskset 1N81U or
 - NXP Automotive Linux BSP

Building the application
========================
1. Build Linux kernel with UIO suport enabled, i.e., `CONFIG_UIO=y`.
   See build section from "IPCF Shared Memory Sample Application for Linux" for
   more details on building kernel and IPCF modules.

2. Build kernel UIO module::
   
    make -C ./ipc-shm KERNELDIR=./linux

3. Build sample application with IPCF-ShM library::
   
    make -C ./ipc-shm-us/sample IPC_UIO_DIR="${PWD}/ipc-shm/os" all

.. _run-shm-us-linux:

Running the application
=======================
1. Copy ipc-shm-uio.ko and ipc-shm-sample.elf in rootfs

2. Boot Linux: for silicon, see section "How to boot" from Auto Linux BSP user
   manual.

3. Insert IPCF UIO kernel modules after Linux boot::

    insmod /lib/modules/`uname -r`/extra/ipc-shm-uio.ko inter_core_rx_irq=1 inter_core_tx_irq=2 remote_core_index=0

4. Run sample specifying number of ping messages to be exchanged with peer::

    ./ipc-shm-sample.elf 10
