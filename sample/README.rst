.. SPDX-License-Identifier: BSD-3-Clause

==========================================================
IPCF Shared Memory User-space Sample Application for Linux
==========================================================

:Copyright: 2018-2021 NXP

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
 - EVB board for supported processors: S32V234, S32G274A and S32R45
 - NXP Automotive Linux BSP

Building the application
========================

Building with Yocto
-------------------
1. Follow the steps for building NXP Auto Linux BSP with Yocto:
  https://source.codeaurora.org/external/autobsps32/auto_yocto_bsp/tree/README

* for S32V234 use branch release/bsp23.0 and modify build/sources/meta-alb/recipes-kernel/ipc-shm/ipc-shm.bb::

    - SRCREV = "af9a41d262a57a2c3f4be0f4042adc10b47ffdd6"
    + SRCREV = "a32bb41885c21fd440385c2a382a672d40d2397f"

    + KERNEL_MODULE_PROBECONF += "ipc-shm-uio"
    + module_conf_ipc-shm-uio = "blacklist ipc-shm-uio"
    + FILES_${PN} += "${sysconfdir}/modprobe.d/*"

* for S32R45 use branch release/bsp24.0 and do the following modifications:

  * in build/sources/meta-alb/recipes-kernel/ipc-shm/ipc-shm.bb::

     - SRCREV = "a32bb41885c21fd440385c2a382a672d40d2397f"
     + SRCREV = "90d0aa48d557ae8099ae39553e0ba0154f8b5f28"

     - PROVIDES_append_s32v2xx = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"
     - RPROVIDES_${PN}_append_s32v2xx = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"
     + PROVIDES_append = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"
     + RPROVIDES_${PN}_append = " kernel-module-ipc-shm-uio${KERNEL_MODULE_PACKAGE_SUFFIX}"

     + KERNEL_MODULE_PROBECONF += "ipc-shm-uio"
     + module_conf_ipc-shm-uio = "blacklist ipc-shm-uio"
     + FILES_${PN} += "${sysconfdir}/modprobe.d/*"

  * in build/sources/meta-alb/recipes-fsl/images/fsl-image-s32-common.inc::

     + IMAGE_INSTALL_append_s32r45evb += " ipc-shm "

* for S32G274A use branch release/bsp27.0

* enable User-space I/O driver, e.g.::

    bitbake virtual/kernel -c menuconfig

  then select::

    device driver --->
    {*} Userspace I/O drivers

* use image fsl-image-auto with any of the following machines supported for IPCF:
  s32g274aevb, s32r45evb, s32v234evb.

2. Get IPCF-ShM user-space driver from Code Aurora::

    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/
    git -C ipc-shm-us submodule update --init

3. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target board rootfs and the platform name, e.g.::

    make -C ./ipc-shm-us/sample PLATFORM=S32V234 IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing ``uname -r`` in the target board
   and PLATFORM value can be S32V234 or S32GEN1 (for S32G274A and S32R45).

Building manually
-----------------
1. Get NXP Auto Linux kernel and IPCF driver from Code Aurora::

    git clone https://source.codeaurora.org/external/autobsps32/linux/ -b release/bsp23.0
    git clone https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/
    git -C ipc-shm-us submodule update --init

- use branch release/bsp23.0 for S32V234 and release/bsp24.0 for S32G274A and S32R45

2. Configure Linux kernel to enable User-space I/O driver::

    make -C ./linux menuconfig

  then select::

    device driver --->
    {*} Userspace I/O drivers

3. Export CROSS_COMPILE and ARCH variables and build Linux kernel, e.g.::

    export CROSS_COMPILE=/<toolchain-path>/aarch64-linux-gnu-
    export ARCH=arm64
    make -C ./linux

4. Build IPCF-ShM driver modules providing kernel source location, e.g.::

    make -C ./ipc-shm-us/common KERNELDIR=$PWD/linux modules

5. Build sample application with IPCF-ShM library, providing the location of the
   IPC UIO kernel module in the target board rootfs and the platform name, e.g.::

    make -C ./ipc-shm-us/sample PLATFORM=S32V234 IPC_UIO_MODULE_DIR="/lib/modules/<kernel-release>/extra"

   where <kernel-release> can be obtained executing `uname -r` in the target board
   and PLATFORM value can be S32V234 or S32GEN1 (for S32G274A and S32R45).

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

Configuration Notes
===================

Polling
-------
In order to compile the shared memory sample application with polling support,
the makefile parameter ``POLLING`` must be set to ``yes``, e.g.::

    make -C ./ipc-shm-us/sample POLLING=yes PLATFORM=S32GEN1

Notes:
  The remote sample application must be built with polling support as well.
  Please refer to the remote sample build instructions for more details.

This sample demonstrates how shared memory polling API can be used to poll for
incoming messages instead of using inter-core interrupts notifications.
