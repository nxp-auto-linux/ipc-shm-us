.. SPDX-License-Identifier: BSD-3-Clause

==============================================
IPCF Shared Memory User-space Driver for Linux
==============================================

:Copyright: 2018-2021,2023 NXP

Overview
========
The Linux IPCF Shared Memory User-space Driver enables communication over shared memory
with an RTOS application running on a different core of the same processor.

The driver is accompanied by a sample application which demonstrates a ping-pong
message communication with a RTOS application (for more details see the readme
from the sample directory).

The driver is integrated as an out-of-tree kernel module in NXP Auto
Linux BSP.

The source code of this Linux driver is published on `github.com
<https://github.com/nxp-auto-linux/ipc-shm>`_.

HW platforms
============
The supported processors are listed in the sample application documentation.

Configuration notes
===================
For hardware configurations please see the Configuration Notes from the "IPCF Shared
Memory Kernel Driver for Linux".

The user-space static library (libipc-shm) will automatically insert the IPC UIO
kernel module at initialization. The path to the kernel module in the
target board rootfs can be overwritten at compile time by setting
IPC_UIO_MODULE_DIR variable from the caller.

Cautions
========
This driver provides direct access to physical memory that is mapped as
non-cachable in user-space. Therefore, applications should make only aligned accesses in the
shared memory buffers. Caution should be used when working with libc functions that
may do unaligned accesses (e.g., string processing functions).

For technical support please go to:
    https://www.nxp.com/support
