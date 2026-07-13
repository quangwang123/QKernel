// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015 MediaTek Inc.
 */

#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/timer.h>
#include "ccci_config.h"
#include "ccci_common_config.h"
#if defined(CONFIG_MTK_AEE_FEATURE)
#include <mt-plat/aee.h>
#endif
#ifdef ENABLE_EMI_PROTECTION
#include <mach/emi_mpu.h>
#endif
#include "mdee_dumper_v2.h"

#ifndef DB_OPT_DEFAULT
#define DB_OPT_DEFAULT    (0)	/* Dummy macro define to avoid build error */
#endif

#ifndef DB_OPT_FTRACE
#define DB_OPT_FTRACE   (0)	/* Dummy macro define to avoid build error */
#endif

static void ccci_aed_v2(struct ccci_fsm_ee *mdee, unsigned int dump_flag,
	char *aed_str, int db_opt)
{
	void *ex_log_addr = NULL;
	int ex_log_len = 0;
	void *md_img_addr = NULL;
	int md_img_len = 0;
	int info_str_len = 0;
	char *buff;		/*[AED_STR_LEN]; */
#if defined(CONFIG_MTK_AEE_FEATURE)
	char buf_fail[] = "Fail alloc mem for exception\n";
#endif
	char *img_inf;
	struct mdee_dumper_v2 *dumper = mdee->dumper_obj;
	int md_id = mdee->md_id;
	struct ccci_smem_region *mdss_dbg =
		ccci_md_get_smem_by_user_id(mdee->md_id,
			SMEM_USER_RAW_MDSS_DBG);
	struct ccci_mem_layout *mem_layout = ccci_md_get_mem(mdee->md_id);
	struct ccci_per_md *per_md_data = ccci_get_per_md_data(mdee->md_id);
	int md_dbg_dump_flag = per_md_data->md_dbg_dump_flag;
	int ret = 0;

	if (!mem_layout) {
		((void)0);
		return;
	}
	buff = kmalloc(AED_STR_LEN, GFP_ATOMIC);
	if (buff == NULL) {
		((void)0);
		goto err_exit1;
	}
	img_inf = ccci_get_md_info_str(md_id);
	if (img_inf == NULL)
		img_inf = "";
	info_str_len = strlen(aed_str);
	info_str_len += strlen(img_inf);

	if (info_str_len > AED_STR_LEN)
		/* Cut string length to AED_STR_LEN */
		buff[AED_STR_LEN - 1] = '\0';

	ret = snprintf(buff, AED_STR_LEN, "md%d:%s%s",
		md_id + 1, aed_str, img_inf);
	if (ret < 0 || ret >= AED_STR_LEN)
		((void)0);
	/* MD ID must sync with aee_dump_ccci_debug_info() */
 err_exit1:
	if (dump_flag & CCCI_AED_DUMP_CCIF_REG) {
		ex_log_addr = mdss_dbg->base_ap_view_vir;
		ex_log_len = mdss_dbg->size;
		ccci_md_dump_info(mdee->md_id,
			DUMP_FLAG_CCIF | DUMP_FLAG_CCIF_REG,
			mdss_dbg->base_ap_view_vir + CCCI_EE_OFFSET_CCIF_SRAM,
			CCCI_EE_SIZE_CCIF_SRAM);
	}
	if (dump_flag & CCCI_AED_DUMP_EX_MEM) {
		ex_log_addr = mdss_dbg->base_ap_view_vir;
		ex_log_len = mdss_dbg->size;
	}
	if (dump_flag & CCCI_AED_DUMP_EX_PKT) {
		ex_log_addr = (void *)dumper->ex_pl_info;
		ex_log_len = MD_HS1_FAIL_DUMP_SIZE;
	}
	if (dump_flag & CCCI_AED_DUMP_MD_IMG_MEM) {
		md_img_addr = (void *)mem_layout->md_bank0.base_ap_view_vir;
		md_img_len = MD_IMG_DUMP_SIZE;
	}
	if (buff == NULL) {
#if defined(CONFIG_MTK_AEE_FEATURE)
		if (md_dbg_dump_flag & (1 << MD_DBG_DUMP_SMEM))
			aed_md_exception_api(ex_log_addr, ex_log_len,
				md_img_addr, md_img_len, buf_fail, db_opt);
		else
			aed_md_exception_api(NULL, 0, md_img_addr, md_img_len,
				buf_fail, db_opt);
#endif
	} else {
#if defined(CONFIG_MTK_AEE_FEATURE)
		if (md_dbg_dump_flag & (1 << MD_DBG_DUMP_SMEM))
			aed_md_exception_api(ex_log_addr, ex_log_len,
				md_img_addr, md_img_len, buff, db_opt);
		else
			aed_md_exception_api(NULL, 0, md_img_addr, md_img_len,
				buff, db_opt);
#endif
		kfree(buff);
	}
}

static void mdee_output_debug_info_to_buf(struct ccci_fsm_ee *mdee,
	struct debug_info_t *debug_info, char *ex_info)
{
	int md_id = mdee->md_id;
	struct ccci_mem_layout *mem_layout;
	char *ex_info_temp = NULL;
	int ret = 0;
	int val = 0;

	switch (debug_info->type) {
	case MD_EX_DUMP_ASSERT:
		((void)0);
		((void)0);
		((void)0);
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY,
			"%s\n[%s] file:%s line:%d\np1:0x%08x\np2:0x%08x\np3:0x%08x\n\n",
			debug_info->core_name,
			debug_info->name,
			debug_info->assert.file_name,
			debug_info->assert.line_num,
			debug_info->assert.parameters[0],
			debug_info->assert.parameters[1],
			debug_info->assert.parameters[2]);
		break;
	case MD_EX_DUMP_3P_EX:
	case MD_EX_CC_C2K_EXCEPTION:
		((void)0);
		((void)0);
		((void)0);
		((void)0);
		if (debug_info->fatal_error.offender[0] != '\0') {
			ret = snprintf(ex_info, EE_BUF_LEN_UMOLY,
				"%s\n[%s] err_code1:0x%08X err_code2:0x%08X erro_code3:0x%08X\nMD Offender:%s\n%s",
				debug_info->core_name, debug_info->name,
				debug_info->fatal_error.err_code1,
				debug_info->fatal_error.err_code2,
				debug_info->fatal_error.err_code3,
				debug_info->fatal_error.offender,
				debug_info->fatal_error.ExStr);
		} else {
			ret = snprintf(ex_info, EE_BUF_LEN_UMOLY,
				"%s\n[%s] err_code1:0x%08X err_code2:0x%08X err_code3:0x%08X\n%s\n",
				debug_info->core_name, debug_info->name,
				debug_info->fatal_error.err_code1,
				debug_info->fatal_error.err_code2,
				debug_info->fatal_error.err_code3,
				debug_info->fatal_error.ExStr);
		}
		if (debug_info->fatal_error.err_code1 == 0x3104) {
			ex_info_temp = kmalloc(EE_BUF_LEN_UMOLY, GFP_ATOMIC);
			if (ex_info_temp == NULL) {
				((void)0);
				break;
			}
			val = snprintf(ex_info_temp, EE_BUF_LEN_UMOLY, "%s", ex_info);
			if (val < 0 || val >= EE_BUF_LEN_UMOLY) {
				((void)0);
				kfree(ex_info_temp);
				return;
			}
			mem_layout = ccci_md_get_mem(mdee->md_id);
			if (mem_layout == NULL) {
				((void)0);
				kfree(ex_info_temp);
				return;
			}
			val = snprintf(ex_info, EE_BUF_LEN_UMOLY,
			"%s%s, MD base = 0x%08X\n\n", ex_info_temp,
			mdee->ex_mpu_string,
			(unsigned int)mem_layout->md_bank0.base_ap_view_phy);
			if (val < 0 || val >= EE_BUF_LEN_UMOLY) {
				((void)0);
				kfree(ex_info_temp);
				return;
			}
			memset(mdee->ex_mpu_string, 0x0,
				sizeof(mdee->ex_mpu_string));
			kfree(ex_info_temp);
		}
		break;
	case MD_EX_DUMP_2P_EX:
		((void)0);
		((void)0);

		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY,
			"%s\n[%s] err_code1:0x%08X err_code2:0x%08X\n\n",
			debug_info->core_name, debug_info->name,
			debug_info->fatal_error.err_code1,
			debug_info->fatal_error.err_code2);
		break;
	case MD_EX_DUMP_EMI_CHECK:
		((void)0);
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY,
		"%s\n[emi_chk] 0x%08X, 0x%08X, %02d, 0x%08X\n\n",
		debug_info->core_name, debug_info->data.data0,
		debug_info->data.data1,
		debug_info->data.channel, debug_info->data.reserved);
		break;
	case MD_EX_DUMP_UNKNOWN:
	default:	/* Only display exception name */
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s\n[%s]\n",
			debug_info->core_name, debug_info->name);
		break;
	}
	if (ret < 0 || ret >= EE_BUF_LEN_UMOLY)
		((void)0);
}

static void mdee_info_dump_v2(struct ccci_fsm_ee *mdee)
{
	char *ex_info;		/*[EE_BUF_LEN] = ""; */
	/*[EE_BUF_LEN] = "\n[Others] May I-Bit dis too long\n";*/
	char *i_bit_ex_info = NULL;
	char buf_fail[] = "Fail alloc mem for exception\n";
	int db_opt = (DB_OPT_DEFAULT | DB_OPT_FTRACE);
	int dump_flag = 0;
	int core_id;
	char *ex_info_temp = NULL;/*[EE_BUF_LEN] = "";*/
	char *ex_info_buf = NULL;
	struct debug_info_t *debug_info = NULL;
	unsigned char c;
	struct mdee_dumper_v2 *dumper = mdee->dumper_obj;
	int md_id = mdee->md_id;
	struct ex_PL_log *ex_pl_info = (struct ex_PL_log *)dumper->ex_pl_info;
	int md_state = ccci_fsm_get_md_state(mdee->md_id);
	struct ccci_smem_region *mdccci_dbg =
		ccci_md_get_smem_by_user_id(mdee->md_id,
			SMEM_USER_RAW_MDCCCI_DBG);
	struct ccci_smem_region *mdss_dbg =
		ccci_md_get_smem_by_user_id(mdee->md_id,
			SMEM_USER_RAW_MDSS_DBG);
	struct rtc_time tm;
	struct timeval tv = { 0 };
	struct timeval tv_android = { 0 };
	struct rtc_time tm_android;
	struct ccci_per_md *per_md_data =
		ccci_get_per_md_data(mdee->md_id);
	int md_dbg_dump_flag = per_md_data->md_dbg_dump_flag;
	int ret = 0;
	int val = 0;

	ex_info = kmalloc(EE_BUF_LEN_UMOLY, GFP_ATOMIC);
	if (ex_info == NULL) {
		((void)0);
		goto err_exit;
	}
	ex_info_temp = kmalloc(EE_BUF_LEN_UMOLY, GFP_ATOMIC);
	if (ex_info_temp == NULL) {
		((void)0);
		goto err_exit;
	}
	ex_info_buf = kzalloc(EE_BUF_LEN_UMOLY, GFP_ATOMIC);
	if (ex_info_buf == NULL) {
		((void)0);
		goto err_exit;
	}

	do_gettimeofday(&tv);
	tv_android = tv;
	rtc_time_to_tm(tv.tv_sec, &tm);
	tv_android.tv_sec -= sys_tz.tz_minuteswest * 60;
	rtc_time_to_tm(tv_android.tv_sec, &tm_android);
	((void)0);
	for (core_id = 0; core_id < dumper->ex_core_num; core_id++) {
		if (core_id == 1)
			ret = snprintf(ex_info_temp, EE_BUF_LEN_UMOLY, "%s", ex_info);
		else if (core_id > 1) {
			ret = snprintf(ex_info_buf, EE_BUF_LEN_UMOLY, "%smd%d:%s",
				ex_info_temp, md_id + 1, ex_info);
			val = snprintf(ex_info_temp, EE_BUF_LEN_UMOLY,
				"%s", ex_info_buf);
		}
		if (val < 0 || val >= EE_BUF_LEN_UMOLY ||
			ret < 0 || ret >= EE_BUF_LEN_UMOLY)
			((void)0);
		debug_info = &dumper->debug_info[core_id];
		((void)0);
		mdee_output_debug_info_to_buf(mdee, debug_info, ex_info);
		pr_err("ccci%d: %s\n", md_id + 1, ex_info);
	}
	if (dumper->ex_core_num > 1) {
		((void)0);
		ret = snprintf(ex_info_buf, EE_BUF_LEN_UMOLY,
			"%smd%d:%s", ex_info_temp, md_id + 1, ex_info);
		val = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s", ex_info_buf);
		if (val < 0 || val >= EE_BUF_LEN_UMOLY)
			((void)0);

		debug_info = &dumper->debug_info[0];
	} else if (dumper->ex_core_num == 0)
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "\n");
	if (ret < 0 || ret >= EE_BUF_LEN_UMOLY)
		((void)0);
	/* Add additional info */
	ret = snprintf(ex_info_temp, EE_BUF_LEN_UMOLY, "%s", ex_info);
	if (ret < 0 || ret >= EE_BUF_LEN_UMOLY)
		((void)0);
	switch (dumper->more_info) {
	case MD_EE_CASE_ONLY_SWINT:
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s%s",
			ex_info_temp, "\nOnly SWINT case\n");
		break;
	case MD_EE_CASE_ONLY_EX:
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s%s",
			ex_info_temp, "\nOnly EX case\n");
		break;
	case MD_EE_CASE_NO_RESPONSE:
		/* use strcpy, otherwise if this happens after a MD EE,
		 * the former EE info will be printed out
		 */
		strncpy(ex_info, "\n[Others] MD long time no response\n",
			EE_BUF_LEN_UMOLY);
		db_opt |= DB_OPT_FTRACE;
		break;
	case MD_EE_CASE_WDT:
		strncpy(ex_info, "\n[Others] MD watchdog timeout interrupt\n",
			EE_BUF_LEN_UMOLY);
		break;
	default:
		break;
	}
	if (ret < 0 || ret >= EE_BUF_LEN_UMOLY)
		((void)0);

	/* get ELM_status field from MD side */
	ret = snprintf(ex_info_temp, EE_BUF_LEN_UMOLY, "%s", ex_info);
	if (ret < 0 || ret >= EE_BUF_LEN_UMOLY)
		((void)0);
	c = ex_pl_info->envinfo.ELM_status;
	((void)0);
	switch (c) {
	case 0xFF:
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s%s", ex_info_temp,
			"\nno ELM info\n");
		break;
	case 0xAE:
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s%s", ex_info_temp,
			"\nELM rlat:FAIL\n");
		break;
	case 0xBE:
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s%s", ex_info_temp,
			"\nELM wlat:FAIL\n");
		break;
	case 0xDE:
		ret = snprintf(ex_info, EE_BUF_LEN_UMOLY, "%s%s", ex_info_temp,
			"\nELM r/wlat:PASS\n");
		break;
	default:
		break;
	}
	if (ret < 0 || ret >= EE_BUF_LEN_UMOLY)
		((void)0);

	/* Dump MD EE info */
	((void)0);
	if (dumper->more_info == MD_EE_CASE_NORMAL
		&& md_state == BOOT_WAITING_FOR_HS1) {
		print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
				     dumper->ex_pl_info, MD_HS1_FAIL_DUMP_SIZE,
				     false);
		/* MD will not fill in share memory
		 * before we send runtime data
		 */
		dump_flag = CCCI_AED_DUMP_EX_PKT;
	} else if (md_dbg_dump_flag & (1 << MD_DBG_DUMP_SMEM)) {
		((void)0);
		print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
				     mdccci_dbg->base_ap_view_vir,
				     mdccci_dbg->size, false);
		((void)0);
		print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
				     mdss_dbg->base_ap_view_vir, 512, false);
		print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
				     (mdss_dbg->base_ap_view_vir + 6 * 1024),
				     2048, false);
		/*
		 * otherwise always dump whole share memory,
		 * as MD will fill debug log into
		 * its 2nd 1K region after bootup
		 */
		dump_flag = CCCI_AED_DUMP_EX_MEM;
		if (dumper->more_info == MD_EE_CASE_NO_RESPONSE)
			dump_flag |= CCCI_AED_DUMP_CCIF_REG;
	}

err_exit:

	/* update here to maintain handshake stage
	 * info during exception handling
	 */
	if (debug_info && debug_info->type == MD_EX_TYPE_C2K_ERROR)
		((void)0);
	else if (debug_info && (debug_info->type == MD_EX_DUMP_EMI_CHECK)
			&& (Is_MD_EMI_voilation() == 0))
		((void)0);
	else if (ex_info == NULL)
		ccci_aed_v2(mdee, dump_flag, buf_fail, db_opt);
	else
		ccci_aed_v2(mdee, dump_flag, ex_info, db_opt);


	kfree(ex_info);
	kfree(ex_info_temp);
	kfree(ex_info_buf);
	kfree(i_bit_ex_info);
}

static char mdee_plstr[MD_EX_PL_FATALE_TOTAL + MD_EX_OTHER_CORE_EXCEPTIN -
	MD_EX_CC_INVALID_EXCEPTION][32] = {
	"INVALID",
	"Fatal error (undefine)",
	"Fatal error (swi)",
	"Fatal error (prefetch abort)",
	"Fatal error (data abort)",
	"Fatal error (stack)",
	"Fatal error (task)",
	"Fatal error (buff)",
	"Fatal error (CC invalid)",
	"Fatal error (CC PCore)",
	"Fatal error (CC L1Core)",
	"Fatal error (CC CS)",
	"Fatal error (CC MD32)",
	"Fatal error (CC C2K)",
	"Fatal error (CC spc)"
};

static void strmncopy(char *src, char *dst, int src_len, int dst_len)
{
	int temp_m, temp_n, temp_i;

	temp_m = src_len - 1;
	temp_n = dst_len - 1;
	temp_n = (temp_m > temp_n) ? temp_n : temp_m;
	for (temp_i = 0; temp_i < temp_n; temp_i++) {
		dst[temp_i] = src[temp_i];
		if (dst[temp_i] == 0x00)
			break;
	}
	((void)0);
}

static int mdee_pl_core_parse(int md_id, struct debug_info_t *debug_info,
	struct ex_PL_log *ex_PLloginfo)
{
	int ee_type = 0;
	int ee_case = 0;

	ee_type = ex_PLloginfo->header.ex_type;
	debug_info->type = ee_type;
	ee_case = ee_type;
	((void)0);
	switch (ee_type) {
	case MD_EX_PL_INVALID:
		debug_info->name = "INVALID";
		break;
	case MD_EX_CC_INVALID_EXCEPTION:
		/* Fall through */
	case MD_EX_CC_PCORE_EXCEPTION:
		/* Fall through */
	case MD_EX_CC_L1CORE_EXCEPTION:
		/* Fall through */
	case MD_EX_CC_CS_EXCEPTION:
		/* Fall through */
	case MD_EX_CC_MD32_EXCEPTION:
		/* Fall through */
	case MD_EX_CC_C2K_EXCEPTION:
		/* Fall through */
	case MD_EX_CC_ARM7_EXCEPTION:
		/*
		 * md1:(MCU_PCORE)
		 * [Fatal error (CC xxx)]
		 * err_code1:0x00000xx err_code2:0x00xxx err_code3:0xxxx
		 * Offender:
		 */
		ee_type = ee_type - MD_EX_CC_INVALID_EXCEPTION
					+ MD_EX_PL_FATALE_TOTAL;
		/* Fall through */
	case MD_EX_PL_UNDEF:
		/* Fall through */
	case MD_EX_PL_SWI:
		/* Fall through */
	case MD_EX_PL_PREF_ABT:
		/* Fall through */
	case MD_EX_PL_DATA_ABT:
		/* Fall through */
	case MD_EX_PL_STACKACCESS:
		/* Fall through */
	case MD_EX_PL_FATALERR_TASK:
		/* Fall through */
	case MD_EX_PL_FATALERR_BUF:
		/* all offender is zero,
		 * goto from tail of function, reparser.
		 */
		/* the only one case: none offender, c2k ee */
		if (ee_type ==
				(MD_EX_CC_C2K_EXCEPTION
				- MD_EX_CC_INVALID_EXCEPTION +
				MD_EX_PL_FATALE_TOTAL))
			debug_info->type = MD_EX_CC_C2K_EXCEPTION;
		else
			debug_info->type = MD_EX_DUMP_3P_EX;
		debug_info->name = mdee_plstr[ee_type];
		if (ex_PLloginfo->content.fatalerr.ex_analy.owner[0] != 0xCC) {
			strmncopy(ex_PLloginfo->content.fatalerr.ex_analy.owner,
			debug_info->fatal_error.offender,
			sizeof(ex_PLloginfo->content.fatalerr.ex_analy.owner),
			sizeof(debug_info->fatal_error.offender));
			((void)0);
		}
		debug_info->fatal_error.err_code1 =
		    ex_PLloginfo->content.fatalerr.error_code.code1;
		debug_info->fatal_error.err_code2 =
		    ex_PLloginfo->content.fatalerr.error_code.code2;
		debug_info->fatal_error.err_code3 =
		    ex_PLloginfo->content.fatalerr.error_code.code3;
		if (ex_PLloginfo->content.fatalerr.ex_analy.is_cadefa_sup
			== 0x01)
			debug_info->fatal_error.ExStr = "CaDeFa Supported\n";
		else
			debug_info->fatal_error.ExStr = "";
		break;
	case MD_EX_PL_ASSERT_FAIL:
		/* Fall through */
	case MD_EX_PL_ASSERT_DUMP:
		/* Fall through */
	case MD_EX_PL_ASSERT_NATIVE:
		debug_info->type = MD_EX_DUMP_ASSERT;/* = MD_EX_TYPE_ASSERT; */
		debug_info->name = "ASSERT";
		((void)0);
		strmncopy(ex_PLloginfo->content.assert.filepath,
			debug_info->assert.file_name,
			sizeof(ex_PLloginfo->content.assert.filepath),
			sizeof(debug_info->assert.file_name));
		((void)0);
		debug_info->assert.line_num =
			ex_PLloginfo->content.assert.linenumber;
		debug_info->assert.parameters[0] =
			ex_PLloginfo->content.assert.para[0];
		debug_info->assert.parameters[1] =
			ex_PLloginfo->content.assert.para[1];
		debug_info->assert.parameters[2] =
			ex_PLloginfo->content.assert.para[2];
		break;

	case EMI_MPU_VIOLATION:
		debug_info->type = MD_EX_DUMP_EMI_CHECK;
		ee_case = MD_EX_TYPE_EMI_CHECK;
		debug_info->name = "Fatal error (rmpu violation)";
		debug_info->fatal_error.err_code1 =
		    ex_PLloginfo->content.fatalerr.error_code.code1;
		debug_info->fatal_error.err_code2 =
		    ex_PLloginfo->content.fatalerr.error_code.code2;
		debug_info->fatal_error.err_code3 =
		    ex_PLloginfo->content.fatalerr.error_code.code3;
		debug_info->fatal_error.ExStr = "EMI MPU VIOLATION\n";
		break;
	default:
		debug_info->name = "UNKNOWN Exception";
		break;
	}
	debug_info->ext_mem = ex_PLloginfo;
	debug_info->ext_size = sizeof(struct ex_PL_log);

	return ee_case;
}


static int mdee_cs_core_parse(int md_id, struct debug_info_t *debug_info,
	struct ex_cs_log *ex_csLogInfo)
{
	int ee_type = 0;
	int ee_case = 0;

	ee_type = ex_csLogInfo->except_type;
	((void)0);
	switch (ee_type) {
	case CS_EXCEPTION_ASSERTION:
		debug_info->type = MD_EX_DUMP_ASSERT;
		ee_case = MD_EX_TYPE_ASSERT;

		debug_info->name = "ASSERT";
		strmncopy(ex_csLogInfo->except_content.assert.file_name,
			debug_info->assert.file_name,
			sizeof(ex_csLogInfo->except_content.assert.file_name),
			sizeof(debug_info->assert.file_name));
		debug_info->assert.line_num =
			ex_csLogInfo->except_content.assert.line_num;
		debug_info->assert.parameters[0] =
			ex_csLogInfo->except_content.assert.para1;
		debug_info->assert.parameters[1] =
			ex_csLogInfo->except_content.assert.para2;
		debug_info->assert.parameters[2] =
			ex_csLogInfo->except_content.assert.para3;
		break;
	case CS_EXCEPTION_FATAL_ERROR:
		debug_info->type = MD_EX_DUMP_2P_EX;
		ee_case = MD_EX_TYPE_FATALERR_TASK;

		debug_info->name = "Fatal error";
		debug_info->fatal_error.err_code1 =
		    ex_csLogInfo->except_content.fatalerr.error_code1;
		debug_info->fatal_error.err_code2 =
		    ex_csLogInfo->except_content.fatalerr.error_code2;
		break;
	case CS_EXCEPTION_CTI_EVENT:
		debug_info->name = "CC CTI Exception";
		break;
	case CS_EXCEPTION_UNKNOWN:
	default:
		debug_info->name = "UNKNOWN Exception";
		break;
	}
	debug_info->ext_mem = ex_csLogInfo;
	debug_info->ext_size = sizeof(struct ex_cs_log);
	return ee_case;
}

static int mdee_md32_core_parse(int md_id, struct debug_info_t *debug_info,
	struct ex_md32_log *ex_md32LogInfo)
{
	int ee_type = 0;
	int ee_case = 0;
	char core_name_temp[MD_CORE_NAME_DEBUG];
	int ret = 0;

	ee_type = ex_md32LogInfo->except_type;
	((void)0);
	ret = snprintf(core_name_temp,
		MD_CORE_NAME_DEBUG, "%s", debug_info->core_name);
	if (ret < 0 || ret >= MD_CORE_NAME_DEBUG)
		((void)0);
	switch (ex_md32LogInfo->md32_active_mode) {
	case 1:
		ret = snprintf(debug_info->core_name, MD_CORE_NAME_DEBUG, "%s%s",
			core_name_temp, MD32_FDD_ROCODE);
		break;
	case 2:
		ret = snprintf(debug_info->core_name, MD_CORE_NAME_DEBUG, "%s%s",
			core_name_temp, MD32_TDD_ROCODE);
		break;
	default:
		break;
	}
	if (ret < 0 || ret >= MD_CORE_NAME_DEBUG)
		((void)0);
	switch (ee_type) {
	case CMIF_MD32_EX_ASSERT_LINE:
		/* Fall through */
	case CMIF_MD32_EX_ASSERT_EXT:
		debug_info->type = MD_EX_DUMP_ASSERT;
		ee_case = MD_EX_TYPE_ASSERT;
		debug_info->name = "ASSERT";
		strmncopy(ex_md32LogInfo->except_content.assert.file_name,
			debug_info->assert.file_name,
			sizeof(ex_md32LogInfo->except_content.assert.file_name),
			sizeof(debug_info->assert.file_name));
		debug_info->assert.line_num =
			ex_md32LogInfo->except_content.assert.line_num;
		debug_info->assert.parameters[0] =
		    ex_md32LogInfo->except_content.assert.ex_code[0];
		debug_info->assert.parameters[1] =
		    ex_md32LogInfo->except_content.assert.ex_code[1];
		debug_info->assert.parameters[2] =
		    ex_md32LogInfo->except_content.assert.ex_code[2];
		break;
	case CMIF_MD32_EX_FATAL_ERROR:
		/* Fall through */
	case CMIF_MD32_EX_FATAL_ERROR_EXT:
		debug_info->type = MD_EX_DUMP_2P_EX;
		ee_case = MD_EX_TYPE_FATALERR_TASK;

		debug_info->name = "Fatal error";
		debug_info->fatal_error.err_code1 =
		    ex_md32LogInfo->except_content.fatalerr.ex_code[0];
		debug_info->fatal_error.err_code2 =
		    ex_md32LogInfo->except_content.fatalerr.ex_code[1];
		break;
	case CS_EXCEPTION_CTI_EVENT:
		/* Fall through */
	case CS_EXCEPTION_UNKNOWN:
	default:
		debug_info->name = "UNKNOWN Exception";
		break;
	}
	debug_info->ext_mem = ex_md32LogInfo;
	debug_info->ext_size = sizeof(struct ex_md32_log);

	return ee_case;
}

static void mdee_set_core_name(int md_id, struct debug_info_t *debug_info,
	char *core_name)
{
	unsigned int temp_i;
	/* (core name):
	 * PCORE/L1CORE/CS_ICC/CS_IMC/CS_MPC/MD32_BRP/MD32_DFE/MD32_RAKE
	 */
	debug_info->core_name[0] = '(';
	for (temp_i = 1; temp_i < MD_CORE_NAME_LEN; temp_i++) {
		debug_info->core_name[temp_i] = core_name[temp_i - 1];
		if (debug_info->core_name[temp_i] == '\0')
			break;
	}
	debug_info->core_name[temp_i++] = ')';
	debug_info->core_name[temp_i] = '\0';
}

static void mdee_info_prepare_v2(struct ccci_fsm_ee *mdee)
{
	struct ex_overview_t *ex_overview;
	int ee_case = 0;
	struct mdee_dumper_v2 *dumper = mdee->dumper_obj;
	int md_id = mdee->md_id;
	struct debug_info_t *debug_info = NULL;
	int core_id;
	/* number of offender core: need parse */
	unsigned char off_core_num = 0;
	int ret = 0;

	struct ex_PL_log *ex_pl_info = (struct ex_PL_log *)dumper->ex_pl_info;
	struct ex_PL_log *ex_PLloginfo;
	struct ex_cs_log *ex_csLogInfo;
	struct ex_md32_log *ex_md32LogInfo;
	struct ccci_smem_region *mdss_dbg =
	ccci_md_get_smem_by_user_id(mdee->md_id, SMEM_USER_RAW_MDSS_DBG);

	((void)0);

	if ((dumper->more_info == MD_EE_CASE_NORMAL)
		&& (ccci_fsm_get_md_state(mdee->md_id)
		== BOOT_WAITING_FOR_HS1)) {
		debug_info = &dumper->debug_info[0];
		ex_PLloginfo = ex_pl_info;
		off_core_num++;
		ret = snprintf(debug_info->core_name,
			MD_CORE_NAME_DEBUG, "(MCU_PCORE)");
		if (ret < 0 || ret >= MD_CORE_NAME_DEBUG)
			((void)0);
		ee_case = mdee_pl_core_parse(md_id,
		debug_info, ex_PLloginfo);
		mdee->ex_type = ee_case;
		dumper->ex_core_num = off_core_num;
		((void)0);
		return;
	}

	ex_overview = (struct ex_overview_t *) mdss_dbg->base_ap_view_vir;
	for (core_id = 0; core_id < MD_CORE_NUM; core_id++) {
		((void)0);
		if (ex_overview->main_reson[core_id].is_offender == 0)
			continue;
		debug_info = &dumper->debug_info[off_core_num];
		memset(debug_info, 0, sizeof(struct debug_info_t));

		off_core_num++;
		mdee_set_core_name(md_id, debug_info,
			ex_overview->main_reson[core_id].core_name);
		((void)0);
		ex_pl_info->envinfo.ELM_status = 0;
		switch (core_id) {
		case MD_PCORE:
		case MD_L1CORE:
			ex_PLloginfo =
			    (struct ex_PL_log *) ((char *)ex_overview +
				ex_overview->main_reson[core_id].core_offset);
			ex_pl_info->envinfo.ELM_status =
				ex_PLloginfo->envinfo.ELM_status;
			ee_case = mdee_pl_core_parse(md_id,
				debug_info, ex_PLloginfo);
			break;
		case MD_CS_ICC:
			/* Fall through */
		case MD_CS_IMC:
			/* Fall through */
		case MD_CS_MPC:
			ex_csLogInfo =
			    (struct ex_cs_log *) ((char *)ex_overview +
				ex_overview->main_reson[core_id].core_offset);
			ee_case = mdee_cs_core_parse(md_id,
				debug_info, ex_csLogInfo);
			break;
		case MD_MD32_DFE:
			/* Fall through */
		case MD_MD32_BRP:
			/* Fall through */
		case MD_MD32_RAKE:
			ex_md32LogInfo =
				(struct ex_md32_log *) ((char *)ex_overview +
				ex_overview->main_reson[core_id].core_offset);
			ee_case = mdee_md32_core_parse(md_id,
				debug_info, ex_md32LogInfo);
			break;
		default:
			ee_case = 0;
			break;
		}
		if (off_core_num == 1) {
			mdee->ex_type = ee_case;
			((void)0);
		}
	}

	if (off_core_num == 0) {
		debug_info = &dumper->debug_info[0];
		ex_PLloginfo = (struct ex_PL_log *) ((char *)ex_overview +
				ex_overview->main_reson[MD_PCORE].core_offset);
		ex_pl_info->envinfo.ELM_status =
				ex_PLloginfo->envinfo.ELM_status;
		off_core_num++;
		core_id = MD_CORE_NUM;
		ret = snprintf(debug_info->core_name,
			MD_CORE_NAME_DEBUG, "(MCU_PCORE)");
		if (ret < 0 || ret >= MD_CORE_NAME_DEBUG)
			((void)0);
		ee_case = mdee_pl_core_parse(md_id,
			debug_info, ex_PLloginfo);
		mdee->ex_type = ee_case;
	}

	dumper->ex_core_num = off_core_num;
	((void)0);
}

static void mdee_dumper_v2_set_ee_pkg(struct ccci_fsm_ee *mdee,
	char *data, int len)
{
	struct mdee_dumper_v2 *dumper = mdee->dumper_obj;
	int cpy_len =
	len > MD_HS1_FAIL_DUMP_SIZE ? MD_HS1_FAIL_DUMP_SIZE : len;

	memcpy(dumper->ex_pl_info, data, cpy_len);
}

static void mdee_dumper_v2_dump_ee_info(struct ccci_fsm_ee *mdee,
	enum MDEE_DUMP_LEVEL level, int more_info)
{
	struct mdee_dumper_v2 *dumper = mdee->dumper_obj;
	int md_id = mdee->md_id;
	struct ccci_smem_region *mdccci_dbg =
		ccci_md_get_smem_by_user_id(mdee->md_id,
			SMEM_USER_RAW_MDCCCI_DBG);
	struct ccci_smem_region *mdss_dbg =
		ccci_md_get_smem_by_user_id(mdee->md_id,
			SMEM_USER_RAW_MDSS_DBG);
	int md_state = ccci_fsm_get_md_state(mdee->md_id);
	char ex_info[EE_BUF_LEN] = {0};
	struct ccci_per_md *per_md_data =
		ccci_get_per_md_data(mdee->md_id);
	int md_dbg_dump_flag = per_md_data->md_dbg_dump_flag;
	int ret = 0;

	dumper->more_info = more_info;
	if (level == MDEE_DUMP_LEVEL_BOOT_FAIL) {
		if (md_state == BOOT_WAITING_FOR_HS1) {
			ret = snprintf(ex_info, EE_BUF_LEN,
				"\n[Others] MD_BOOT_UP_FAIL(HS%d)\n", 1);
			/* Handshake 1 fail */
			ccci_aed_v2(mdee,
			CCCI_AED_DUMP_CCIF_REG
			| CCCI_AED_DUMP_MD_IMG_MEM
			| CCCI_AED_DUMP_EX_MEM,
			ex_info, DB_OPT_DEFAULT);
		} else if (md_state == BOOT_WAITING_FOR_HS2) {
			ret = snprintf(ex_info, EE_BUF_LEN,
				"\n[Others] MD_BOOT_UP_FAIL(HS%d)\n", 2);
			/* Handshake 2 fail */
			((void)0);
			if (md_dbg_dump_flag & (1 << MD_DBG_DUMP_SMEM)) {
				print_hex_dump_debug(
					"ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
					mdccci_dbg->base_ap_view_vir,
					mdccci_dbg->size, false);
				print_hex_dump_debug(
					"ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
					mdss_dbg->base_ap_view_vir,
					mdss_dbg->size, false);
			}

			ccci_aed_v2(mdee,
			CCCI_AED_DUMP_CCIF_REG
			| CCCI_AED_DUMP_EX_MEM,
			ex_info, DB_OPT_FTRACE);
		}
		if (ret < 0 || ret >= EE_BUF_LEN)
			((void)0);
	} else if (level == MDEE_DUMP_LEVEL_STAGE1) {
		if (md_dbg_dump_flag & (1 << MD_DBG_DUMP_SMEM)) {
			((void)0);
			print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16,
					     4, mdccci_dbg->base_ap_view_vir,
					     mdccci_dbg->size, false);
			((void)0);
			print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16,
					     4, mdss_dbg->base_ap_view_vir, 512,
					     false);
			print_hex_dump_debug(
				"ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
				(mdss_dbg->base_ap_view_vir + 6 * 1024), 2048,
				false);
		}
	} else if (level == MDEE_DUMP_LEVEL_STAGE2) {
		mdee_info_prepare_v2(mdee);
		mdee_info_dump_v2(mdee);
	}
}

static struct md_ee_ops mdee_ops_v2 = {
	.dump_ee_info = &mdee_dumper_v2_dump_ee_info,
	.set_ee_pkg = &mdee_dumper_v2_set_ee_pkg,
};
int mdee_dumper_v2_alloc(struct ccci_fsm_ee *mdee)
{
	struct mdee_dumper_v2 *dumper;
	int md_id = mdee->md_id;

	/* Allocate port_proxy obj and set all member zero */
	dumper = kzalloc(sizeof(struct mdee_dumper_v2), GFP_KERNEL);
	if (dumper == NULL) {
		((void)0);
		return -1;
	}
	mdee->dumper_obj = dumper;
	mdee->ops = &mdee_ops_v2;
	return 0;
}
