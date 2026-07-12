/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __CONN_MD_DUMP_H_
#define __CONN_MD_DUMP_H_

#include "conn_md_log.h"
#include "conn_md_exp.h"

#define LENGTH_PER_PACKAGE 8
#define NUMBER_OF_MSG_LOGGED 16

enum conn_md_msg_type {
	MSG_ENQUEUE = 1,
	MSG_DEQUEUE = 2,
	MSG_EN_DE_QUEUE = 3,
};

#ifdef CONFIG_MTK_ENABLE_GMO
struct conn_md_dmp_msg_log;

static inline struct conn_md_dmp_msg_log *conn_md_dmp_init(void)
{
	return NULL;
}

static inline int conn_md_dmp_deinit(struct conn_md_dmp_msg_log *p_log)
{
	return 0;
}

static inline int conn_md_dmp_in(struct ipc_ilm *p_ilm,
		enum conn_md_msg_type msg_type,
		struct conn_md_dmp_msg_log *p_msg_log)
{
	return 0;
}

static inline int conn_md_dmp_out(struct conn_md_dmp_msg_log *p_msg_log,
		uint32 src_id, uint32 dst_id)
{
	return 0;
}
#else
struct conn_md_dmp_msg_str {
	unsigned int sec;
	unsigned int usec;
	enum conn_md_msg_type type;
	struct ipc_ilm ilm;
	uint16 msg_len;
	uint8 data[LENGTH_PER_PACKAGE];
};

struct conn_md_dmp_msg_log {
	struct conn_md_dmp_msg_str msg[NUMBER_OF_MSG_LOGGED];
	uint16 in;
	uint16 out;
	uint32 size;
	struct mutex lock;
};

extern struct conn_md_dmp_msg_log *conn_md_dmp_init(void);
extern int conn_md_dmp_deinit(struct conn_md_dmp_msg_log *p_log);

extern int conn_md_dmp_in(struct ipc_ilm *p_ilm,
			enum conn_md_msg_type msg_type,
			struct conn_md_dmp_msg_log *p_msg_log);
extern int conn_md_dmp_out(struct conn_md_dmp_msg_log *p_msg_log,
			uint32 src_id, uint32 dst_id);
#endif

#endif
