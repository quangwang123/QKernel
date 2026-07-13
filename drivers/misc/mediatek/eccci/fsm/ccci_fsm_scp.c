// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 MediaTek Inc.
 */

#include "ccci_config.h"
#include "ccci_common_config.h"
#include "ccci_fsm_internal.h"

#ifdef FEATURE_SCP_CCCI_SUPPORT
#include <scp.h>


static atomic_t scp_state = ATOMIC_INIT(SCP_CCCI_STATE_INVALID);
static struct ccci_ipi_msg scp_ipi_tx_msg;
static struct mutex scp_ipi_tx_mutex;
static struct work_struct scp_ipi_rx_work;
static wait_queue_head_t scp_ipi_rx_wq;
static struct ccci_skb_queue scp_ipi_rx_skb_list;

static int ccci_scp_ipi_send(int md_id, int op_id, void *data)
{
	int ret = 0;

	if (atomic_read(&scp_state) == SCP_CCCI_STATE_INVALID) {
		((void)0);
		return -CCCI_ERR_MD_NOT_READY;
	}

	mutex_lock(&scp_ipi_tx_mutex);
	memset(&scp_ipi_tx_msg, 0, sizeof(scp_ipi_tx_msg));
	scp_ipi_tx_msg.md_id = md_id;
	scp_ipi_tx_msg.op_id = op_id;
	scp_ipi_tx_msg.data[0] = *((u32 *)data);
	((void)0);
	if (scp_ipi_send(IPI_APCCCI, &scp_ipi_tx_msg,
			sizeof(scp_ipi_tx_msg), 1, SCP_A_ID) != SCP_IPI_DONE) {
		((void)0);
		ret = -CCCI_ERR_MD_NOT_READY;
	}
	mutex_unlock(&scp_ipi_tx_mutex);
	return ret;
}

static void ccci_scp_md_state_sync_work(struct work_struct *work)
{
	struct ccci_fsm_scp *scp_ctl = container_of(work,
		struct ccci_fsm_scp, scp_md_state_sync_work);
	int ret;
	enum MD_STATE_FOR_USER state =
		ccci_fsm_get_md_state_for_user(scp_ctl->md_id);
	int count = 0;

	switch (state) {
	case MD_STATE_READY:
		switch (scp_ctl->md_id) {
		case MD_SYS1:
			while (count < SCP_BOOT_TIMEOUT/EVENT_POLL_INTEVAL) {
				if (atomic_read(&scp_state) ==
					SCP_CCCI_STATE_BOOTING)
					break;
				count++;
				msleep(EVENT_POLL_INTEVAL);
			}
			if (count == SCP_BOOT_TIMEOUT/EVENT_POLL_INTEVAL)
				((void)0);
			else {
				ret = ccci_port_send_msg_to_md(scp_ctl->md_id,
					CCCI_SYSTEM_TX, CCISM_SHM_INIT, 0, 1);
				if (ret < 0)
					((void)0);
			}
			break;
		case MD_SYS3:
			ret = ccci_port_send_msg_to_md(scp_ctl->md_id,
				CCCI_CONTROL_TX, C2K_CCISM_SHM_INIT, 0, 1);
			if (ret < 0)
				((void)0);
			break;
		};
		break;
	case MD_STATE_EXCEPTION:
		ccci_scp_ipi_send(scp_ctl->md_id,
			CCCI_OP_MD_STATE, &state);
		break;
	default:
		break;
	};
}

static void ccci_scp_ipi_rx_work(struct work_struct *work)
{
	struct ccci_ipi_msg *ipi_msg_ptr = NULL;
	struct sk_buff *skb = NULL;
	int data;

	while (!skb_queue_empty(&scp_ipi_rx_skb_list.skb_list)) {
		skb = ccci_skb_dequeue(&scp_ipi_rx_skb_list);
		if (skb == NULL) {
			((void)0);
			return;
		}
		ipi_msg_ptr = (struct ccci_ipi_msg *)skb->data;
		if (!get_modem_is_enabled(ipi_msg_ptr->md_id)) {
			((void)0);
			return;
		}
		switch (ipi_msg_ptr->op_id) {
		case CCCI_OP_SCP_STATE:
			switch (ipi_msg_ptr->data[0]) {
			case SCP_CCCI_STATE_BOOTING:
				if (atomic_read(&scp_state) ==
					SCP_CCCI_STATE_RBREADY) {
					((void)0);
					ccci_port_send_msg_to_md(MD_SYS1,
					CCCI_SYSTEM_TX, CCISM_SHM_INIT, 0, 1);
					ccci_port_send_msg_to_md(MD_SYS3,
					CCCI_CONTROL_TX,
					C2K_CCISM_SHM_INIT, 0, 1);
				} else {
					((void)0);
				}
				/* too early to init share memory here,
				 * EMI MPU may not be ready yet
				 */
				break;
			case SCP_CCCI_STATE_RBREADY:
				switch (ipi_msg_ptr->md_id) {
				case MD_SYS1:
					ccci_port_send_msg_to_md(MD_SYS1,
					CCCI_SYSTEM_TX,
					CCISM_SHM_INIT_DONE, 0, 1);
					break;
				case MD_SYS3:
					ccci_port_send_msg_to_md(MD_SYS3,
					CCCI_CONTROL_TX,
					C2K_CCISM_SHM_INIT_DONE, 0, 1);
					break;
				};
				data =
				ccci_fsm_get_md_state_for_user(
					ipi_msg_ptr->md_id);
				ccci_scp_ipi_send(ipi_msg_ptr->md_id,
					CCCI_OP_MD_STATE, &data);
				break;
			default:
				break;
			};
			atomic_set(&scp_state, ipi_msg_ptr->data[0]);
			break;
		default:
			break;
		};
		ccci_free_skb(skb);
	}
}

static void ccci_scp_ipi_handler(int id, void *data, unsigned int len)
{
	struct ccci_ipi_msg *ipi_msg_ptr = (struct ccci_ipi_msg *)data;
	struct sk_buff *skb = NULL;

	if (len != sizeof(struct ccci_ipi_msg)) {
		((void)0);
		return;
	}
	((void)0);

	skb = ccci_alloc_skb(len, 0, 0);
	if (!skb)
		return;
	memcpy(skb_put(skb, len), data, len);
	ccci_skb_enqueue(&scp_ipi_rx_skb_list, skb);
	/* ipi_send use mutex, can not be called from ISR context */
	schedule_work(&scp_ipi_rx_work);
}
#endif

int fsm_ccism_init_ack_handler(int md_id, int data)
{
#ifdef FEATURE_SCP_CCCI_SUPPORT
	struct ccci_smem_region *ccism_scp =
		ccci_md_get_smem_by_user_id(md_id, SMEM_USER_CCISM_SCP);

	memset_io(ccism_scp->base_ap_view_vir, 0, ccism_scp->size);
	ccci_scp_ipi_send(md_id, CCCI_OP_SHM_INIT,
		&ccism_scp->base_ap_view_phy);
#endif
	return 0;
}

#ifdef CONFIG_MTK_SIM_LOCK_POWER_ON_WRITE_PROTECT
static int fsm_sim_lock_handler(int md_id, int data)
{
	fsm_monitor_send_message(md_id, CCCI_MD_MSG_RANDOM_PATTERN, 0);
	return 0;
}
#endif

static int fsm_sim_type_handler(int md_id, int data)
{
	struct ccci_per_md *per_md_data = ccci_get_per_md_data(md_id);

	per_md_data->sim_type = data;
	return 0;
}

#ifdef FEATURE_SCP_CCCI_SUPPORT
void fsm_scp_init0(void)
{
	mutex_init(&scp_ipi_tx_mutex);
	((void)0);
	if (scp_ipi_registration(IPI_APCCCI, ccci_scp_ipi_handler,
		"AP CCCI") != SCP_IPI_DONE)
		((void)0);
	INIT_WORK(&scp_ipi_rx_work, ccci_scp_ipi_rx_work);
	init_waitqueue_head(&scp_ipi_rx_wq);
	ccci_skb_queue_init(&scp_ipi_rx_skb_list, 16, 16, 0);
	atomic_set(&scp_state, SCP_CCCI_STATE_BOOTING);
}
EXPORT_SYMBOL(fsm_scp_init0);
#endif

int fsm_scp_init(struct ccci_fsm_scp *scp_ctl)
{
	struct ccci_fsm_ctl *ctl =
		container_of(scp_ctl, struct ccci_fsm_ctl, scp_ctl);
	int ret = 0;

	scp_ctl->md_id = ctl->md_id;
#ifdef FEATURE_SCP_CCCI_SUPPORT
	INIT_WORK(&scp_ctl->scp_md_state_sync_work,
		ccci_scp_md_state_sync_work);
	register_ccci_sys_call_back(scp_ctl->md_id, CCISM_SHM_INIT_ACK,
		fsm_ccism_init_ack_handler);
#endif
#ifdef CONFIG_MTK_SIM_LOCK_POWER_ON_WRITE_PROTECT
	register_ccci_sys_call_back(scp_ctl->md_id, SIM_LOCK_RANDOM_PATTERN,
		fsm_sim_lock_handler);
#endif
	register_ccci_sys_call_back(scp_ctl->md_id, MD_SIM_TYPE,
		fsm_sim_type_handler);

	return ret;
}

