/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2019 NXP
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include "ipc-os.h"
#include "ipc-hw.h"
#include "ipc-shm.h"
#include "ipc-uio.h"

#define IPC_SHM_DEV_UIO_NAME    "/dev/uio0"
#define IPC_SHM_DEV_MEM_NAME    "/dev/mem"

/**
 * struct ipc_os_priv - OS specific private data
 * @shm_size:		local/remote ShM size
 * @local_virt_shm:	local ShM virtual address
 * @remote_virt_shm:	remote ShM virtual address
 * @local_shm_map:	local ShM mapped page address
 * @remote_shm_map:	remote ShM mapped page address
 * @local_shm_offset:	local ShM offset in mapped page
 * @remote_shm_offset:	remote ShM offset in mapped page
 * @rx_cb:		upper layer Rx callback function
 * @irq_thread_id:	Rx interrupt thread id
 * @uio_fd:		UIO device file descriptor
 * @mem_fd:		MEM device file descriptor
 */
struct ipc_os_priv {
	size_t shm_size;
	void *local_virt_shm;
	void *remote_virt_shm;
	void *local_shm_map;
	void *remote_shm_map;
	size_t local_shm_offset;
	size_t remote_shm_offset;
	int (*rx_cb)(int budget);
	pthread_t irq_thread_id;
	int uio_fd;
	int mem_fd;
};

/* OS specific private data */
static struct ipc_os_priv priv;

/* Rx sotfirq thread */
static void *ipc_shm_softirq(void *arg)
{
	const int budget = IPC_SOFTIRQ_BUDGET;
	int irq_count;
	int work;

	while (1) {
		/* block(sleep) until notified from kernel IRQ handler */
		read(priv.uio_fd, &irq_count, sizeof(irq_count));

		work = priv.rx_cb(budget);

		if (work < budget) {
			/* work done, re-enable irq */
			ipc_hw_irq_enable();
		} else {
			/* work not done, yield and wait for reschedule */
			sched_yield();
		}
	}

	return 0;
}

/**
 * ipc_shm_os_init() - OS specific initialization code
 * @cfg:         configuration parameters
 * @rx_cb:	Rx callback to be called from Rx softirq
 *
 * Return: 0 on success, error code otherwise
 */
int ipc_os_init(const struct ipc_shm_cfg *cfg, int (*rx_cb)(int))
{
	size_t page_size = sysconf(_SC_PAGE_SIZE);
	off_t page_phys_addr;
	int err;

	if (!rx_cb)
		return -EINVAL;

	/* save params */
	priv.shm_size = cfg->shm_size;
	priv.rx_cb = rx_cb;

	/*
	 * TODO: automatically insert ipc-shm-uio.ko module here with the
	 * interrupt parameters specified in cfg.
	 */

	/* open MEM device for interrupt support */
	priv.mem_fd = open(IPC_SHM_DEV_MEM_NAME, O_RDWR);
	if (priv.mem_fd == -1) {
		shm_err("Can't open %s device\n", IPC_SHM_DEV_MEM_NAME);
		return -ENODEV;
	}

	/* map local physical shared memory */
	/* truncate address to a multiple of page size, or mmap will fail */
	page_phys_addr = (cfg->local_shm_addr / page_size) * page_size;
	priv.local_shm_offset = cfg->local_shm_addr - page_phys_addr;

	priv.local_shm_map = mmap(NULL, priv.local_shm_offset + cfg->shm_size,
				  PROT_READ | PROT_WRITE, MAP_SHARED,
				  priv.mem_fd, page_phys_addr);
	if (priv.local_shm_map == MAP_FAILED) {
		shm_err("Can't map memory: %lx\n", cfg->local_shm_addr);
		err = -ENOMEM;
		goto err_close_mem_dev;
	}

	priv.local_virt_shm = priv.local_shm_map + priv.local_shm_offset;

	/* map remote physical shared memory */
	page_phys_addr = (cfg->remote_shm_addr / page_size) * page_size;
	priv.remote_shm_offset = cfg->remote_shm_addr - page_phys_addr;

	priv.remote_shm_map = mmap(NULL, priv.remote_shm_offset + cfg->shm_size,
				   PROT_READ | PROT_WRITE, MAP_SHARED,
				   priv.mem_fd, page_phys_addr);
	if (priv.remote_shm_map == MAP_FAILED) {
		shm_err("Can't map memory: %lx\n", cfg->remote_shm_addr);
		err = -ENOMEM;
		goto err_unmap_local_shm;
	}

	priv.remote_virt_shm = priv.remote_shm_map + priv.remote_shm_offset;

	/* open UIO device for interrupt support */
	priv.uio_fd = open(IPC_SHM_DEV_UIO_NAME, O_RDWR);
	if (priv.uio_fd == -1) {
		shm_err("Can't open %s device\n", IPC_SHM_DEV_UIO_NAME);
		err = -ENODEV;
		goto err_unmap_remote_shm;
	}

	/* start Rx softirq thread */
	/* TODO: start this thread with highest priority */
	err = pthread_create(&priv.irq_thread_id, NULL, ipc_shm_softirq, &priv);
	if (err == -1) {
		shm_err("Can't start Rx softirq thread\n");
		goto err_close_uio_dev;
	}

	shm_dbg("done\n");

	return 0;

err_close_uio_dev:
	close(priv.uio_fd);
err_unmap_remote_shm:
	munmap(priv.remote_shm_map, priv.remote_shm_offset + priv.shm_size);
err_unmap_local_shm:
	munmap(priv.local_shm_map, priv.local_shm_offset + priv.shm_size);
err_close_mem_dev:
	close(priv.mem_fd);

	return err;
}

/**
 * ipc_os_free() - free OS specific resources
 */
void ipc_os_free(void)
{
	void *res;

	/* disable hardirq */
	ipc_hw_irq_disable();

	shm_dbg("stopping irq thread\n");

	/* stop irq thread */
	pthread_cancel(priv.irq_thread_id);
	pthread_join(priv.irq_thread_id, &res);

	close(priv.uio_fd);

	/* unmap remote/local shm */
	munmap(priv.remote_shm_map, priv.remote_shm_offset + priv.shm_size);
	munmap(priv.local_shm_map, priv.local_shm_offset + priv.shm_size);

	close(priv.mem_fd);
}

/**
 * ipc_os_get_local_shm() - get local shared mem address
 */
uintptr_t ipc_os_get_local_shm(void)
{
	return (uintptr_t)priv.local_virt_shm;
}

/**
 * ipc_os_get_remote_shm() - get remote shared mem address
 */
uintptr_t ipc_os_get_remote_shm(void)
{
	return (uintptr_t)priv.remote_virt_shm;
}

static void ipc_send_uio_cmd(uint32_t uio_fd, int32_t cmd)
{
	int ret;

	ret = write(uio_fd, &cmd, sizeof(int));
	if (ret != sizeof(int)) {
		shm_dbg("Failed to execute UIO command %d", cmd);
	}
}

/**
 * ipc_hw_irq_enable() - enable notifications from remote
 */
void ipc_hw_irq_enable(void)
{
	ipc_send_uio_cmd(priv.uio_fd, IPC_UIO_ENABLE_RX_IRQ_CMD);
}

/**
 * ipc_hw_irq_disable() - disable notifications from remote
 */
void ipc_hw_irq_disable(void)
{
	ipc_send_uio_cmd(priv.uio_fd, IPC_UIO_DISABLE_RX_IRQ_CMD);
}

/**
 * ipc_hw_irq_notify() - notify remote that data is available
 */
void ipc_hw_irq_notify(void)
{
	ipc_send_uio_cmd(priv.uio_fd, IPC_UIO_TRIGGER_TX_IRQ_CMD);
}

int ipc_hw_init(const struct ipc_shm_cfg *cfg)
{
	/* dummy implementation: ipc-hw init is handled by kernel UIO module */
	return 0;
}

void ipc_hw_free(void)
{
	/* dummy implementation: ipc-hw free is handled by kernel UIO module */
}
