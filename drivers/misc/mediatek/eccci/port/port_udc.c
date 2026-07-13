// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 */
#include <linux/kthread.h>
#include "ccci_bm.h"
#include "port_proxy.h"
#include "port_udc.h"
#include "port_smem.h"

extern atomic_t udc_status;

#define MAX_QUEUE_LENGTH 16

#define Min(a, b) (a < b ? a : b)
#define MAX_PACKET_SIZE 2555872 /* 2.4375*1024*1024 -32 */

struct ap_md_rw_index *rw_index;
unsigned char *comp_data_buf_base, *uncomp_data_buf_base;
unsigned char *uncomp_cache_data_base;
struct udc_comp_req_t *req_des_0_base, *req_des_1_base;
struct udc_comp_rslt_t *rslt_des_0_base, *rslt_des_1_base;
static unsigned int total_comp_size;

void set_udc_status(struct sk_buff *skb)
{
	struct ccci_udc_cmd_rsp_t *udc_cmd_rsp =
		(struct ccci_udc_cmd_rsp_t *)skb->data;

	if (udc_cmd_rsp->udc_inst_id == 0 &&
		udc_cmd_rsp->udc_cmd == UDC_CMD_KICK)
		atomic_set(&udc_status, UDC_HighKick);
	if (udc_cmd_rsp->udc_cmd == UDC_CMD_DEACTV)
		atomic_set(&udc_status, UDC_DEACTV);
	if (udc_cmd_rsp->udc_cmd == UDC_CMD_DISC)
		atomic_set(&udc_status, UDC_DISCARD);
}

int udc_resp_msg_to_md(struct port_t *port,
	struct sk_buff *skb, int handle_udc_ret)
{
	int md_id = port->md_id;
	int data_len, ret;
	struct ccci_udc_cmd_rsp_t *udc_cmd_rsp =
		(struct ccci_udc_cmd_rsp_t *)skb->data;

	/* write back to modem */
	/* update message */
	udc_cmd_rsp->udc_cmd |= UDC_API_RESP_ID;
	data_len = sizeof(*udc_cmd_rsp);
	if (handle_udc_ret < 0) {
		udc_cmd_rsp->rslt = UDC_CMD_RSLT_ERROR;
		((void)0);
	} else
		udc_cmd_rsp->rslt = UDC_CMD_RSLT_OK;

	/* resize skb */
	((void)0);
	if (data_len > skb->len)
		skb_put(skb, data_len - skb->len);
	else if (data_len < skb->len)
		skb_trim(skb, data_len);
	/* update CCCI header */
	udc_cmd_rsp->header.channel = CCCI_UDC_TX;
	udc_cmd_rsp->header.data[1] = data_len;
	((void)0);
	/* switch to Tx request */
	ret = port_send_skb_to_md(port, skb, 1);

	((void)0);

	return ret;
}

void udc_cmd_check(struct port_t *port,
	struct sk_buff **skb_tmp, struct sk_buff **skb,
	u32 inst_id, struct udc_state_ctl *ctl)
{
	int md_id = port->md_id;
	unsigned long flags;
	struct ccci_udc_deactv_param_t *ccci_udc_deactv;
	struct ccci_udc_actv_param_t *ccci_udc_actv;
	int skb_len, skb_tmp_len, skb_tmp1_len;
	struct sk_buff *skb_tmp1 = NULL;
	u32 ins_id_tmp;

	ctl->last_state = ctl->curr_state;
	ctl->curr_state = atomic_read(&udc_status);

	switch (ctl->curr_state) {
	case UDC_DEACTV_DONE:
	case UDC_DISC_DONE:
		atomic_set(&udc_status, UDC_IDLE);
		ctl->curr_state = UDC_IDLE;
		break;
	case UDC_IDLE:
	case UDC_HandleHighKick:
	case UDC_KICKDEACTV:
		break;
	case UDC_HighKick:
	{
		if (inst_id == 0) {
			atomic_set(&udc_status, UDC_IDLE);
			ctl->curr_state = UDC_IDLE;
			break;
		}
		((void)0);
		if (!skb_queue_empty(&port->rx_skb_list)) {
			skb_len = (*skb)->len;
			skb_tmp_len = (*skb_tmp)->len;
			 /* resize skb */
			 ((void)0);
			if (skb_len > skb_tmp_len)
				skb_put(*skb_tmp, skb_len - skb_tmp_len);
			else if (skb_len < skb_tmp_len)
				skb_trim(*skb_tmp, skb_len);

			/* backup skb to skb_tmp */
			skb_copy_to_linear_data(*skb_tmp,
				(*skb)->data, skb_len);
			spin_lock_irqsave(&port->rx_skb_list.lock, flags);
			*skb = __skb_dequeue(&port->rx_skb_list);
			spin_unlock_irqrestore(&port->rx_skb_list.lock, flags);
			ctl->last_state = ctl->curr_state;
			atomic_set(&udc_status, UDC_HandleHighKick);
			ctl->curr_state = UDC_HandleHighKick;
		} else
			goto err;
		break;
	}
	case UDC_DEACTV:
	{
		if (!skb_queue_empty(&port->rx_skb_list)) {
			if (ctl->last_state == UDC_HandleHighKick) {
				skb_tmp1 = ccci_alloc_skb(
					sizeof(*ccci_udc_actv), 1, 1);
				if (unlikely(!skb_tmp1)) {
					((void)0);
					return;
				}
				/* backup skb_tmp to skb_tmp1 */
				skb_copy_to_linear_data(skb_tmp1,
					(*skb_tmp)->data, (*skb_tmp)->len);
				skb_put(skb_tmp1, (*skb_tmp)->len);
			}
			skb_len = (*skb)->len;
			skb_tmp_len = (*skb_tmp)->len;
			/* resize skb */
			((void)0);
			if (skb_len > skb_tmp_len)
				skb_put(*skb_tmp, skb_len - skb_tmp_len);
			else if (skb_len < skb_tmp_len)
				skb_trim(*skb_tmp, skb_len);

			/* backup skb to skb_tmp */
			skb_copy_to_linear_data(*skb_tmp,
				(*skb)->data, skb_len);
			spin_lock_irqsave(&port->rx_skb_list.lock, flags);
			/* dequeue */
			*skb = __skb_dequeue(&port->rx_skb_list);
			if ((*skb) == NULL) {
				((void)0);
				spin_unlock_irqrestore(&port->rx_skb_list.lock, flags);
				return;
			}
			spin_unlock_irqrestore(&port->rx_skb_list.lock, flags);
			ccci_udc_deactv
				= (struct ccci_udc_deactv_param_t *)
				(*skb)->data;
			ins_id_tmp = ccci_udc_deactv->udc_inst_id;
			if (ctl->last_state == UDC_HandleHighKick) {
				if (ins_id_tmp == 0) {
					skb_tmp_len = (*skb_tmp)->len;
					skb_tmp1_len = skb_tmp1->len;
					if (skb_tmp_len > skb_tmp1_len)
						skb_put(*skb_tmp,
						skb_tmp_len - skb_tmp1_len);
					else if (skb_tmp_len < skb_tmp1_len)
						skb_trim(*skb_tmp, skb_tmp_len);

					skb_copy_to_linear_data(*skb_tmp,
						skb_tmp1->data, skb_tmp1_len);
				}
				ccci_free_skb(skb_tmp1);
				ctl->last_state = ctl->curr_state;
				atomic_set(&udc_status, UDC_KICKDEACTV);
				ctl->curr_state = UDC_KICKDEACTV;
			} else {
				if (ins_id_tmp == inst_id) {
					ctl->last_state = ctl->curr_state;
					atomic_set(&udc_status,
						UDC_DEACTV_DONE);
					ctl->curr_state = UDC_DEACTV_DONE;
				} else {
					ctl->last_state = ctl->curr_state;
					atomic_set(&udc_status, UDC_KICKDEACTV);
					ctl->curr_state = UDC_KICKDEACTV;
				}
			}
		} else
			goto err;
		break;
	}
	case UDC_DISCARD:
	{
		if (!skb_queue_empty(&port->rx_skb_list)) {
			skb_len = (*skb)->len;
			skb_tmp_len = (*skb_tmp)->len;
			/* resize skb */
			((void)0);
			if (skb_len > skb_tmp_len)
				skb_put(*skb_tmp, skb_len - skb_tmp_len);
			else if (skb_len < skb_tmp_len)
				skb_trim(*skb_tmp, skb_len);

			/* backup skb to skb_tmp */
			skb_copy_to_linear_data(*skb_tmp,
				(*skb)->data, skb_len);
			spin_lock_irqsave(&port->rx_skb_list.lock, flags);
			/* dequeue */
			*skb = __skb_dequeue(&port->rx_skb_list);
			spin_unlock_irqrestore(&port->rx_skb_list.lock, flags);

			ctl->last_state = ctl->curr_state;
			atomic_set(&udc_status, UDC_DISC_DONE);
			ctl->curr_state = UDC_DISC_DONE;
		} else
			goto err;
		break;
	}
	default:
		((void)0);
		break;
	}
	if (ctl->last_state != ctl->curr_state)
		((void)0);
	return;
err:
	((void)0);
	atomic_set(&udc_status, UDC_IDLE);
	ctl->curr_state = UDC_IDLE;
}

int udc_actv_handler(struct z_stream_s *zcpr, enum udc_dict_opt_e dic_option,
	unsigned int buffer_size, u32 inst_id)
{
	int ret = 0;
	static struct udc_private_data my_param0, my_param1;

	if (inst_id == 0)
		ret = udc_init(zcpr, &my_param0);
	else if (inst_id == 1)
		ret = udc_init(zcpr, &my_param1);

	ret |= deflateInit2_cb(zcpr, Z_DEFAULT_COMPRESSION,
		Z_DEFLATED, buffer_size, 8, Z_FIXED);

	if (ret < 0) {
		((void)0);
		return ret;
	}
	if (dic_option == UDC_DICT_STD_FOR_SIP) {
		ret |= deflateSetDictionary_cb(zcpr,
			get_dictionary_content(dic_option),
			UDC_DICTIONARY_LENGTH);
		if (ret < 0) {
			((void)0);
		}
	}

	return ret;
}

/* phase out:when ap recevice deflateEnd cmd, call deflateEnd_cb directly */
int udc_deactv_handler(struct z_stream_s *zcpr, u32 inst_id)
{
	int deflate_end_flag = 0;
	struct udc_comp_req_t *req_des;
	struct udc_comp_rslt_t *rslt_des;
	unsigned int ap_read = 0, ap_write = 0, md_write = 0, md_read = 0;
	struct udc_comp_req_t *req_des_base = NULL;
	struct udc_comp_rslt_t *rslt_des_base = NULL;

	if (inst_id == 0) {
		req_des_base = req_des_0_base;
		rslt_des_base = rslt_des_0_base;
		ap_read = rw_index->md_des_ins0.read;
		md_write = rw_index->md_des_ins0.write;
		md_read = rw_index->ap_resp_ins0.read;
	} else if (inst_id == 1) {
		req_des_base = req_des_1_base;
		rslt_des_base = rslt_des_1_base;
		ap_read = rw_index->md_des_ins1.read;
		md_write = rw_index->md_des_ins1.write;
		md_read = rw_index->ap_resp_ins1.read;
	}

	while (ap_read != md_write) {
		if (inst_id == 0) {
			ap_read = rw_index->md_des_ins0.read;
			ap_write = rw_index->ap_resp_ins0.write;
			md_write = rw_index->md_des_ins0.write;
			md_read = rw_index->ap_resp_ins0.read;
		} else if (inst_id == 1) {
			ap_read = rw_index->md_des_ins1.read;
			ap_write = rw_index->ap_resp_ins1.write;
			md_write = rw_index->md_des_ins1.write;
			md_read = rw_index->ap_resp_ins1.read;
		}

		/* req_des table is only 4kb */
		req_des = req_des_base + ap_read;
		/* md_write must be <=511 */
		ap_read = (ap_read + 1) % 512;
		/* dump req_des */
		((void)0);
		if (req_des->con == 0) {
			if ((ap_write+1) == md_read) {
				((void)0);
				((void)0);
				break;
			}
			rslt_des = rslt_des_base + ap_write;
			rslt_des->sdu_idx = req_des->sdu_idx;
			rslt_des->sit_type = req_des->sit_type;
			rslt_des->udc = 0;

			ap_write = (ap_write + 1) % 512;
			/* insure sequential execution */
			mb();
			if (inst_id == 0) {
				rw_index->md_des_ins0.read = ap_read;
				rw_index->ap_resp_ins0.write = ap_write;
			} else if (inst_id == 1) {
				rw_index->md_des_ins1.read = ap_read;
				rw_index->ap_resp_ins1.write = ap_write;
			}
			((void)0);
		} else {
			if (inst_id == 0)
				rw_index->md_des_ins0.read = ap_read;
			else if (inst_id == 1)
				rw_index->md_des_ins1.read = ap_read;
		}
	}
	deflate_end_flag = deflateEnd_cb(zcpr);
	udc_deinit(zcpr);

	((void)0);
	if (deflate_end_flag < 0) {
		/* the continuous input is unprocessed,
		 *it maybe return -3
		 */
		if (deflate_end_flag == -3)
			return 0;
		else
			return deflate_end_flag;
	}
	return deflate_end_flag;
}

static void ccci_udc_req_data_dump(u32 inst_id,
	unsigned char *uncomp_data, unsigned int uncomp_len)
{
#ifdef UDC_DATA_DUMP
	int j = 0;

	((void)0);
	for (j = 0; j < 16; j++) {
		if (j % 16 == 0)
			((void)0);
		((void)0);
		if (j == 15)
			((void)0);
	}
#endif
}

static void ccci_udc_rslt_data_dump(u32 inst_id,
	unsigned char *comp_data, unsigned int comp_len)
{
#ifdef UDC_DATA_DUMP
	unsigned int ap_read, ap_write, md_read, md_write;
	unsigned int j = 0;

	if (inst_id == 0) {
		ap_read = rw_index->md_des_ins0.read;
		ap_write = rw_index->ap_resp_ins0.write;
		md_read = rw_index->ap_resp_ins0.read;
		md_write = rw_index->md_des_ins0.write;
	} else if (inst_id == 1) {
		ap_read = rw_index->md_des_ins1.read;
		ap_write = rw_index->ap_resp_ins1.write;
		md_read = rw_index->ap_resp_ins1.read;
		md_write = rw_index->md_des_ins1.write;
	}

	((void)0);
	((void)0);

	for (j = 0; j < comp_len; j++) {
		if (j % 16 == 0) {
			if (j > 0)
				((void)0);
			else
				((void)0);
		}
		((void)0);
		if (j == (comp_len - 1))
			((void)0);
	}
#endif
}

/* <0:full 0:not full */
static int check_cmp_buf(u32 inst_id,
	int max_output_size)
{
	unsigned int ap_read = 0, ap_write = 0, md_read = 0, md_read_len = 0;
	struct udc_comp_req_t *req_des, *req_des_base = NULL;
	struct udc_comp_rslt_t *rslt_des, *rslt_des_base = NULL;

	if (inst_id == 0) {
		req_des_base = req_des_0_base;
		rslt_des_base = rslt_des_0_base;
		ap_read = rw_index->md_des_ins0.read;
		ap_write = rw_index->ap_resp_ins0.write;
		md_read = rw_index->ap_resp_ins0.read;
	} else if (inst_id == 1) {
		req_des_base = req_des_1_base;
		rslt_des_base = rslt_des_1_base;
		ap_read = rw_index->md_des_ins1.read;
		ap_write = rw_index->ap_resp_ins1.write;
		md_read = rw_index->ap_resp_ins1.read;
	} else {
		((void)0);
		return -1;
	}

	md_read_len = (rslt_des_base + md_read)->cmp_addr
		+ (rslt_des_base + md_read)->cmp_len;
	if (total_comp_size < md_read_len) {
		if ((total_comp_size + max_output_size)
				>= md_read_len) {
			((void)0);
			((void)0);
			req_des = req_des_base + ap_read;
			rslt_des = rslt_des_base + ap_write;

			rslt_des->sdu_idx = req_des->sdu_idx;
			rslt_des->sit_type = req_des->sit_type;
			rslt_des->udc = 0;

			/* insure sequential execution */
			mb();
			/* update ap write index */
			ap_write = (ap_write + 1) % 512;
			if (inst_id == 0)
				rw_index->ap_resp_ins0.write = ap_write;
			else if (inst_id == 1)
				rw_index->ap_resp_ins1.write = ap_write;
			return -CMP_BUF_FULL;
		}
	}
	return 0;
}

static int cal_udc_param(struct z_stream_s *zcpr, u32 inst_id,
	int *max_output_size, int *udc_chksum)
{
	struct udc_comp_req_t *req_des_tmp = NULL, *req_des_base = NULL;
	unsigned int ap_read = 0, md_write = 0;
	unsigned int uncomp_len_total = 0;
	int j = 0;

	if (inst_id == 0) {
		req_des_base = req_des_0_base;
		ap_read = rw_index->md_des_ins0.read;
		md_write = rw_index->md_des_ins0.write;
	} else if (inst_id == 1) {
		req_des_base = req_des_1_base;
		ap_read = rw_index->md_des_ins1.read;
		md_write = rw_index->md_des_ins1.write;
	} else {
		((void)0);
		return -1;
	}

	if (*max_output_size == 0) {
		req_des_tmp = req_des_base + ap_read;
		if (!req_des_tmp) {
			((void)0);
			return -1;
		}
		if (req_des_tmp->con == 0)
			*max_output_size = deflateBound_cb(zcpr,
			req_des_tmp->seg_len);
		else if (req_des_tmp->con == 1) {
			for (j = 0; ap_read != md_write; j++) {
				req_des_tmp = req_des_base + (ap_read+j)%512;
				uncomp_len_total += req_des_tmp->seg_len;
				if (req_des_tmp->con == 0)
					break;
			}
			*max_output_size =
				deflateBound_cb(zcpr, uncomp_len_total);
			((void)0);
			/* packet_count > 2*/
			if (j > 1)
				((void)0);
		}
		/* calc chksum before call deflate */
		*udc_chksum = udc_chksum_cb(zcpr);
		((void)0);
	}

	return 0;
}

int udc_deflate(struct z_stream_s *zcpr, u32 inst_id, u32 con,
	unsigned char *uncomp_data, unsigned int uncomp_len,
	unsigned char *comp_data, unsigned int remain_len)
{
	unsigned long bytes_processed, prev_bytes_processed;
	int deflate_st;
	int comp_len = 0;

	(*zcpr).next_in = uncomp_data;
	(*zcpr).next_out = comp_data;
	(*zcpr).avail_in = uncomp_len;
	(*zcpr).avail_out = remain_len;

	prev_bytes_processed = (*zcpr).total_in;
	deflate_st = deflate_cb(zcpr,
		con ? Z_NO_FLUSH : Z_SYNC_FLUSH);
	if (deflate_st < 0) {
		((void)0);
		if (deflate_st == Z_BUF_ERROR)
			((void)0);
		return deflate_st;
	}
	if ((*zcpr).avail_in > 0)
		((void)0);
	bytes_processed = (*zcpr).total_in - prev_bytes_processed;
	if (bytes_processed != uncomp_len)
		((void)0);
	comp_len = udc_GetCmpLen_cb(zcpr, comp_data, (*zcpr).next_out);
	total_comp_size += (*zcpr).next_out - comp_data;
	((void)0);

	return comp_len;
}

int udc_kick_handler(struct port_t *port, struct z_stream_s *zcpr,
	u32 inst_id, unsigned char **comp_data)
{
	int md_id = port->md_id;
	int ret = 0;
	static int max_output_size;
	int max_packet_size = MAX_PACKET_SIZE;
	static unsigned int udc_chksum;
	static unsigned int is_rst;
	unsigned int ap_read = 0, ap_write = 0, md_read = 0, md_write = 0;
	struct udc_comp_req_t *req_des = NULL, *req_des_base = NULL;
	struct udc_comp_rslt_t *rslt_des = NULL, *rslt_des_base = NULL;
	unsigned int uncomp_len, comp_len = 0;
	unsigned int remain_len;
	unsigned char *uncomp_data;
	/* reserved 8k for reduce memcpy op */
	unsigned int rsvd_len = MAX_PACKET_SIZE - 8*1024;

	if (inst_id == 0) {
		req_des_base = req_des_0_base;
		rslt_des_base = rslt_des_0_base;
		ap_read = rw_index->md_des_ins0.read;
		ap_write = rw_index->ap_resp_ins0.write;
		md_read = rw_index->ap_resp_ins0.read;
		md_write = rw_index->md_des_ins0.write;
	} else if (inst_id == 1) {
		req_des_base = req_des_1_base;
		rslt_des_base = rslt_des_1_base;
		ap_read = rw_index->md_des_ins1.read;
		ap_write = rw_index->ap_resp_ins1.write;
		md_read = rw_index->ap_resp_ins1.read;
		md_write = rw_index->md_des_ins1.write;
	} else {
		((void)0);
		return -1;
	}

	/* check if cmp_rslt table is full */
	if ((ap_write+1) == md_read) {
		((void)0);
		((void)0);
		return -CMP_RSLT_FULL;
	}
	/* req_des table is only 4kb */
	req_des = req_des_base + ap_read;
	if (!req_des) {
		((void)0);
		return -1;
	}
	/* dump req_des */
	((void)0);
	uncomp_len = req_des->seg_len;
	if (req_des->buf_type == 0)
		uncomp_data = (unsigned char *)
			((unsigned long)uncomp_data_buf_base +
			req_des->seg_phy_addr);
	else if (req_des->buf_type == 1)
		uncomp_data = (unsigned char *)
			((unsigned long)uncomp_cache_data_base +
			req_des->seg_phy_addr);

	ccci_udc_req_data_dump(inst_id, uncomp_data, uncomp_len);

	if (req_des->rst == 1) {
		is_rst = 1;
		((void)0);
		deflateReset_cb(zcpr);
	}

	/* check max_output_size&udc_chksum */
	cal_udc_param(zcpr, inst_id, &max_output_size, &udc_chksum);

	/* md_write must be <=511 */
	ap_read = (ap_read + 1) % 512;

	if (inst_id == 0)
		rw_index->md_des_ins0.read = ap_read;
	else if (inst_id == 1)
		rw_index->md_des_ins1.read = ap_read;

	/* deinit comp_data to reduce memcpy */
	if (total_comp_size >= rsvd_len ||
		(total_comp_size + max_output_size) > max_packet_size) {
		((void)0);
		*comp_data = comp_data_buf_base;
		total_comp_size = 0;
	}
	/* cal md_read_len for check if cmp_buf is full or not */
	ret = check_cmp_buf(inst_id, max_output_size);
	if (ret == -CMP_BUF_FULL)
		return ret;
	remain_len = Min(max_output_size,
		max_packet_size - total_comp_size);

	ret = udc_deflate(zcpr, inst_id, req_des->con,
		uncomp_data, uncomp_len, *comp_data, remain_len);
	if (ret < 0)
		return ret;
	comp_len += ret;

	if (req_des->con == 0) {
		rslt_des = rslt_des_base + ap_write;
		rslt_des->sdu_idx = req_des->sdu_idx;
		rslt_des->sit_type = req_des->sit_type;
		rslt_des->udc = 1;
		rslt_des->rst = is_rst;
		rslt_des->cksm = udc_chksum;
		rslt_des->cmp_addr = *comp_data - comp_data_buf_base;
		rslt_des->cmp_len = comp_len;

		((void)0);

		if (comp_len == 0) {
			/* if no check comp_len,ke will happen */
			((void)0);
			return -CMP_ZERO_LEN;
		}

		comp_len = 0;
		is_rst = 0;
		max_output_size = 0;
		/* update ap write index */
		ap_write = (ap_write + 1) % 512;
		/* insure sequential execution */
		mb();
		if (inst_id == 0)
			rw_index->ap_resp_ins0.write = ap_write;
		else if (inst_id == 1)
			rw_index->ap_resp_ins1.write = ap_write;
		ccci_udc_rslt_data_dump(inst_id,
			*comp_data, rslt_des->cmp_len);
	}
	*comp_data = (*zcpr).next_out;

	return ret;
}

int udc_restore_skb(struct port_t *port,
	struct udc_state_ctl *ctl,
	struct sk_buff **skb_tmp, struct sk_buff **skb)
{
	struct ccci_udc_actv_param_t *ccci_udc_actv;
	int ret = 0;
	int md_id = port->md_id;

	ctl->last_state = ctl->curr_state;
	/* ctl->curr_state = atomic_read(&udc_status); */

	switch (ctl->curr_state) {
	case UDC_HandleHighKick:
	case UDC_DISC_DONE:
	case UDC_KICKDEACTV:
	case UDC_DEACTV_DONE:
	{
		*skb = ccci_alloc_skb(sizeof(*ccci_udc_actv), 1, 1);
		if (unlikely(!(*skb))) {
			((void)0);
			return ret;
		}

		skb_copy_to_linear_data(*skb,
			(*skb_tmp)->data, (*skb_tmp)->len);
		skb_put(*skb, (*skb_tmp)->len);

		ctl->last_state = ctl->curr_state;
		atomic_set(&udc_status, UDC_IDLE);
		ctl->curr_state = UDC_IDLE;
		ret = 1;
		break;
	}
	case UDC_IDLE:
	case UDC_HighKick:
	case UDC_DISCARD:
	case UDC_DEACTV:
		break;
	default:
		((void)0);
		break;
	}
	if (ctl->last_state != ctl->curr_state)
		((void)0);
	return ret;
}

void udc_cmd_handler(struct port_t *port, struct sk_buff *skb)
{
	int md_id = port->md_id;
	struct ccci_smem_region *region;
	int ret = 0;
	unsigned int udc_cmd = 0;
	struct udc_state_ctl *ctl;
	static unsigned char *comp_data;
	struct ccci_udc_deactv_param_t *ccci_udc_deactv;
	struct ccci_udc_disc_param_t *ccci_udc_disc;
	struct sk_buff *skb_tmp;
	struct ccci_udc_actv_param_t *ccci_udc_actv;
	static struct z_stream_s zcpr0, zcpr1;
	int deflate_end_flag = 0;
	unsigned int md_write = 0, ap_read = 0;

	skb_tmp = ccci_alloc_skb(sizeof(*ccci_udc_actv), 1, 1);
	if (!skb_tmp) {
		((void)0);
		return;
	}

	ctl = kzalloc(sizeof(struct udc_state_ctl), GFP_KERNEL);

	ccci_udc_actv = (struct ccci_udc_actv_param_t *)skb->data;
	udc_cmd = ccci_udc_actv->udc_cmd;
	((void)0);

	switch (udc_cmd) {
	case UDC_CMD_ACTV:
	{
		unsigned int buffer_size = ccci_udc_actv->buf_sz;
		enum udc_dict_opt_e dic_option = ccci_udc_actv->dict_opt;
		unsigned int inst_id  = ccci_udc_actv->udc_inst_id;

		((void)0);

		if (inst_id == 0)
			ret = udc_actv_handler(&zcpr0, dic_option,
					buffer_size, inst_id);
		else if (inst_id == 1)
			ret = udc_actv_handler(&zcpr1, dic_option,
					buffer_size, inst_id);
		if (ret < 0)
			goto end;
		/* get sharememory info */
		region = ccci_md_get_smem_by_user_id(md_id,
					SMEM_USER_RAW_UDC_DATA);
		if (region) {
			uncomp_data_buf_base = (unsigned char *)
				region->base_ap_view_vir;
			comp_data_buf_base = (unsigned char *)
				(region->base_ap_view_vir + 0x500000+32);
			rw_index = (struct ap_md_rw_index *)
				(region->base_ap_view_vir + 0x500000);
			comp_data = comp_data_buf_base;
			((void)0);
			((void)0);
		} else
			((void)0);

		region = ccci_md_get_smem_by_user_id(md_id,
					SMEM_USER_RAW_UDC_DESCTAB);
		if (region) {
			uncomp_cache_data_base =
				(unsigned char *)region->base_ap_view_vir;
			/* cmp_req and cmp_rslt offset:48k */
			req_des_0_base = (struct udc_comp_req_t *)
				(region->base_ap_view_vir + 0xC000);
			rslt_des_0_base = (struct udc_comp_rslt_t *)
				(region->base_ap_view_vir + 0xC000 + 0x1000);
			req_des_1_base = (struct udc_comp_req_t *)
				(region->base_ap_view_vir + 0xC000 + 0x2000);
			rslt_des_1_base = (struct udc_comp_rslt_t *)
				(region->base_ap_view_vir + 0xC000 + 0x3000);
			((void)0);
		} else
			((void)0);
		break;
	}
	case UDC_CMD_DEACTV:
	{
		unsigned int inst_id;

deactive_exit:
		ccci_udc_deactv =
			(struct ccci_udc_deactv_param_t *)skb->data;

		udc_cmd = ccci_udc_deactv->udc_cmd;
		inst_id = ccci_udc_deactv->udc_inst_id;
		((void)0);

		if (inst_id == 0) {
			/* ret = udc_deactv_handler(&zcpr0, inst_id); */
			deflate_end_flag = deflateEnd_cb(&zcpr0);
			udc_deinit(&zcpr0);

		} else if (inst_id == 1) {
			/* ret = udc_deactv_handler(&zcpr1, inst_id); */
			deflate_end_flag = deflateEnd_cb(&zcpr1);
			udc_deinit(&zcpr1);
		}

		/* the continuous input is unprocessed, it maybe return -3 */
		if (deflate_end_flag < 0 && deflate_end_flag != -3) {
			ret = deflate_end_flag;
			((void)0);
		}

		ctl->curr_state = atomic_read(&udc_status);
		if (ctl->curr_state == UDC_DEACTV ||
			ctl->curr_state == UDC_DEACTV_DONE) {
			atomic_set(&udc_status, UDC_IDLE);
			ctl->last_state = ctl->curr_state;
			ctl->curr_state = UDC_IDLE;
		}
		break;
	}
	case UDC_CMD_DISC:
	{
		unsigned int inst_id;
		unsigned int new_req_r;

discard_req:
		ccci_udc_disc =
			(struct ccci_udc_disc_param_t *)skb->data;
		udc_cmd = ccci_udc_disc->udc_cmd;
		new_req_r = ccci_udc_disc->new_req_r;
		inst_id = ccci_udc_disc->udc_inst_id;
		((void)0);

		if (inst_id == 0) {
			ap_read = rw_index->md_des_ins0.read;
			rw_index->md_des_ins0.read =
				ccci_udc_disc->new_req_r;
			((void)0);
		} else if (inst_id == 1) {
			ap_read = rw_index->md_des_ins1.read;
			rw_index->md_des_ins1.read =
				ccci_udc_disc->new_req_r;
			((void)0);
		}
		ctl->curr_state = atomic_read(&udc_status);
		if (ctl->curr_state == UDC_DISCARD) {
			atomic_set(&udc_status, UDC_IDLE);
			ctl->last_state = ctl->curr_state;
			ctl->curr_state = UDC_IDLE;
		}
		break;
	}
	case UDC_CMD_KICK:
	{
		unsigned int inst_id, exp_timer;
		struct udc_comp_req_t *req_des, *req_des_base;
		struct ccci_udc_kick_param_t *ccci_udc_kick;

retry_kick:
		ccci_udc_kick =
			(struct ccci_udc_kick_param_t *)skb->data;
		inst_id = ccci_udc_kick->udc_inst_id;
		udc_cmd = ccci_udc_kick->udc_cmd;
		/* to do exp_timer does not work now */
		exp_timer = ccci_udc_kick->exp_tmr;

		((void)0);
		if (inst_id == 0) {
			req_des_base = req_des_0_base;
			ap_read = rw_index->md_des_ins0.read;
			md_write = rw_index->md_des_ins0.write;
		} else if (inst_id == 1) {
			req_des_base = req_des_1_base;
			ap_read = rw_index->md_des_ins1.read;
			md_write = rw_index->md_des_ins1.write;
		}

		while (ap_read != md_write) {
			if (inst_id == 0) {
				req_des = req_des_base + ap_read;
				ret = udc_kick_handler(port, &zcpr0,
						inst_id, &comp_data);
				if (ret < 0) {
					((void)0);
					goto end;
				}
				if (req_des->con == 0) {
					udc_cmd_check(port, &skb_tmp,
						&skb, inst_id, ctl);
					if (ctl->curr_state ==
						UDC_DEACTV_DONE ||
						ctl->curr_state ==
						UDC_KICKDEACTV) {
						((void)0);
						goto deactive_exit;
					} else if (ctl->curr_state ==
						UDC_DISC_DONE) {
						((void)0);
						goto discard_req;
					}
				}
				/* insure sequential execution */
				/* mb(); */
				ap_read = rw_index->md_des_ins0.read;
				md_write = rw_index->md_des_ins0.write;
			} else if (inst_id == 1) {
				req_des = req_des_base + ap_read;
				ret = udc_kick_handler(port, &zcpr1,
						inst_id, &comp_data);
				if (ret < 0) {
					((void)0);
					goto end;
				}
				if (req_des->con == 0) {
					udc_cmd_check(port, &skb_tmp,
						&skb, inst_id, ctl);
					if (ctl->curr_state ==
						UDC_HandleHighKick) {
						((void)0);
						goto retry_kick;
					} else if (ctl->curr_state ==
						UDC_DEACTV_DONE ||
						ctl->curr_state ==
						UDC_KICKDEACTV) {
						((void)0);
						goto deactive_exit;
					} else if (ctl->curr_state ==
						UDC_DISC_DONE) {
						((void)0);
						goto discard_req;
					}
				}

				/* insure sequential execution */
				/* mb(); */
				ap_read = rw_index->md_des_ins1.read;
				md_write = rw_index->md_des_ins1.write;
			}
		}
		break;
	}
	default:
		((void)0);
		break;
	}
end:
	/* resp_to_md */
	ret = udc_resp_msg_to_md(port, skb, ret);
	if (ret < 0)
		((void)0);
	((void)0);
	/* dump read write index */
	((void)0);
	if (udc_restore_skb(port, ctl, &skb_tmp, &skb)) {
		((void)0);
		goto retry_kick;
	}
	ccci_free_skb(skb_tmp);
	kfree(ctl);
}

static int port_udc_init(struct port_t *port)
{
	((void)0);
	port->skb_handler = &udc_cmd_handler;
	port->private_data = kthread_run(port_kthread_handler,
		port, "%s", port->name);
	port->rx_length_th = MAX_QUEUE_LENGTH;
	port->skb_from_pool = 1;
	return 0;
}

struct port_ops ccci_udc_port_ops = {
	.init = &port_udc_init,
	.recv_skb = &port_recv_skb,
};

