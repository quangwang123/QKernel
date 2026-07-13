// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 MediaTek Inc.
 */

#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/sched/clock.h> /* local_clock() */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/stacktrace.h>

#include "mt-plat/mtk_ccci_common.h"
#include "ccci_config.h"
#include "ccci_common_config.h"
#include "ccci_bm.h"
#ifdef CCCI_BM_TRACE
#define CREATE_TRACE_POINTS
#include "ccci_bm_events.h"
#endif

/*#define CCCI_WP_DEBUG*/
/*#define CCCI_MEM_BM_DEBUG*/
/*#define CCCI_SAVE_STACK_TRACE*/

#define SKB_MAGIC_HEADER 0xF333F333
#define SKB_MAGIC_FOOTER 0xF444F444

struct ccci_skb_queue skb_pool_4K;
struct ccci_skb_queue skb_pool_16;

struct workqueue_struct *pool_reload_work_queue;

#ifdef CCCI_BM_TRACE
struct timer_list ccci_bm_stat_timer;
void ccci_bm_stat_timer_func(unsigned long data)
{
	trace_ccci_bm(req_pool.count, skb_pool_4K.skb_list.qlen,
		skb_pool_16.skb_list.qlen);
	mod_timer(&ccci_bm_stat_timer, jiffies + HZ / 2);
}
#endif

#ifdef CCCI_WP_DEBUG
#include <mt-plat/hw_watchpoint.h>

static struct wp_event wp_event;
static atomic_t hwp_enable = ATOMIC_INIT(0);

static int my_wp_handler(phys_addr_t addr)
{
	((void)0);
	dump_stack();
	/*BUG();*/

	/* re-enable the watchpoint,
	 * since the auto-disable is not working
	 */
	del_hw_watchpoint(&wp_event);

	return 0;
}

static void enable_watchpoint(void *address)
{
	int wp_err;

	if (atomic_read(&hwp_enable) == 0) {
		init_wp_event(&wp_event, (phys_addr_t) address,
			(phys_addr_t) address,
			WP_EVENT_TYPE_WRITE, my_wp_handler);
		atomic_set(&hwp_enable, 1);
		wp_err = add_hw_watchpoint(&wp_event);
		if (wp_err)
			((void)0);
	}
}
#endif

#ifdef CCCI_MEM_BM_DEBUG
static int is_in_ccci_skb_pool(struct sk_buff *skb)
{
	struct sk_buff *skb_p = NULL;

	for (skb_p = skb_pool_16.skb_list.next;
		skb_p != NULL && skb_p !=
			(struct sk_buff *)&skb_pool_16.skb_list;
		skb_p = skb_p->next) {
		if (skb == skb_p) {
			((void)0);
			return 1;
		}
	}
	for (skb_p = skb_pool_4K.skb_list.next;
		skb_p != NULL && skb_p !=
			(struct sk_buff *)&skb_pool_4K.skb_list;
		skb_p = skb_p->next) {
		if (skb == skb_p) {
			((void)0);
			return 1;
		}
	}
	return 0;
}

static int ccci_skb_addr_checker(struct sk_buff *skb)
{
	unsigned long skb_addr_value;
	unsigned long queue16_addr_value;
	unsigned long queue4k_addr_value;

	skb_addr_value = (unsigned long)skb;
	queue16_addr_value = (unsigned long)&skb_pool_16;
	queue4k_addr_value = (unsigned long)&skb_pool_4K;

	if ((skb_addr_value >= queue16_addr_value
			&& skb_addr_value <
			queue16_addr_value + sizeof(struct ccci_skb_queue))
		||
		(skb_addr_value >= queue4k_addr_value
			&& skb_addr_value <
			queue4k_addr_value + sizeof(struct ccci_skb_queue))
		) {
		((void)0);
		((void)0);

		return 1;
	}
	return 0;
}

void ccci_magic_checker(void)
{
	if (skb_pool_16.magic_header != SKB_MAGIC_HEADER ||
		skb_pool_16.magic_footer != SKB_MAGIC_FOOTER) {
		((void)0);
		ccci_mem_dump(-1, &skb_pool_16,
			sizeof(struct ccci_skb_queue));
		dump_stack();
	}

	if (skb_pool_4K.magic_header != SKB_MAGIC_HEADER ||
		skb_pool_4K.magic_footer != SKB_MAGIC_FOOTER) {
		((void)0);
		ccci_mem_dump(-1, &skb_pool_4K,
			sizeof(struct ccci_skb_queue));
		dump_stack();
	}
}
#endif

#ifdef CCCI_SAVE_STACK_TRACE
#define CCCI_TRACK_ADDRS_COUNT 8
#define CCCI_TRACK_HISTORY_COUNT 8

struct ccci_stack_trace {
	void *who;
	int cpu;
	int pid;
	unsigned long long when;
	unsigned long addrs[CCCI_TRACK_ADDRS_COUNT];
};

void ccci_get_back_trace(void *who, struct ccci_stack_trace *trace)
{
	struct stack_trace stack_trace;

	if (trace == NULL)
		return;
	stack_trace.max_entries = CCCI_TRACK_ADDRS_COUNT;
	stack_trace.nr_entries = 0;
	stack_trace.entries = trace->addrs;
	stack_trace.skip = 3;
	save_stack_trace(&stack_trace);
	trace->who = who;
	trace->when = sched_clock();
	trace->cpu = smp_processor_id();
	trace->pid = current->pid;
}
void ccci_print_back_trace(struct ccci_stack_trace *trace)
{
	int i;

	if (trace->who != NULL) {
		((void)0);
	}
	for (i = 0; i < CCCI_TRACK_ADDRS_COUNT; i++) {
		if (trace->addrs[i] != 0)
			((void)0);
	}
}

static unsigned int backtrace_idx;
static struct ccci_stack_trace
	backtrace_history[CCCI_TRACK_HISTORY_COUNT];

static void ccci_add_bt_hisory(void *ptr)
{
	ccci_get_back_trace(ptr, &backtrace_history[backtrace_idx]);
	backtrace_idx++;
	if (backtrace_idx == CCCI_TRACK_HISTORY_COUNT)
		backtrace_idx = 0;
}

static void ccci_print_bt_history(char *info)
{
	int i, k;

	((void)0);
	for (i = 0, k = backtrace_idx; i < CCCI_TRACK_HISTORY_COUNT; i++) {
		if (k == CCCI_TRACK_HISTORY_COUNT)
			k = 0;
		ccci_print_back_trace(&backtrace_history[k]);
		k++;
	}
}
#endif

static inline struct sk_buff *__alloc_skb_from_pool(int size)
{
	struct sk_buff *skb = NULL;

	if (size > SKB_16)
		skb = ccci_skb_dequeue(&skb_pool_4K);
	else if (size > 0)
		skb = ccci_skb_dequeue(&skb_pool_16);
	return skb;
}

static inline struct sk_buff *__alloc_skb_from_kernel(int size, gfp_t gfp_mask)
{
	struct sk_buff *skb = NULL;

	if (size > SKB_1_5K)
		skb = __dev_alloc_skb(SKB_4K, gfp_mask);
	else if (size > SKB_16)
		skb = __dev_alloc_skb(SKB_1_5K, gfp_mask);
	else if (size > 0)
		skb = __dev_alloc_skb(SKB_16, gfp_mask);
	if (!skb)
		((void)0);
	return skb;
}

struct sk_buff *ccci_skb_dequeue(struct ccci_skb_queue *queue)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&queue->skb_list.lock, flags);
	result = __skb_dequeue(&queue->skb_list);
	if (queue->max_occupied < queue->max_len - queue->skb_list.qlen)
		queue->max_occupied = queue->max_len - queue->skb_list.qlen;
	queue->deq_count++;
	if (queue->pre_filled && queue->skb_list.qlen <
		queue->max_len / RELOAD_TH)
		queue_work(pool_reload_work_queue, &queue->reload_work);
	spin_unlock_irqrestore(&queue->skb_list.lock, flags);

	return result;
}
EXPORT_SYMBOL(ccci_skb_dequeue);

void ccci_skb_enqueue(struct ccci_skb_queue *queue, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&queue->skb_list.lock, flags);
	if (queue->skb_list.qlen < queue->max_len) {
		queue->enq_count++;
		__skb_queue_tail(&queue->skb_list, newsk);
		if (queue->skb_list.qlen > queue->max_history)
			queue->max_history = queue->skb_list.qlen;

	} else {
		dev_kfree_skb_any(newsk);
	}
	spin_unlock_irqrestore(&queue->skb_list.lock, flags);
}

void ccci_skb_queue_init(struct ccci_skb_queue *queue, unsigned int skb_size,
	unsigned int max_len, char fill_now)
{
	int i;

	queue->magic_header = SKB_MAGIC_HEADER;
	queue->magic_footer = SKB_MAGIC_FOOTER;
#ifdef CCCI_WP_DEBUG
	if (((unsigned long)queue) == ((unsigned long)(&skb_pool_16))) {
		((void)0);
		enable_watchpoint(&queue->magic_footer);
	}
#endif
	skb_queue_head_init(&queue->skb_list);
	queue->max_len = max_len;
	if (fill_now) {
		for (i = 0; i < queue->max_len; i++) {
			struct sk_buff *skb =
				__alloc_skb_from_kernel(skb_size, GFP_KERNEL);

			if (skb != NULL)
				skb_queue_tail(&queue->skb_list, skb);
		}
		queue->pre_filled = 1;
	} else {
		queue->pre_filled = 0;
	}
	queue->max_history = 0;
}
EXPORT_SYMBOL(ccci_skb_queue_init);

/* may return NULL, caller should check, network should always use blocking
 * as we do not want it consume our own pool
 */
struct sk_buff *ccci_alloc_skb(int size, unsigned char from_pool,
	unsigned char blocking)
{
	int count = 0;
	struct sk_buff *skb = NULL;
	struct ccci_buffer_ctrl *buf_ctrl = NULL;

	if (size > SKB_4K || size < 0)
		goto err_exit;

	if (from_pool) {
 slow_retry:
		skb = __alloc_skb_from_pool(size);
		if (unlikely(!skb && blocking)) {
			((void)0);
			msleep(100);
			goto slow_retry;
		}
		if (likely(skb && skb_headroom(skb) == NET_SKB_PAD)) {
			buf_ctrl = (struct ccci_buffer_ctrl *)skb_push(skb,
			sizeof(struct ccci_buffer_ctrl));
			buf_ctrl->head_magic = CCCI_BUF_MAGIC;
			buf_ctrl->policy = RECYCLE;
			buf_ctrl->ioc_override = 0x0;
			skb_pull(skb, sizeof(struct ccci_buffer_ctrl));
			((void)0);

		} else {
			((void)0);
		}
	} else {
		if (blocking) {
			skb = __alloc_skb_from_kernel(size, GFP_KERNEL);
		} else {
 fast_retry:
			skb = __alloc_skb_from_kernel(size, GFP_ATOMIC);
			if (!skb && count++ < 20)
				goto fast_retry;
		}
	}
 err_exit:
	if (unlikely(!skb))
		((void)0);

	return skb;
}
EXPORT_SYMBOL(ccci_alloc_skb);

void ccci_free_skb(struct sk_buff *skb)
{
	struct ccci_buffer_ctrl *buf_ctrl = NULL;
	enum DATA_POLICY policy = FREE;

	/*skb is onlink from caller cldma_gpd_bd_tx_collect*/
	buf_ctrl = (struct ccci_buffer_ctrl *)(skb->head + NET_SKB_PAD -
		sizeof(struct ccci_buffer_ctrl));
	if (buf_ctrl->head_magic == CCCI_BUF_MAGIC) {
		policy = buf_ctrl->policy;
		memset(buf_ctrl, 0, sizeof(*buf_ctrl));
	}
	if (policy != RECYCLE || skb->dev != NULL ||
		skb_size(skb) < NET_SKB_PAD + SKB_16)
		policy = FREE;

	((void)0);
	switch (policy) {
	case RECYCLE:
		/* 1. reset sk_buff (take __alloc_skb as ref.) */
		skb->data = skb->head;
		skb->len = 0;
		skb_reset_tail_pointer(skb);
		/*reserve memory as netdev_alloc_skb*/
		skb_reserve(skb, NET_SKB_PAD);
		/* 2. enqueue */
		if (skb_size(skb) < SKB_4K)
			ccci_skb_enqueue(&skb_pool_16, skb);
		else
			ccci_skb_enqueue(&skb_pool_4K, skb);
		break;
	case FREE:
		dev_kfree_skb_any(skb);
		break;
	default:
		/*default free skb to avoid memory leak*/
		dev_kfree_skb_any(skb);
		break;
	};
}
EXPORT_SYMBOL(ccci_free_skb);

void ccci_dump_skb_pool_usage(int md_id)
{
	((void)0);
	((void)0);
	skb_pool_4K.max_occupied = 0;
	skb_pool_4K.enq_count = 0;
	skb_pool_4K.deq_count = 0;
	skb_pool_16.max_occupied = 0;
	skb_pool_16.enq_count = 0;
	skb_pool_16.deq_count = 0;
}

static void __4K_reload_work(struct work_struct *work)
{
	struct sk_buff *skb;

	((void)0);
	while (skb_pool_4K.skb_list.qlen < SKB_POOL_SIZE_4K) {
		skb = __alloc_skb_from_kernel(SKB_4K, GFP_KERNEL);
		if (skb)
			skb_queue_tail(&skb_pool_4K.skb_list, skb);
		else
			((void)0);
	}
}

static void __16_reload_work(struct work_struct *work)
{
	struct sk_buff *skb;

	((void)0);
	while (skb_pool_16.skb_list.qlen < SKB_POOL_SIZE_16) {
		skb = __alloc_skb_from_kernel(SKB_16, GFP_KERNEL);
		if (skb)
			skb_queue_tail(&skb_pool_16.skb_list, skb);
		else
			((void)0);
	}
}

/*
 * a write operation may block at 3 stages:
 * 1. ccci_alloc_req
 * 2. wait until the queue has available slot (threshold check)
 * 3. wait until the SDIO transfer is complete --> abandoned,
 * see the reason below.
 * the 1st one is decided by @blk1. and the 2nd and 3rd are decided by
 * @blk2, waiting on @wq.
 * NULL is returned if no available skb, even when you set blk1=1.
 *
 * we removed the wait_queue_head_t in ccci_request, so user can NOT wait
 * for certain request to be completed. this is because request will be
 * recycled and its state will be reset, so if a request is completed and
 * then used again, the poor guy who is waiting for it may never see
 * the state transition (FLYING->IDLE/COMPLETE->FLYING) and wait forever.
 */

void ccci_mem_dump(int md_id, void *start_addr, int len)
{
	unsigned int *curr_p = (unsigned int *)start_addr;
	unsigned char *curr_ch_p;
	int _16_fix_num = len / 16;
	int tail_num = len % 16;
	char buf[16];
	int i, j;

	if (curr_p == NULL) {
		((void)0);
		return;
	}
	if (len == 0) {
		((void)0);
		return;
	}

	((void)0);
	/* Fix section */
	for (i = 0; i < _16_fix_num; i++) {
		((void)0);
		curr_p += 4;
	}

	/* Tail section */
	if (tail_num > 0) {
		curr_ch_p = (unsigned char *)curr_p;
		for (j = 0; j < tail_num; j++) {
			buf[j] = *curr_ch_p;
			curr_ch_p++;
		}
		for (; j < 16; j++)
			buf[j] = 0;
		curr_p = (unsigned int *)buf;
		((void)0);
	}
}

void ccci_cmpt_mem_dump(int md_id, void *start_addr, int len)
{
	unsigned int *curr_p = (unsigned int *)start_addr;
	unsigned char *curr_ch_p;
	int _64_fix_num = len / 64;
	int tail_num = len % 64;
	char buf[64];
	int i, j;

	if (curr_p == NULL) {
		((void)0);
		return;
	}
	if (len == 0) {
		((void)0);
		return;
	}

	/* Fix section */
	for (i = 0; i < _64_fix_num; i++) {
		((void)0);
		curr_p += 64/4;
	}

	/* Tail section */
	if (tail_num > 0) {
		curr_ch_p = (unsigned char *)curr_p;
		for (j = 0; j < tail_num; j++) {
			buf[j] = *curr_ch_p;
			curr_ch_p++;
		}
		for (; j < 64; j++)
			buf[j] = 0;
		curr_p = (unsigned int *)buf;
		((void)0);
	}
}

void ccci_dump_skb(struct sk_buff *skb)
{
	ccci_mem_dump(-1, skb->data, skb->len > 32 ? 32 : skb->len);
}

int ccci_subsys_bm_init(void)
{
	/* init ccci_request */

	((void)0);
	/* init skb pool */
	ccci_skb_queue_init(&skb_pool_4K, SKB_4K, SKB_POOL_SIZE_4K, 1);
	ccci_skb_queue_init(&skb_pool_16, SKB_16, SKB_POOL_SIZE_16, 1);
	/* init pool reload work */
	pool_reload_work_queue = alloc_workqueue("pool_reload_work",
		WQ_UNBOUND | WQ_MEM_RECLAIM | WQ_HIGHPRI, 1);
	INIT_WORK(&skb_pool_4K.reload_work, __4K_reload_work);
	INIT_WORK(&skb_pool_16.reload_work, __16_reload_work);

#ifdef CCCI_BM_TRACE
	init_timer(&ccci_bm_stat_timer);
	ccci_bm_stat_timer.function = ccci_bm_stat_timer_func;
	mod_timer(&ccci_bm_stat_timer, jiffies + 10 * HZ);
#endif
	return 0;
}
