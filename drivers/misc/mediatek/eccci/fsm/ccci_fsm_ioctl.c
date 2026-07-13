// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 MediaTek Inc.
 */

#ifdef CONFIG_MTK_SIM_LOCK_POWER_ON_WRITE_PROTECT
/* #include <mt-plat/env.h> Fix me, header file not found */
#endif
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#include "ccci_auxadc.h"
#include "ccci_fsm_internal.h"
#include "ccci_platform.h"
#include "modem_sys.h"
#include "md_sys1_platform.h"

signed int __weak battery_get_bat_voltage(void)
{
	pr_debug("[ccci/dummy] %s is not supported!\n", __func__);
	return 0;
}

#ifdef CCCI_KMODULE_ENABLE
int switch_sim_mode(int id, char *buf, unsigned int len)
{
	pr_debug("[ccci/dummy] %s is not supported!\n", __func__);
	return 0;
}

unsigned int get_sim_switch_type(void)
{
	pr_debug("[ccci/dummy] %s is not supported!\n", __func__);
	return 0;
}
#endif

static int fsm_md_data_ioctl(int md_id, unsigned int cmd, unsigned long arg)
{
	int ret = 0, retry;
	int data;
	char buffer[64];
	unsigned int sim_slot_cfg[4];
	char ap_platform[5];
	int md_gen = 0;
	struct device_node *node = NULL;
	struct ccci_per_md *per_md_data = ccci_get_per_md_data(md_id);
	struct ccci_per_md *other_per_md_data
			= ccci_get_per_md_data(GET_OTHER_MD_ID(md_id));

	node = of_find_compatible_node(NULL, NULL,
		"mediatek,mddriver");
	of_property_read_u32(node,
		"mediatek,md_generation", &md_gen);

	switch (cmd) {
	case CCCI_IOC_GET_MD_PROTOCOL_TYPE:
		snprintf(buffer, sizeof(buffer), "%d",
			md_gen);
		snprintf((void *)ap_platform, sizeof(ap_platform), "%d",
			md_gen);
		if (copy_to_user((void __user *)arg,
			ap_platform, sizeof(ap_platform))) {
			((void)0);
			return -EFAULT;
		}
		break;
	case CCCI_IOC_SEND_BATTERY_INFO:
		data = (int)battery_get_bat_voltage();
		((void)0);
		ret = ccci_port_send_msg_to_md(md_id, CCCI_SYSTEM_TX,
				MD_GET_BATTERY_INFO, data, 1);
		break;
	case CCCI_IOC_GET_EXT_MD_POST_FIX:
		if (copy_to_user((void __user *)arg,
				per_md_data->img_post_fix, IMG_POSTFIX_LEN)) {
			((void)0);
			ret = -EFAULT;
		}
		break;

	case CCCI_IOC_DL_TRAFFIC_CONTROL:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int)))
			((void)0);
		if (data == 1)
			;/* turn off downlink queue */
		else if (data == 0)
			;/* turn on donwlink queue */
		else
			;
		ret = 0;
		break;
#ifdef CONFIG_MTK_SIM_LOCK_POWER_ON_WRITE_PROTECT
#ifdef ENABLE_SIM_LOCK_RANDOM
	case CCCI_IOC_SIM_LOCK_RANDOM_PATTERN: /* Fix me */
		if (copy_from_user(&val, (void __user *)arg,
				sizeof(unsigned int)))
			((void)0);

		((void)0);

		snprintf(buffer, sizeof(buffer), "%x", data);
		set_env("sml_sync", buffer);
		break;
#endif
#endif
	case CCCI_IOC_SET_MD_BOOT_MODE:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
		} else {
			((void)0);
			per_md_data->md_boot_mode = data;
			if (other_per_md_data)
				other_per_md_data->md_boot_mode = data;
		}
		break;
	case CCCI_IOC_GET_MD_BOOT_MODE:
		ret = put_user((unsigned int)per_md_data->md_boot_mode,
				(unsigned int __user *)arg);
		break;
	case CCCI_IOC_GET_MD_INFO:
		ret = put_user(
		(unsigned int)per_md_data->img_info[IMG_MD].img_info.version,
		(unsigned int __user *)arg);
		break;
	case CCCI_IOC_SET_BOOT_DATA:
		if (copy_from_user(&per_md_data->md_boot_data,
			(void __user *)arg,
			sizeof(per_md_data->md_boot_data))) {
			((void)0);
			ret = -EFAULT;
		} else {
			if (per_md_data->md_boot_data[MD_CFG_DUMP_FLAG]
				!= MD_DBG_DUMP_INVALID
				&&
				(per_md_data->md_boot_data[MD_CFG_DUMP_FLAG]
				& 1 << MD_DBG_DUMP_PORT)) {
				/*port traffic use 0x6000_000x
				 * as port dump flag
				 */
				ccci_port_set_traffic_flag(md_id,
				per_md_data->md_boot_data[MD_CFG_DUMP_FLAG]);
				per_md_data->md_boot_data[MD_CFG_DUMP_FLAG]
					= MD_DBG_DUMP_INVALID;
			}
			ret = ccci_md_set_boot_data(md_id,
					per_md_data->md_boot_data,
					ARRAY_SIZE(per_md_data->md_boot_data));
			if (ret < 0) {
				((void)0);
				ret = -EFAULT;
			}
		}
		break;
	case CCCI_IOC_SIM_SWITCH:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
		} else {
			switch_sim_mode(md_id, (char *)&data, sizeof(data));
			((void)0);
		}
		break;
	case CCCI_IOC_SIM_SWITCH_TYPE:
		data = get_sim_switch_type();
		((void)0);
		ret = put_user(data, (unsigned int __user *)arg);
		break;
	case CCCI_IOC_GET_SIM_TYPE:
		if (per_md_data->sim_type == 0xEEEEEEEE)
			((void)0);
		else
			((void)0);
		ret = put_user(per_md_data->sim_type,
				(unsigned int __user *)arg);
		break;
	case CCCI_IOC_ENABLE_GET_SIM_TYPE:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
		} else {
			((void)0);
			ret = ccci_port_send_msg_to_md(md_id,
					CCCI_SYSTEM_TX, MD_SIM_TYPE, data, 1);
		}
		break;
	case CCCI_IOC_RELOAD_MD_TYPE:
		data = 0;
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
		} else {
			((void)0);
			/* add md type check to
			 * avoid it being changed to illegal value
			 */
			if (check_md_type(data) > 0) {
				if (set_modem_support_cap(md_id, data) == 0)
					per_md_data->config.load_type = data;
			} else {
				((void)0);
			}
		}
		break;
	case CCCI_IOC_SET_MD_IMG_EXIST:
		if (copy_from_user(&per_md_data->md_img_exist,
				(void __user *)arg,
				sizeof(per_md_data->md_img_exist))) {
			((void)0);
			ret = -EFAULT;
		}
		per_md_data->md_img_type_is_set = 1;
		((void)0);
		break;
	case CCCI_IOC_GET_MD_IMG_EXIST:
		data = get_md_type_from_lk(md_id);
		if (data) {
			memset(&per_md_data->md_img_exist, 0,
				sizeof(per_md_data->md_img_exist));
			per_md_data->md_img_exist[0] = data;
			((void)0);
		} else {
			((void)0);
			while (per_md_data->md_img_type_is_set == 0)
				msleep(200);
		}
		((void)0);
		if (copy_to_user((void __user *)arg,
			&per_md_data->md_img_exist,
			sizeof(per_md_data->md_img_exist))) {
			((void)0);
			ret = -EFAULT;
		}
		break;
	case CCCI_IOC_GET_MD_TYPE:
		retry = 600;
		do {
			data = get_legacy_md_type(md_id);
			if (data)
				break;
			msleep(500);
			retry--;
		} while (retry);
		((void)0);
		ret = put_user((unsigned int)data,
			(unsigned int __user *)arg);
		break;
	case CCCI_IOC_STORE_MD_TYPE:
		if (copy_from_user(&data, (void __user *)arg,
			sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
			break;
		}
		per_md_data->config.load_type_saving = data;

		((void)0);
		if (per_md_data->config.load_type_saving >= 1
			&& per_md_data->config.load_type_saving
			<= MAX_IMG_NUM) {
			if (per_md_data->config.load_type_saving
				!= per_md_data->config.load_type)
				((void)0);
		} else {
			((void)0);
			ret = -EFAULT;
		}
		if (ret == 0)
			fsm_monitor_send_message(md_id,
				CCCI_MD_MSG_STORE_NVRAM_MD_TYPE, 0);
		break;
	case CCCI_IOC_GET_MD_TYPE_SAVING:
		ret = put_user(per_md_data->config.load_type_saving,
				(unsigned int __user *)arg);
		break;
	case CCCI_IOC_SEND_ICUSB_NOTIFY:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
		} else {
			ret = ccci_port_send_msg_to_md(md_id,
			CCCI_SYSTEM_TX, MD_ICUSB_NOTIFY, data, 1);
		}
		break;
	case CCCI_IOC_UPDATE_SIM_SLOT_CFG:
		if (copy_from_user(&sim_slot_cfg, (void __user *)arg,
				sizeof(sim_slot_cfg))) {
			((void)0);
			ret = -EFAULT;
		} else {
			int need_update;

			data = get_sim_switch_type();
			((void)0);
			need_update = sim_slot_cfg[0];
			per_md_data->sim_setting.sim_mode = sim_slot_cfg[1];
			per_md_data->sim_setting.slot1_mode = sim_slot_cfg[2];
			per_md_data->sim_setting.slot2_mode = sim_slot_cfg[3];
			data = ((data << 16)
					| per_md_data->sim_setting.sim_mode);
			switch_sim_mode(md_id, (char *)&data, sizeof(data));
			fsm_monitor_send_message(md_id,
			CCCI_MD_MSG_CFG_UPDATE, need_update);
			ret = 0;
		}
		break;
	case CCCI_IOC_STORE_SIM_MODE:
		if (copy_from_user(&data, (void __user *)arg,
			sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
			break;
		}
		((void)0);
		if (per_md_data->sim_setting.sim_mode != data) {
			per_md_data->sim_setting.sim_mode = data;
			fsm_monitor_send_message(md_id,
			CCCI_MD_MSG_CFG_UPDATE, 1);
		} else {
			((void)0);
		}
		break;
	case CCCI_IOC_GET_SIM_MODE:
		((void)0);
		ret = put_user(per_md_data->sim_setting.sim_mode,
		(unsigned int __user *)arg);
		break;
	case CCCI_IOC_GET_CFG_SETTING:
		if (copy_to_user((void __user *)arg,
			&per_md_data->sim_setting,
			sizeof(struct ccci_sim_setting))) {
			((void)0);
			ret = -EFAULT;
		}
		break;
	case CCCI_IOC_GET_AT_CH_NUM:
		{
			unsigned int at_ch_num = 4; /*default value*/
			struct ccci_runtime_feature *rt_feature = NULL;

			rt_feature = ccci_md_get_rt_feature_by_id(md_id,
				AT_CHANNEL_NUM, 1);
			if (rt_feature)
				ret = ccci_md_parse_rt_feature(md_id,
				rt_feature, &at_ch_num, sizeof(at_ch_num));
			else
				((void)0);

			((void)0);
			ret = put_user(at_ch_num,
					(unsigned int __user *)arg);
			break;
		}

	default:
		ret = -ENOTTY;
		break;
	}
	return ret;
}

long ccci_fsm_ioctl(int md_id, unsigned int cmd, unsigned long arg)
{
	struct ccci_fsm_ctl *ctl = fsm_get_entity_by_md_id(md_id);
	int ret = 0;
	enum MD_STATE_FOR_USER state_for_user;
	unsigned int data;
	char *VALID_USER = "ccci_mdinit";

	if (!ctl)
		return -EINVAL;

	switch (cmd) {
	case CCCI_IOC_GET_MD_STATE:
		state_for_user = ccci_fsm_get_md_state_for_user(md_id);
		if (state_for_user >= 0) {
			ret = put_user((unsigned int)state_for_user,
					(unsigned int __user *)arg);
		} else {
			((void)0);
			ret = state_for_user;
		}
		break;
	case CCCI_IOC_GET_OTHER_MD_STATE:
		state_for_user =
		ccci_fsm_get_md_state_for_user(GET_OTHER_MD_ID(md_id));
		if (state_for_user >= 0) {
			ret = put_user((unsigned int)state_for_user,
					(unsigned int __user *)arg);
		} else {
			((void)0);
			ret = state_for_user;
		}
		break;
	case CCCI_IOC_MD_RESET:
		((void)0);
		ret = fsm_monitor_send_message(ctl->md_id,
			CCCI_MD_MSG_RESET_REQUEST, 0);
		fsm_monitor_send_message(GET_OTHER_MD_ID(ctl->md_id),
			CCCI_MD_MSG_RESET_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_RESET_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_FORCE_MD_ASSERT:
		((void)0);
		ret = ccci_md_force_assert(md_id,
			MD_FORCE_ASSERT_BY_USER_TRIGGER, NULL, 0);
		inject_md_status_event(md_id, MD_STA_EV_F_ASSERT_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_SEND_STOP_MD_REQUEST:
		((void)0);
		ret = fsm_monitor_send_message(ctl->md_id,
			CCCI_MD_MSG_FORCE_STOP_REQUEST, 0);
		fsm_monitor_send_message(GET_OTHER_MD_ID(ctl->md_id),
			CCCI_MD_MSG_FORCE_STOP_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_STOP_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_SEND_START_MD_REQUEST:
		((void)0);
		ret = fsm_monitor_send_message(ctl->md_id,
			CCCI_MD_MSG_FORCE_START_REQUEST, 0);
		fsm_monitor_send_message(GET_OTHER_MD_ID(ctl->md_id),
			CCCI_MD_MSG_FORCE_START_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_START_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_DO_START_MD:
		/* add check whether the user call md start ioctl is valid */
		if (strncmp(current->comm,
			VALID_USER, strlen(VALID_USER)) == 0) {
			((void)0);
			ret = fsm_append_command(ctl, CCCI_COMMAND_START, 0);
		} else {
			((void)0);
		}
		break;
	case CCCI_IOC_DO_STOP_MD:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
		} else {
			((void)0);
			ret = fsm_append_command(ctl, CCCI_COMMAND_STOP,
					(data ? MD_FLIGHT_MODE_ENTER
					: MD_FLIGHT_MODE_NONE)
					== MD_FLIGHT_MODE_ENTER ?
					FSM_CMD_FLAG_FLIGHT_MODE : 0);
		}
		break;
	case CCCI_IOC_ENTER_DEEP_FLIGHT:
		((void)0);
		ret = fsm_monitor_send_message(ctl->md_id,
				CCCI_MD_MSG_FLIGHT_STOP_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_ENTER_FLIGHT_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_LEAVE_DEEP_FLIGHT:
		((void)0);
		__pm_wakeup_event(ctl->wakelock, jiffies_to_msecs(10 * HZ));
		ret = fsm_monitor_send_message(ctl->md_id,
				CCCI_MD_MSG_FLIGHT_START_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_LEAVE_FLIGHT_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_ENTER_DEEP_FLIGHT_ENHANCED:
		((void)0);
		ret = fsm_monitor_send_message(ctl->md_id,
				CCCI_MD_MSG_FLIGHT_STOP_REQUEST, 0);
		fsm_monitor_send_message(GET_OTHER_MD_ID(ctl->md_id),
			CCCI_MD_MSG_FLIGHT_STOP_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_ENTER_FLIGHT_E_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_LEAVE_DEEP_FLIGHT_ENHANCED:
		((void)0);
		__pm_wakeup_event(ctl->wakelock, jiffies_to_msecs(10 * HZ));
		ret = fsm_monitor_send_message(ctl->md_id,
				CCCI_MD_MSG_FLIGHT_START_REQUEST, 0);
		fsm_monitor_send_message(GET_OTHER_MD_ID(ctl->md_id),
			CCCI_MD_MSG_FLIGHT_START_REQUEST, 0);
		inject_md_status_event(md_id, MD_STA_EV_LEAVE_FLIGHT_E_REQUEST,
					current->comm);
		break;
	case CCCI_IOC_SET_EFUN:
		if (copy_from_user(&data, (void __user *)arg,
				sizeof(unsigned int))) {
			((void)0);
			ret = -EFAULT;
			break;
		}
		((void)0);
		if (data == 0)
			ccci_md_soft_stop(md_id, data);
		else if (data != 0)
			ccci_md_soft_start(md_id, data);
		break;
	case CCCI_IOC_MDLOG_DUMP_DONE:
		((void)0);
		ctl->ee_ctl.mdlog_dump_done = 1;
		break;
	case CCCI_IOC_RESET_MD1_MD3_PCCIF:
		ccci_md_reset_pccif(md_id);
		break;
	case CCCI_IOC_GET_MD_EX_TYPE:
		ret = put_user((unsigned int)ctl->ee_ctl.ex_type,
				(unsigned int __user *)arg);
		((void)0);
		break;
	default:
		ret = fsm_md_data_ioctl(md_id, cmd, arg);
		break;
	}
	return ret;
}

