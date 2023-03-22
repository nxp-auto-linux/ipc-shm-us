.. SPDX-License-Identifier: BSD-3-Clause

==========================================================
IPCF Shared Memory User-space Sample Application for Linux
==========================================================

:Copyright: 2018-2021,2023 NXP

Overview
========
This sample application demonstrates a ping-pong message communication with an
RTOS application, using the user-space shared memory driver.

The application initializes the shared memory driver and sends messages to the
remote sample application, waiting for a reply after each message is sent. When
a reply is received from remote application, it wakes up and sends another
message.

This application can be built to notify the remote application using inter-core
interrupts (default behavior) or to transmit without notifying the remote
application. If the latter is used, the remote application polls for available
messages.

Prerequisites
=============
 - EVB board for supported processors: S32G274A, S32R45, S32G399A
 - NXP Automotive Linux BSP

Building the application
========================

Building with Yocto
-------------------
1. Follow the steps for building NXP Auto Linux BSP with Yocto::
   Linux BSP User Manual from Flexera catalog

* user must change the branch release/**IPCF_RELEASE_NAME** and modify in
  build/sources/meta-alb/recipes-kernel/ipc-shm/ipc-shm.bb::

    - BRANCH ?= "${RELEASE_BASE}"
    + BRANCH ?= "release/**IPCF_RELEASE_NAME**"

    - SRCREV = "xxxxxxxxxx"
    + SRCREV = "${AUTOREV}"

  where **IPCF_RELEASE_NAME** is the name of Inter-Platform Communication
  Framework release from Flexera catalog and "xxxxxxxxxx" is the commit ID
  which must be replaced with "${AUTOREV}"

* enable User-space I/O driver, e.g.::

    bitbake virtual/kernel -c menuconfig

  then select::

    device driver --->
    {*} Userspace I/O drivers

* use image fsl-image-auto with any of the following machines supported for IPCF:
  s32g274aevb, s32r45xevb, s32g399aevb.

2. Get IPCF-ShM user-space driver from GitHub::

    git clone https://github.com/nxp-auto-linux/ipc-shm-us
    git -C ipc-shm-us submodule update --init --remote

3. Use branch release/**IPCF_RELEASE_NAME** for ipc-shm-us and also for submodule::

    git -C ipc-shm-us checkout release/**IPCF_RELEASE_NAME**
    git -C ipc-shm-us/common checkout release/**IPCF_RELEASE_NAME**

4. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target board rootfs and the platform name, e.g.::

    make -C ./ipc-shm-us/sample PLATFORM=S32GEN1 IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing ``uname -r`` in the target board
   and PLATFORM value can be S32GEN1.

   **Note:** for S32G3xx must add PLATFORM_FLAVOR=s32g3

Building manually
-----------------
1. Get NXP Auto Linux kernel and IPCF driver from GitHub::

    git clone https://github.com/nxp-auto-linux/linux
    git clone https://github.com/nxp-auto-linux/ipc-shm-us
    git -C ipc-shm-us submodule update --init --remote

2. Use branch release/**IPCF_RELEASE_NAME** for ipc-shm-us and also for submodule::

    git -C ipc-shm-us checkout release/**IPCF_RELEASE_NAME**
    git -C ipc-shm-us/common checkout release/**IPCF_RELEASE_NAME**

3. Configure Linux kernel to enable User-space I/O driver::

    make -C ./linux menuconfig

  then select::

    device driver --->
    {*} Userspace I/O drivers

4. Export CROSS_COMPILE and ARCH variables and build Linux kernel, e.g.::

    export CROSS_COMPILE=/<toolchain-path>/aarch64-linux-gnu-
    export ARCH=arm64
    make -C ./linux s32gen1_defconfig
    make -C ./linux

5. Build IPCF-ShM driver modules providing kernel source location, e.g.::

    make -C ./ipc-shm-us/common KERNELDIR=$PWD/linux modules

6. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target board rootfs and the platform name, e.g.::

    make -C ./ipc-shm-us/sample PLATFORM=S32GEN1 IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing `uname -r` in the target board
   and PLATFORM value can be S32GEN1.

   **Note:** for S32G3xx must add PLATFORM_FLAVOR=s32g3

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

3. Run sample and then specify the number of ping messages to be exchanged with
   peer when prompted::

    ./ipc-shm-sample.elf

    Input number of messages to send:

Notes:
  To exit the sample, input number of messages 0 or send interrupt signal (e.g.
  Ctrl + C)
