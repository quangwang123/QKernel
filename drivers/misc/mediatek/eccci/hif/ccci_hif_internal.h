/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 MediaTek Inc.
 */

#ifndef __CCCI_HIF_INTERNAL_H__
#define __CCCI_HIF_INTERNAL_H__

#include "ccci_core.h"
#include "ccci_port.h"
#include "ccci_fsm.h"
#include "ccci_hif.h"
#include "ccci_modem.h"

//#if (MD_GENERATION >= 6295)
#define MAX_TXQ_NUM 16
#define MAX_RXQ_NUM 16
//#else
//#define MAX_TXQ_NUM 8
//#define MAX_RXQ_NUM 8
//#endif

extern void *ccci_hif[CCCI_HIF_NUM];
extern struct ccci_hif_ops *ccci_hif_op[CCCI_HIF_NUM];

struct ccci_hif_traffic {
		short seq_nums[2][CCCI_MAX_CH_NUM];

};


struct ccci_hif_ops {
	/* must-have */
	int (*send_skb)(unsigned char hif_id, int qno, struct sk_buff *skb,
		int skb_from_pool, int blocking);
	int (*give_more)(unsigned char hif_id, unsigned char qno);
	int (*write_room)(unsigned char hif_id, unsigned char qno);
	int (*start_queue)(unsigned char hif_id, unsigned char qno,
		enum DIRECTION dir);
	int (*stop_queue)(unsigned char hif_id, unsigned char qno,
		enum DIRECTION dir);
	int (*broadcast_state)(unsigned char hif_id, enum MD_STATE state);
	int (*dump_status)(unsigned char hif_id, enum MODEM_DUMP_FLAG dump_flag,
		void *buff, int length);
	int (*suspend)(unsigned char hif_id);
	int (*resume)(unsigned char hif_id);

	int (*init)(unsigned char md_id, unsigned int hif_flag);
	int (*late_init)(unsigned char hif_id);
	int (*start)(unsigned char hif_id);
	int (*pre_stop)(unsigned char hif_id);
	int (*stop)(unsigned char hif_id);
	int (*debug)(unsigned char hif_id, enum ccci_hif_debug_flg debug_id,
		int *paras);
	int (*send_data)(unsigned char hif_id, int channel_id);
	void* (*fill_rt_header)(unsigned char hif_id,
		int packet_size, unsigned int tx_ch, unsigned int txqno);

	int (*stop_for_ee)(unsigned char hif_id);
	int (*all_q_reset)(unsigned char hif_id);
	int (*clear_all_queue)(unsigned char hif_id, enum DIRECTION dir);
	int (*clear)(unsigned char hif_id);
	void (*set_clk_cg)(unsigned char md_id, unsigned int on);
	void (*hw_reset)(unsigned char md_id);
};

enum RX_COLLECT_RESULT {
	ONCE_MORE,
	ALL_CLEAR,
	LOW_MEMORY,
	ERROR_STOP,
};

#ifndef CCCI_KMODULE_ENABLE
static inline void *ccci_hif_get_by_id(unsigned char hif_id)
{
	if (hif_id >= CCCI_HIF_NUM) {
		((void)0);
		return NULL;
	} else
		return ccci_hif[hif_id];
}
#else
extern void *ccci_hif_get_by_id(unsigned char hif_id);
#endif

static inline void ccci_hif_queue_status_notify(int md_id, int hif_id,
	int qno, int dir, int state)
{
	return ccci_port_queue_status_notify(md_id, hif_id, qno,
		dir, state);
}


static inline void ccci_reset_seq_num(struct ccci_hif_traffic *traffic_info)
{
	/* it's redundant to use 2 arrays,
	 * but this makes sequence checking easy
	 */
	memset(traffic_info->seq_nums[OUT], 0,
		sizeof(traffic_info->seq_nums[OUT]));
	memset(traffic_info->seq_nums[IN], -1,
		sizeof(traffic_info->seq_nums[IN]));
}

/*
 * as one channel can only use one hardware queue,
 * so it's safe we call this function in hardware
 * queue's lock protection
 */
static inline void ccci_md_inc_tx_seq_num(unsigned char md_id,
	struct ccci_hif_traffic *traffic_info,
	struct ccci_header *ccci_h)
{
	if (ccci_h->channel >= ARRAY_SIZE(traffic_info->seq_nums[OUT])
		|| ccci_h->channel < 0) {
		((void)0);
		return;		/* for force assert channel, etc. */
	}
	ccci_h->seq_num = traffic_info->seq_nums[OUT][ccci_h->channel]++;
	ccci_h->assert_bit = 1;

	/* for rpx channel, can only set assert_bit when
	 * md is in single-task phase.
	 */
	/* when md is in multi-task phase, assert bit should be 0,
	 * since ipc task are preemptible
	 */
	if ((ccci_h->channel == CCCI_RPC_TX
		|| ccci_h->channel == CCCI_FS_TX)
		&& ccci_fsm_get_md_state(md_id) != BOOT_WAITING_FOR_HS2)
		ccci_h->assert_bit = 0;
}

static inline unsigned int ccci_md_get_seq_num(
	struct ccci_hif_traffic *traffic_info, enum DIRECTION dir,
	enum CCCI_CH ch)
{
	return traffic_info->seq_nums[dir][ch];
}

extern void ccci_hif_register(unsigned char hif_id, void *hif_per_data,
	struct ccci_hif_ops *ops);

#endif
