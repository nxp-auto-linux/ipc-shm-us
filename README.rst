.. SPDX-License-Identifier: BSD-3-Clause

==============================================
IPCF Shared Memory User-space Driver for Linux
==============================================

:Copyright: 2018-2021 NXP

Overview
========
Linux IPCF Shared Memory User-space Driver enables communication over shared
memory with an RTOS running on different cores of the same processor.

The driver is accompanied by a sample application which demonstrates a ping-pong
message communication with an RTOS application (for more details see the readme
from sample directory).

The driver is integrated as out-of-tree kernel modules in NXP Auto
Linux BSP.

The source code of this Linux driver is published on `source.codeaurora.org
<https://source.codeaurora.org/external/autobsps32/ipcf/ipc-shm-us/>`_.

HW platforms
============
The supported processors are listed in the sample application documentation.

Configuration notes
===================
For hardware configuration, please see Configuration Notes from "IPCF Shared
Memory Kernel Driver for Linux".

The user-space static library (libipc-shm) will automatically insert the IPC UIO
kernel module at initialization time. The path to the kernel module in the
target board rootfs can be overwritten at compile time by setting
IPC_UIO_MODULE_DIR variable from the caller.

Cautions
========
The driver provides direct access to physical memory that is mapped non-cachable
in user-space. Therefore, applications should make only aligned accesses in the
shared memory buffers. Caution should be used when working with libc functions
that may do unaligned accesses (e.g., string processing functions).

For technical support please go to:
    https://www.nxp.com/support
