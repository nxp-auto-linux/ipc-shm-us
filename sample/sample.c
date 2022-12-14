/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2019-2020 NXP
 */
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>

#include "ipc-shm.h"

/* IPC SHM configuration defines */
#if defined(CONFIG_SOC_S32GEN1)
	#define LOCAL_SHM_ADDR 0x34100000
#elif defined(CONFIG_SOC_S32V234)
	#define LOCAL_SHM_ADDR 0x3E900000
#else
	#error "Platform not supported"
#endif
#define IPC_SHM_SIZE 0x100000 /* 1M local shm, 1M remote shm */
#define REMOTE_SHM_ADDR (LOCAL_SHM_ADDR + IPC_SHM_SIZE)
#define INTER_CORE_TX_IRQ 2u
#define INTER_CORE_RX_IRQ 1u
#define S_BUF_LEN 16
#define M_BUF_LEN 256
#define L_BUF_LEN 4096
#define CTRL_CHAN_ID 0
#define CTRL_CHAN_SIZE 64

/* convenience wrappers for printing messages */
#define pr_fmt(fmt) "ipc-shm-us-app: %s(): "fmt
#define sample_err(fmt, ...) printf(pr_fmt(fmt), __func__, ##__VA_ARGS__)
#define sample_info(fmt, ...) printf("ipc-shm-us-app: "fmt, ##__VA_ARGS__)
#ifdef DEBUG
#define sample_dbg(fmt, ...) printf(pr_fmt(fmt), __func__, ##__VA_ARGS__)
#else
#define sample_dbg(fmt, ...)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

static int msg_sizes[IPC_SHM_MAX_POOLS] = {S_BUF_LEN};
static int msg_sizes_count = 1;

/**
 * struct ipc_sample_app - sample app private data
 * @num_channels:	number of channels configured in this sample
 * @num_msgs:		number of messages to be sent to remote app
 * @ctrl_shm:		control channel local shared memory
 * @last_tx_msg:	last transmitted message
 * @last_rx_msg:	last received message
 * @sema:		binary semaphore for sync send_msg func with shm_rx_cb
 */
static struct ipc_sample_app {
	int num_channels;
	int num_msgs;
	char *ctrl_shm;
	char last_tx_msg[L_BUF_LEN];
	char last_rx_msg[L_BUF_LEN];
	sem_t sema;
} app;

static void data_chan_rx_cb(void *cb_arg, int chan_id, void *buf, size_t size);
static void ctrl_chan_rx_cb(void *cb_arg, int chan_id, void *mem);

/* Init IPC shared memory driver (see ipc-shm.h for API) */
static int init_ipc_shm(void)
{
	int err;

	/* memory buffer pools */
	struct ipc_shm_pool_cfg buf_pools[] = {
		{
			.num_bufs = 5,
			.buf_size = S_BUF_LEN
		},
		{
			.num_bufs = 5,
			.buf_size = M_BUF_LEN
		},
		{
			.num_bufs = 5,
			.buf_size = L_BUF_LEN
		},
	};

	/* data channel configuration */
	struct ipc_shm_channel_cfg data_chan_cfg = {
		.type = IPC_SHM_MANAGED,
		.ch = {
			.managed = {
				.num_pools = ARRAY_SIZE(buf_pools),
				.pools = buf_pools,
				.rx_cb = data_chan_rx_cb,
				.cb_arg = &app,
			},
		}
	};

	/* control channel configuration */
	struct ipc_shm_channel_cfg ctrl_chan_cfg = {
		.type = IPC_SHM_UNMANAGED,
		.ch = {
			.unmanaged = {
				.size = CTRL_CHAN_SIZE,
				.rx_cb = ctrl_chan_rx_cb,
				.cb_arg = &app,
			},
		}
	};

	/* use same configuration for all data channels */
	struct ipc_shm_channel_cfg channels[] = {ctrl_chan_cfg,
						 data_chan_cfg, data_chan_cfg};

	/* ipc shm configuration */
	struct ipc_shm_cfg shm_cfg = {
		.local_shm_addr = LOCAL_SHM_ADDR,
		.remote_shm_addr = REMOTE_SHM_ADDR,
		.shm_size = IPC_SHM_SIZE,
		.inter_core_tx_irq = INTER_CORE_TX_IRQ,
		.inter_core_rx_irq = INTER_CORE_RX_IRQ,
		.remote_core = {
			.type = IPC_CORE_DEFAULT,
			.index = 0,
		},
		.num_channels = ARRAY_SIZE(channels),
		.channels = channels
	};

	err = ipc_shm_init(&shm_cfg);
	if (err)
		return err;

	app.num_channels = shm_cfg.num_channels;

	/* acquire control channel memory once */
	app.ctrl_shm = ipc_shm_unmanaged_acquire(CTRL_CHAN_ID);
	if (!app.ctrl_shm) {
		sample_err("failed to get memory of control channel");
		return -ENOMEM;
	}

	return 0;
}

/*
 * data channel Rx callback: print message, release buffer and signal the
 * completion variable.
 */
static void data_chan_rx_cb(void *arg, int chan_id, void *buf, size_t size)
{
	int err = 0;

	assert(arg == &app);
	assert(size <= L_BUF_LEN);

	/* process the received data */
	memcpy(app.last_rx_msg, buf, size);
	sample_info("ch %d << %ld bytes: %s\n", chan_id, size, app.last_rx_msg);

	/* release the buffer */
	err = ipc_shm_release_buf(chan_id, buf);
	if (err) {
		sample_err("failed to free buffer for channel %d,"
			    "err code %d\n", chan_id, err);
	}

	/* notify send_msg function a reply was received */
	sem_post(&app.sema);
}

/*
 * control channel Rx callback: print control message
 */
static void ctrl_chan_rx_cb(void *arg, int chan_id, void *mem)
{
	/* temp buffer for string operations that do unaligned SRAM accesses */
	char tmp[CTRL_CHAN_SIZE] = {0};

	assert(arg == &app);
	assert(chan_id == CTRL_CHAN_ID);
	assert(strlen(mem) <= L_BUF_LEN);

	memcpy(tmp, mem, CTRL_CHAN_SIZE);
	sample_info("ch %d << %ld bytes: %s\n",
		    chan_id, strlen(tmp), (char *)tmp);

	/* notify run_demo() the ctrl reply was received and demo can end */
	sem_post(&app.sema);
}

/* send control message with number of data messages to be sent */
static int send_ctrl_msg(void)
{
	/* last channel is control channel */
	const int chan_id = CTRL_CHAN_ID;
	/* temp buffer for string operations that do unaligned SRAM accesses */
	char tmp[CTRL_CHAN_SIZE] = {0};
	int err;

	/* Write number of messages to be sent in control channel memory */
	sprintf(tmp, "SENDING MESSAGES: %d", app.num_msgs);
	memcpy(app.ctrl_shm, tmp, CTRL_CHAN_SIZE);

	sample_info("ch %d >> %ld bytes: %s\n", chan_id, strlen(tmp), tmp);

	/* notify remote */
	err = ipc_shm_unmanaged_tx(chan_id);
	if (err) {
		sample_err("tx failed on control channel");
		return err;
	}

	return 0;
}

/**
 * generate_msg() - generates message with fixed pattern
 * @dest:	destination buffer
 * @len:	destination buffer length
 * @msg_no:	message number
 *
 * Generate message by repeated concatenation of a pattern
 */
static void generate_msg(void *dest, int len, int msg_no)
{
	static char *pattern = "Hello world! ";
	/* temp buffer for string operations that do unaligned SRAM accesses */
	char tmp[L_BUF_LEN] = {0};

	sprintf(tmp, "#%d ", msg_no);

	/* copy pattern as many times it fits in dest buffer */
	while (strlen(tmp) < len) {
		strncpy(tmp + strlen(tmp), pattern, len - strlen(tmp));
	}
	tmp[len - 1] = '\0';

	memcpy(dest, tmp, len);
}

/**
 * send_data_msg() - Send generated data message to remote peer
 * @msg_len: message length
 * @msg_no: message sequence number to be written in the test message
 * @chan_id: ipc channel to be used for remote CPU communication
 *
 * It uses a completion variable for synchronization with reply callback.
 */
static int send_data_msg(int msg_len, int msg_no, int chan_id)
{
	int err = 0;
	char *buf = NULL;

	buf = ipc_shm_acquire_buf(chan_id, msg_len);
	if (!buf) {
		sample_err("failed to get buffer for channel ID"
			   " %d and size %d\n", chan_id, msg_len);
		return -ENOMEM;
	}

	/* write data to acquired buffer */
	generate_msg(buf, msg_len, msg_no);

	/* save data for comparison with echo reply */
	memcpy(app.last_tx_msg, buf, msg_len);

	sample_info("ch %d >> %d bytes: %s\n", chan_id, msg_len,
		    app.last_tx_msg);

	/* send data to remote peer */
	err = ipc_shm_tx(chan_id, buf, msg_len);
	if (err) {
		sample_err("tx failed for channel ID %d, size "
			   "%d, error code %d\n", 0, msg_len, err);
		return err;
	}

	/* wait for echo reply from remote (signaled from Rx callback) */
	sem_wait(&app.sema);
	if (errno == EINTR) {
		sample_info("interrupted...\n");
		return err;
	}

	/* check if received message match with sent message */
	if (strcmp(app.last_rx_msg, app.last_tx_msg) != 0) {
		sample_err("last rx msg != last tx msg\n");
		sample_err(">> %s\n", app.last_tx_msg);
		sample_err("<< %s\n", app.last_rx_msg);
		return -EINVAL;
	}

	return 0;
}

/*
 * Send requested number of messages to remote peer, cycling through all data
 * channels and wait for an echo reply after each sent message.
 */
static int run_demo(int num_msgs)
{
	int err, msg, ch, i;

	/* signal number of messages to remote via control channel */
	err = send_ctrl_msg();
	if (err)
		return err;

	/* send generated data messages */
	msg = 0;
	while (msg < num_msgs) {
		for (ch = CTRL_CHAN_ID + 1; ch < app.num_channels; ch++) {
			for (i = 0; i < msg_sizes_count; i++) {
				err = send_data_msg(msg_sizes[i], msg + 1, ch);
				if (err)
					return err;

				if (++msg == num_msgs) {
					/* wait for ctrl msg reply */
					sem_wait(&app.sema);
					return 0;
				}
			}
		}
	}

	return 0;
}

/*
 * interrupt signal handler for terminating the sample execution gracefully
 */
void int_handler(int signum)
{
	app.num_msgs = 0;
}

int main(int argc, char *argv[])
{
	int err = 0;
	struct sigaction sig_action;

	sem_init(&app.sema, 0, 0);

	/* init ipc shm driver */
	err = init_ipc_shm();
	if (err)
		return err;

	/* catch interrupt signals to terminate the execution gracefully */
	sig_action.sa_handler = int_handler;
	sigaction(SIGINT, &sig_action, NULL);

	app.num_msgs = 1;
	while (app.num_msgs) {
		printf("\nInput number of messages to send: ");
		scanf("%d", &app.num_msgs);

		if (!app.num_msgs)
			break;

		err = run_demo(app.num_msgs);
		if (err)
			break;
	}

	ipc_shm_free();

	sem_destroy(&app.sema);

	sample_info("exit\n");

	return err;
}
