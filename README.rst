.. SPDX-License-Identifier: BSD-3-Clause

==============================================
IPCF Shared Memory User-space Driver for Linux
==============================================

:Copyright: 2018 NXP

Overview
========
Linux IPCF Shared Memory User-space Driver enables communication over shared
memory with an RTOS running on different cores of the same processor.

The driver is accompanied by a sample application which demonstrates a ping-pong
message communication with an RTOS application (for more details see the readme
from sample directory).

HW platforms
============
The driver includes support of the following platforms:
 - S32V234 silicon, cut 2.1, maskset #1N81U

Configuration notes
===================
See Configuration Notes from "IPCF Shared Memory Kernel Driver for Linux".

Cautions
========
The driver provides direct access to physical memory that is mapped non-cachable
in user-space. Therefore, applications should make only aligned accesses in the
shared memory buffers. Caution should be used when working with libc functions
that may do unaligned accesses (e.g., string processing functions).

For technical support please go to:
    https://www.nxp.com/support
