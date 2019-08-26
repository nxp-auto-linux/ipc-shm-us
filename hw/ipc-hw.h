/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2019 NXP
 */
#ifndef IPC_HW_H
#define IPC_HW_H

int ipc_hw_init(const struct ipc_shm_cfg *cfg);
void ipc_hw_free(void);
void ipc_hw_irq_enable(void);
void ipc_hw_irq_disable(void);
void ipc_hw_irq_notify(void);

#endif /* IPC_HW_H */
