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
 - EVB board for supported processors: S32V234, S32G274A and S32R45X
 - NXP Automotive Linux BSP

Building the application
========================

Building with Yocto
-------------------
1. Follow the steps for building NXP Auto Linux BSP with Yocto:
  https://source.codeaurora.org/external/autobsps32/auto_yocto_bsp/tree/README

- for S32V234 use branch release/bsp23.0 and replace in build/sources/meta-alb/recipes-kernel/ipc-shm/ipc-shm.bb::

    - SRCREV = "af9a41d262a57a2c3f4be0f4042adc10b47ffdd6"
    + SRCREV = "a32bb41885c21fd440385c2a382a672d40d2397f"

- for S32G274A or S32R45X, use branch release/bsp24.0 and replace in build/sources/meta-alb/recipes-kernel/ipc-shm/ipc-shm.bb::

    - SRCREV = "a32bb41885c21fd440385c2a382a672d40d2397f"
    + SRCREV = "90d0aa48d557ae8099ae39553e0ba0154f8b5f28"

  and::

    - PROVIDES_append_s32v2xx = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"
    - RPROVIDES_${PN}_append_s32v2xx = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"
    + PROVIDES_append = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"
    + RPROVIDES_${PN}_append = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"

- enable User-space I/O driver, e.g.::

    bitbake virtual/kernel -c menuconfig

  then select::

    device driver --->
    {*} Userspace I/O drivers

- use image fsl-image-auto with any of the following machines supported for IPCF:
  s32g274aevb, s32r45xevb, s32v234evb.

2. Get IPCF-ShM user-space driver from Code Aurora::

    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/
    git -C ipc-shm-us submodule update --init

3. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target board rootfs and the platform name, e.g.::

    make -C ./ipc-shm-us/sample PLATFORM=S32V234 IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing `uname -r` in the target board
   and PLATFORM value can be S32V234 or S32GEN1 (for S32G274A and S32R45X).

Building manually
-----------------
1. Get NXP Auto Linux kernel and IPCF driver from Code Aurora::

    git clone https://source.codeaurora.org/external/autobsps32/linux/ -b release/bsp23.0
    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/
    git -C ipc-shm-us submodule update --init

- use branch release/bsp23.0 for S32V234 and release/bsp24.0 for S32G274A and S32R45x

2. Configure Linux kernel to enable User-space I/O driver::

    make -C ./linux menuconfig

  then select::

    device driver --->
    {*} Userspace I/O drivers

3. Export CROSS_COMPILE variable and build modules providing kernel source location::

    export CROSS_COMPILE=/<toolchain-path>/aarch64-linux-gnu-
    make -C ./linux
    make -C ./ipc-shm-us/common KERNELDIR=./linux modules

4. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target board rootfs and the platform name, e.g.::

    make -C ./ipc-shm-us/sample PLATFORM=S32V234 IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing `uname -r` in the target board
   and PLATFORM value can be S32V234 or S32GEN1 (for S32G274A and S32R45X).

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
