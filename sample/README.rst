.. SPDX-License-Identifier: BSD-3-Clause

==========================================================
IPCF Shared Memory User-space Sample Application for Linux
==========================================================

:Copyright: 2018-2020 NXP

Overview
========
This sample application demonstrates a ping-pong message communication with an
RTOS application, using the user-space shared memory driver.

The sample app initializes the shared memory driver and sends messages to the
remote app, waiting for a reply after each message is sent. When a reply is
received from remote app, it wakes up and sends another message.

Prerequisites
=============
 - EVB board for S32V234 silicon, cut 2.0, maskset 1N81U
 - NXP Automotive Linux BSP

Building the application
========================

Building with Yocto
-------------------
1. Follow the steps for building NXP Auto Linux BSP with Yocto:

- https://source.codeaurora.org/external/autobsps32/auto_yocto_bsp/tree/README?h=release%2Fbsp22.0
- use branch release/bsp22.0 and replace in build/sources/meta-alb/recipes-kernel/ipc-shm/ipc-shm.bb::

    SRCREV = "4aa531d647de951d53d60a1313a356096d1092cd" with
    SRCREV = "a32bb41885c21fd440385c2a382a672d40d2397f"

2. Get IPCF-ShM user-space driver from Code Aurora::

    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/
    git -C ipc-shm-us submodule update --init

3. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target rootfs, e.g.::

    make -C ./ipc-shm-us/sample IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing `uname -r` in the target board.

Building manually
-----------------
1. Get NXP Auto Linux kernel and IPCF driver from Code Aurora::

    git clone https://source.codeaurora.org/external/autobsps32/linux/ -b release/bsp22.0
    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm/
    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/
    git -C ipc-shm-us submodule update --init

2. Export CROSS_COMPILE variable and build modules providing kernel source location::

    export CROSS_COMPILE=/<toolchain-path>/aarch64-linux-gnu-
    make -C ./linux
    make -C ./ipc-shm KERNELDIR=./linux modules

3. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target rootfs, e.g.::

    make -C ./ipc-shm-us/sample IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing `uname -r` in the target board.

.. _run-shm-us-linux:

Running the application
=======================
1. Copy ipc-shm-sample.elf to the target board rootfs. In case of building the
   sample manually, also copy IPC UIO kernel module (ipc-shm-uio.ko) to the
   directory provided during compilation via IPC_UIO_MODULE_DIR.

Notes:
  IPC UIO kernel module must be located in the same directory as provided via
  IPC_UIO_MODULE_DIR when building the sample.

2. Boot Linux: for silicon, see section "How to boot" from Auto Linux BSP user
   manual.

3. Run sample specifying number of ping messages to be exchanged with peer::

    ./ipc-shm-sample.elf 10
