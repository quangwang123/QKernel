// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015 MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include "ccci_config.h"
#include "ccci_common_config.h"
#include <linux/clk.h>
#include <mtk_pbm.h>
#include <mt-plat/mtk-clkbuf-bridge.h>
#ifdef USING_PM_RUNTIME
#include <linux/pm_runtime.h>
#else
#include <dt-bindings/clock/mt6779-clk.h>
#endif
#ifdef CONFIG_MTK_EMI_BWL
#include <emi_mbw.h>
#endif

#ifdef FEATURE_INFORM_NFC_VSIM_CHANGE
#include <mach/mt6605.h>
#endif

#ifdef CONFIG_MTK_QOS_SUPPORT
#include <linux/pm_qos.h>
#include <helio-dvfsrc-opp.h>
#endif

#include <linux/regulator/consumer.h> /* for MD PMIC */
#include <clk-mt6779-pg.h>
#include <mtk_spm_sleep.h>

#include "ccci_core.h"
#include "ccci_platform.h"

#include "md_sys1_platform.h"
#include "modem_secure_base.h"
#include "modem_reg_base.h"
#include "ap_md_reg_dump.h"

static struct regulator *reg_vmodem, *reg_vsram;

static struct ccci_clk_node clk_table[] = {
/* #ifdef USING_PM_RUNTIME */
	{ NULL, "scp-sys-md1-main"},
/* #endif */
	{ NULL, "infra-dpmaif-clk"},
	{ NULL, "infra-ccif-ap"},
	{ NULL, "infra-ccif-md"},
	{ NULL, "infra-ccif1-ap"},
	{ NULL, "infra-ccif1-md"},
	{ NULL, "infra-ccif2-ap"},
	{ NULL, "infra-ccif2-md"},
	{ NULL, "infra-ccif4-md"},
};
#define TAG "mcd"

#define ROr2W(a, b, c)  ccci_write32(a, b, (ccci_read32(a, b)|c))
#define RAnd2W(a, b, c)  ccci_write32(a, b, (ccci_read32(a, b)&c))
#define RabIsc(a, b, c) ((ccci_read32(a, b)&c) != c)

static int md_cd_io_remap_md_side_register(struct ccci_modem *md);
static void md_cd_dump_debug_register(struct ccci_modem *md);
static void md_cd_dump_md_bootup_status(struct ccci_modem *md);
static void md_cd_get_md_bootup_status(struct ccci_modem *md,
	unsigned int *buff, int length);
static void md_cd_check_emi_state(struct ccci_modem *md, int polling);
static int md_start_platform(struct ccci_modem *md);
static int md_cd_power_on(struct ccci_modem *md);
static int md_cd_power_off(struct ccci_modem *md, unsigned int timeout);
static int md_cd_soft_power_off(struct ccci_modem *md, unsigned int mode);
static int md_cd_soft_power_on(struct ccci_modem *md, unsigned int mode);
static int md_cd_let_md_go(struct ccci_modem *md);
static void md_cd_lock_cldma_clock_src(int locked);
static void md_cd_lock_modem_clock_src(int locked);
static void md_cldma_hw_reset(unsigned char md_id);
static void ccci_set_clk_cg(struct ccci_modem *md, unsigned int on);

static int ccci_modem_remove(struct platform_device *dev);
static void ccci_modem_shutdown(struct platform_device *dev);
static int ccci_modem_suspend(struct platform_device *dev, pm_message_t state);
static int ccci_modem_resume(struct platform_device *dev);
static int ccci_modem_pm_suspend(struct device *device);
static int ccci_modem_pm_resume(struct device *device);
static int ccci_modem_pm_restore_noirq(struct device *device);


static struct ccci_plat_ops md_cd_plat_ptr = {
	.init = &ccci_platform_init_6779,
	.md_dump_reg = &md_dump_register_6779,
	.cldma_hw_rst = &md_cldma_hw_reset,
	.set_clk_cg = &ccci_set_clk_cg,
	.remap_md_reg = &md_cd_io_remap_md_side_register,
	.lock_cldma_clock_src = &md_cd_lock_cldma_clock_src,
	.lock_modem_clock_src = &md_cd_lock_modem_clock_src,
	.dump_md_bootup_status = &md_cd_dump_md_bootup_status,
	.get_md_bootup_status = &md_cd_get_md_bootup_status,
	.debug_reg = &md_cd_dump_debug_register,
	.check_emi_state = &md_cd_check_emi_state,
	.soft_power_off = &md_cd_soft_power_off,
	.soft_power_on = &md_cd_soft_power_on,
	.start_platform = &md_start_platform,
	.power_on = &md_cd_power_on,
	.let_md_go = &md_cd_let_md_go,
	.power_off = &md_cd_power_off,
	.vcore_config = NULL,
};


static void md_cldma_hw_reset(unsigned char md_id)
{
}


void md1_subsys_debug_dump(enum subsys_id sys)
{
	struct ccci_modem *md;

	if (sys != SYS_MD1)
		return;
		/* add debug dump */

	((void)0);
	md = ccci_md_get_modem_by_id(0);
	if (md != NULL) {
		((void)0);
		md->ops->dump_info(md, DUMP_FLAG_CCIF_REG | DUMP_FLAG_CCIF |
			DUMP_FLAG_REG | DUMP_FLAG_QUEUE_0_1 |
			DUMP_MD_BOOTUP_STATUS, NULL, 0);
		mdelay(1000);
		md->ops->dump_info(md, DUMP_FLAG_REG, NULL, 0);
	}
	((void)0);
}


struct pg_callbacks md1_subsys_handle = {
	.debug_dump = md1_subsys_debug_dump,
};

#ifdef ENABLE_DEBUG_DUMP /* Fix me! */
void ccci_dump(void)
{
	md1_subsys_debug_dump(SYS_MD1);
}
EXPORT_SYMBOL(ccci_dump);
#endif

static int md_cd_get_modem_hw_info(struct platform_device *dev_ptr,
	struct ccci_dev_cfg *dev_cfg, struct md_hw_info *hw_info)
{
	struct device_node *node = NULL;
	struct device_node *node_infrao = NULL;
	int idx = 0;
#ifdef USING_PM_RUNTIME
	int retval = 0;
#endif

	if (dev_ptr->dev.of_node == NULL) {
		((void)0);
		return -1;
	}

	memset(dev_cfg, 0, sizeof(struct ccci_dev_cfg));
	of_property_read_u32(dev_ptr->dev.of_node,
		"mediatek,md_id", &dev_cfg->index);
	((void)0);
	if (!get_modem_is_enabled(dev_cfg->index)) {
		((void)0);
		return -1;
	}

	switch (dev_cfg->index) {
	case 0:		/* MD_SYS1 */
		dev_cfg->major = 0;
		dev_cfg->minor_base = 0;
		of_property_read_u32(dev_ptr->dev.of_node,
			"mediatek,cldma_capability", &dev_cfg->capability);

		hw_info->ap_ccif_base =
		 (unsigned long)of_iomap(dev_ptr->dev.of_node, 0);
		hw_info->md_ccif_base =
		 (unsigned long)of_iomap(dev_ptr->dev.of_node, 1);

		hw_info->md_wdt_irq_id =
		 irq_of_parse_and_map(dev_ptr->dev.of_node, 0);
		hw_info->ap_ccif_irq0_id =
		 irq_of_parse_and_map(dev_ptr->dev.of_node, 1);
		hw_info->ap_ccif_irq1_id =
		 irq_of_parse_and_map(dev_ptr->dev.of_node, 2);

		hw_info->md_pcore_pccif_base =
		 ioremap_nocache(MD_PCORE_PCCIF_BASE, 0x20);
		((void)0);

		/* Device tree using none flag to register irq,
		 * sensitivity has set at "irq_of_parse_and_map"
		 */
		hw_info->ap_ccif_irq0_flags = IRQF_TRIGGER_NONE;
		hw_info->ap_ccif_irq1_flags = IRQF_TRIGGER_NONE;
		hw_info->md_wdt_irq_flags = IRQF_TRIGGER_NONE;

		hw_info->sram_size = CCIF_SRAM_SIZE;
		hw_info->md_rgu_base = MD_RGU_BASE;
		hw_info->md_boot_slave_En = MD_BOOT_VECTOR_EN;
		of_property_read_u32(dev_ptr->dev.of_node,
			"mediatek,md_generation", &md_cd_plat_val_ptr.md_gen);
		node_infrao = of_find_compatible_node(NULL, NULL,
			"mediatek,mt6779-infracfg_ao");
		md_cd_plat_val_ptr.infra_ao_base = of_iomap(node_infrao, 0);

		hw_info->plat_ptr = &md_cd_plat_ptr;
		hw_info->plat_val = &md_cd_plat_val_ptr;
		if ((hw_info->plat_ptr == NULL) || (hw_info->plat_val == NULL))
			return -1;
		hw_info->plat_val->offset_epof_md1 = 7*1024+0x234;
		for (idx = 0; idx < ARRAY_SIZE(clk_table); idx++) {
			clk_table[idx].clk_ref = devm_clk_get(&dev_ptr->dev,
				clk_table[idx].clk_name);
			if (IS_ERR(clk_table[idx].clk_ref)) {
				((void)0);
				clk_table[idx].clk_ref = NULL;
			}
		}
		node = of_find_compatible_node(NULL, NULL,
			"mediatek,md_ccif4");
		if (node) {
			hw_info->md_ccif4_base = of_iomap(node, 0);
			if (!hw_info->md_ccif4_base) {
				((void)0);
				return -1;
			}
		}
		break;
	default:
		return -1;
	}

	if (hw_info->ap_ccif_base == 0 ||
		hw_info->md_ccif_base == 0) {
		((void)0);
		return -1;
	}
	if (hw_info->ap_ccif_irq0_id == 0 ||
		hw_info->ap_ccif_irq1_id == 0 ||
		hw_info->md_wdt_irq_id == 0) {
		((void)0);
		return -1;
	}

	((void)0);

	((void)0);
	((void)0);
	register_pg_callback(&md1_subsys_handle);
#ifdef USING_PM_RUNTIME
	pm_runtime_enable(&dev_ptr->dev);
	dev_pm_syscore_device(&dev_ptr->dev, true);

	((void)0);
	retval = pm_runtime_get_sync(&dev_ptr->dev); /* match lk on */
	if (retval)
		((void)0);

	((void)0);
#endif

	return 0;
}

/* md1 sys_clk_cg no need set in this API*/
static void ccci_set_clk_cg(struct ccci_modem *md, unsigned int on)
{
}

static int md_cd_io_remap_md_side_register(struct ccci_modem *md)
{
	struct md_pll_reg *md_reg;
	struct md_sys1_info *md_info = (struct md_sys1_info *)md->private_data;

	md_info->md_boot_slave_En =
	 ioremap_nocache(md->hw_info->md_boot_slave_En, 0x4);
	md_info->md_rgu_base =
	 ioremap_nocache(md->hw_info->md_rgu_base, 0x300);
	md_info->l1_rgu_base =
	 ioremap_nocache(md->hw_info->l1_rgu_base, 0x40);

	md_reg = kzalloc(sizeof(struct md_pll_reg), GFP_KERNEL);
	if (md_reg == NULL) {
		((void)0);
		return -1;
	}

	md_reg->md_boot_stats_select =
		ioremap_nocache(MD1_BOOT_STATS_SELECT, 4);
	md_reg->md_boot_stats = ioremap_nocache(MD1_CFG_BOOT_STATS, 4);
	/*just for dump end*/

	md_info->md_pll_base = md_reg;

#ifdef MD_PEER_WAKEUP
	md_info->md_peer_wakeup = ioremap_nocache(MD_PEER_WAKEUP, 0x4);
#endif
	return 0;
}

static void md_cd_lock_cldma_clock_src(int locked)
{
	/* spm_ap_mdsrc_req(locked); */
}

static void md_cd_lock_modem_clock_src(int locked)
{
	spm_ap_mdsrc_req(locked);
}

static void md_cd_dump_md_bootup_status(struct ccci_modem *md)
{
	struct md_sys1_info *md_info = (struct md_sys1_info *)md->private_data;
	struct md_pll_reg *md_reg = md_info->md_pll_base;

	/*To avoid AP/MD interface delay,
	 * dump 3 times, and buy-in the 3rd dump value.
	 */

	ccci_write32(md_reg->md_boot_stats_select, 0, 0);
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	((void)0);

	ccci_write32(md_reg->md_boot_stats_select, 0, 1);
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	((void)0);
}

static void md_cd_get_md_bootup_status(
	struct ccci_modem *md, unsigned int *buff, int length)
{
	struct md_sys1_info *md_info = (struct md_sys1_info *)md->private_data;
	struct md_pll_reg *md_reg = md_info->md_pll_base;

	((void)0);

	if (length < 2 || buff == NULL) {
		md_cd_dump_md_bootup_status(md);
		return;
	}

	ccci_write32(md_reg->md_boot_stats_select, 0, 0);
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	buff[0] = ccci_read32(md_reg->md_boot_stats, 0);

	ccci_write32(md_reg->md_boot_stats_select, 0, 1);
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	ccci_read32(md_reg->md_boot_stats, 0);	/* dummy read */
	buff[1] = ccci_read32(md_reg->md_boot_stats, 0);
	((void)0);

}

static int dump_emi_last_bm(struct ccci_modem *md)
{
	u32 buf_len = 1024;
	u32 i, j;
	char temp_char;
	char *buf = NULL;
	char *temp_buf = NULL;

	buf = kzalloc(buf_len, GFP_ATOMIC);
	if (!buf) {
		((void)0);
		return -1;
	}
#ifdef CONFIG_MTK_EMI_BWL
	dump_last_bm(buf, buf_len);
#endif
	((void)0);
	buf[buf_len - 1] = '\0';
	temp_buf = buf;
	for (i = 0, j = 1; i < buf_len - 1; i++, j++) {
		if (buf[i] == 0x0) /* 0x0 end of hole string. */
			break;
		if (buf[i] == 0x0A && j < 256) {
			/* 0x0A stands for end of string, no 0x0D */
			buf[i] = '\0';
			((void)0);/* max 256 bytes */
			temp_buf = buf + i + 1;
			j = 0;
		} else if (unlikely(j >= 255)) {
			/* ccci_mem_log max buffer length: 256,
			 * but dm log maybe only less than 50 bytes.
			 */
			temp_char = buf[i];
			buf[i] = '\0';
			((void)0);
			temp_buf = buf + i;
			j = 0;
			buf[i] = temp_char;
		}
	}

	kfree(buf);

	return 0;
}

void __weak dump_emi_outstanding(void)
{
	((void)0);
}

static void md_cd_dump_debug_register(struct ccci_modem *md)
{
	/* MD no need dump because of bus hang happened - open for debug */
	unsigned int reg_value[2] = { 0 };
	unsigned int ccif_sram[
		CCCI_EE_SIZE_CCIF_SRAM/sizeof(unsigned int)] = { 0 };

	/*dump_emi_latency();*/
	dump_emi_outstanding();
	dump_emi_last_bm(md);

	md_cd_get_md_bootup_status(md, reg_value, 2);
	md->ops->dump_info(md, DUMP_FLAG_CCIF, ccif_sram, 0);
	/* copy from HS1 timeout */
	if ((reg_value[0] == 0) && (ccif_sram[1] == 0))
		return;
	else if (!((reg_value[0] == 0x5443000C) || (reg_value[0] == 0) ||
		(reg_value[0] >= 0x53310000 && reg_value[0] <= 0x533100FF)))
		return;
	if (unlikely(in_interrupt())) {
		((void)0);
		return;
	}
	md_cd_lock_modem_clock_src(1);
	if (md->hw_info->plat_ptr->md_dump_reg)
		md->hw_info->plat_ptr->md_dump_reg(md->index);

	md_cd_lock_modem_clock_src(0);

}

#ifndef CCCI_KMODULE_ENABLE
void md_cd_dump_pccif_reg(struct ccci_modem *md)
{
	struct md_hw_info *hw_info = md->hw_info;

	md_cd_lock_modem_clock_src(1);

	((void)0);
	((void)0);
	((void)0);
	((void)0);
	((void)0);
	((void)0);

	md_cd_lock_modem_clock_src(0);
}
#endif
static void md_cd_check_emi_state(struct ccci_modem *md, int polling)
{
}

static void md1_pmic_setting_on(void)
{
	int ret = 0;

	ret = regulator_set_voltage(reg_vmodem, 875000, 875000);
	if (ret)
		((void)0);
	ret = regulator_sync_voltage(reg_vmodem);
	if (ret)
		((void)0);

	ret = regulator_set_voltage(reg_vsram, 993750, 993750);
	if (ret)
		((void)0);
	ret = regulator_sync_voltage(reg_vsram);
	if (ret)
		((void)0);

}

void __attribute__((weak)) kicker_pbm_by_md(enum pbm_kicker kicker,
	bool status)
{
}

static int md_cd_soft_power_off(struct ccci_modem *md, unsigned int mode)
{
	clk_buf_set_by_flightmode(true);
	return 0;
}

static int md_cd_soft_power_on(struct ccci_modem *md, unsigned int mode)
{
	clk_buf_set_by_flightmode(false);
	return 0;
}

static int md_start_platform(struct ccci_modem *md)
{
	struct device_node *node = NULL;
	void __iomem *sec_ao_base = NULL;
	int timeout = 100; /* 100 * 20ms = 2s */
	int ret = -1;
#ifndef USING_PM_RUNTIME
	int retval = 0;
#endif

	if ((md->per_md_data.config.setting&MD_SETTING_FIRST_BOOT) == 0)
		return 0;

	reg_vmodem = devm_regulator_get_optional(&md->plat_dev->dev, "vmodem");
	if (IS_ERR(reg_vmodem)) {
		ret = PTR_ERR(reg_vmodem);
		if ((ret != -ENODEV) && md->plat_dev->dev.of_node) {
			((void)0);
			return -1;
		}
	}
	reg_vsram = devm_regulator_get_optional(&md->plat_dev->dev, "vsram");
	if (IS_ERR(reg_vsram)) {
		ret = PTR_ERR(reg_vsram);
		if ((ret != -ENODEV) && md->plat_dev->dev.of_node) {
			((void)0);
			return -1;
		}
	}

	node = of_find_compatible_node(NULL, NULL, "mediatek,security_ao");
	if (node) {
		sec_ao_base = of_iomap(node, 0);
		if (sec_ao_base == NULL) {
			((void)0);
			return -1;
		}
	} else {
		sec_ao_base = ioremap_nocache(0x1001a000, 4);
		if (sec_ao_base == NULL) {
			((void)0);
			return -1;
		}
	}

	while (timeout > 0) {
		if (ccci_read32(sec_ao_base, 0x824) == 0x01 &&
			ccci_read32(sec_ao_base, 0x828) == 0x01 &&
			ccci_read32(sec_ao_base, 0x82C) == 0x01 &&
			ccci_read32(sec_ao_base, 0x830) == 0x01) {
			((void)0);
			ret = 0;
			break;
		}
		timeout--;
		msleep(20);
	}
#ifndef USING_PM_RUNTIME
	((void)0);
	retval = clk_prepare_enable(clk_table[0].clk_ref); /* match lk on */
	if (retval)
		((void)0);
	((void)0);
#endif

	md_cd_dump_md_bootup_status(md);

	md_cd_power_off(md, 0);

	if (ret != 0) {
		/* BROM */
		((void)0);
	}

	return ret;
}

static int md_cd_power_on(struct ccci_modem *md)
{
	int ret = 0;
	unsigned int reg_value;

	/* step 1: PMIC setting */
	md1_pmic_setting_on();

	/* step 2: MD srcclkena setting */
	reg_value = ccci_read32(md->hw_info->plat_val->infra_ao_base,
		INFRA_AO_MD_SRCCLKENA);
	reg_value &= ~(0xFF);
	reg_value |= 0x21;
	ccci_write32(md->hw_info->plat_val->infra_ao_base,
		INFRA_AO_MD_SRCCLKENA, reg_value);
	((void)0);

	/* steip 3: power on MD_INFRA and MODEM_TOP */
	switch (md->index) {
	case MD_SYS1:
		clk_buf_set_by_flightmode(false);
		((void)0);
#ifdef USING_PM_RUNTIME
		pm_runtime_get_sync(&md->plat_dev->dev);
#else
		ret = clk_prepare_enable(clk_table[0].clk_ref);
#endif

		((void)0);
		kicker_pbm_by_md(KR_MD1, true);
		((void)0);
		break;
	}
	if (ret)
		return ret;

#ifdef FEATURE_INFORM_NFC_VSIM_CHANGE
	/* notify NFC */
	inform_nfc_vsim_change(md->index, 1, 0);
#endif

	return 0;
}

static int md_cd_let_md_go(struct ccci_modem *md)
{
	struct md_sys1_info *md_info = (struct md_sys1_info *)md->private_data;

	if (MD_IN_DEBUG(md))
		return -1;
	((void)0);

	/* make boot vector take effect */
	ccci_write32(md_info->md_boot_slave_En, 0, 1);
	((void)0);

	return 0;
}

static int md_cd_power_off(struct ccci_modem *md, unsigned int timeout)
{
	int ret = 0;
	unsigned int reg_value;

#ifdef FEATURE_INFORM_NFC_VSIM_CHANGE
	/* notify NFC */
	inform_nfc_vsim_change(md->index, 0, 0);
#endif
	/* Get infra cfg ao base */

	/* power off MD_INFRA and MODEM_TOP */
	switch (md->index) {
	case MD_SYS1:
		/* 1. power off MD MTCMOS */
#ifdef USING_PM_RUNTIME
		pm_runtime_put_sync(&md->plat_dev->dev);
		((void)0);
#else
		clk_disable_unprepare(clk_table[0].clk_ref);
		((void)0);
#endif
		/* 2. disable srcclkena */

		((void)0);
		reg_value =
			ccci_read32(md->hw_info->plat_val->infra_ao_base,
			INFRA_AO_MD_SRCCLKENA);
		reg_value &= ~(0xFF);
		ccci_write32(md->hw_info->plat_val->infra_ao_base,
			INFRA_AO_MD_SRCCLKENA, reg_value);
		((void)0);
		((void)0);

		clk_buf_set_by_flightmode(true);

		/* 5. DLPT */
		kicker_pbm_by_md(KR_MD1, false);
		((void)0);
		break;
	}
	return ret;
}

void ccci_modem_plt_resume(struct ccci_modem *md)
{
	enum MD_STATE md_state = ccci_fsm_get_md_state(md->index);

	((void)0);

	if (md_state == GATED || md_state == WAITING_TO_STOP ||
		md_state == INVALID) {
		((void)0);
		return;
	}

	ccci_hif_resume(md->index, md->hif_flag);
}

int ccci_modem_plt_suspend(struct ccci_modem *md)
{
	((void)0);

	ccci_hif_suspend(md->index, md->hif_flag);

	return 0;
}

static int ccci_modem_remove(struct platform_device *dev)
{
	return 0;
}

static void ccci_modem_shutdown(struct platform_device *dev)
{
}

static int ccci_modem_suspend(struct platform_device *dev, pm_message_t state)
{
	struct ccci_modem *md = (struct ccci_modem *)dev->dev.platform_data;

	((void)0);
	return 0;
}

static int ccci_modem_resume(struct platform_device *dev)
{
	struct ccci_modem *md = (struct ccci_modem *)dev->dev.platform_data;

	((void)0);
	return 0;
}

static int ccci_modem_pm_suspend(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);

	if (pdev == NULL) {
		((void)0);
		return -1;
	}
	return ccci_modem_suspend(pdev, PMSG_SUSPEND);
}

static int ccci_modem_pm_resume(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);

	if (pdev == NULL) {
		((void)0);
		return -1;
	}
	return ccci_modem_resume(pdev);
}

static int ccci_modem_pm_restore_noirq(struct device *device)
{
	struct ccci_modem *md = (struct ccci_modem *)device->platform_data;

	/* set flag for next md_start */
	md->per_md_data.config.setting |= MD_SETTING_RELOAD;
	md->per_md_data.config.setting |= MD_SETTING_FIRST_BOOT;
	/* restore IRQ */
#ifdef FEATURE_PM_IPO_H
	irq_set_irq_type(md_ctrl->cldma_irq_id, IRQF_TRIGGER_HIGH);
	irq_set_irq_type(md_ctrl->md_wdt_irq_id, IRQF_TRIGGER_RISING);
#endif
	return 0;
}

#include <linux/module.h>
#include <linux/platform_device.h>

static int ccci_modem_probe(struct platform_device *plat_dev)
{
	struct ccci_dev_cfg dev_cfg;
	int ret;
	struct md_hw_info *md_hw;

	/* Allocate modem hardware info structure memory */
	md_hw = kzalloc(sizeof(struct md_hw_info), GFP_KERNEL);
	if (md_hw == NULL) {
		((void)0);
		return -1;
	}
	ret = md_cd_get_modem_hw_info(plat_dev, &dev_cfg, md_hw);
	if (ret != 0) {
		((void)0);
		kfree(md_hw);
		md_hw = NULL;
		return -1;
	}
#ifdef CCCI_KMODULE_ENABLE
	ccci_init();
#endif
	ret = ccci_modem_init_common(plat_dev, &dev_cfg, md_hw);
	if (ret < 0) {
		kfree(md_hw);
		md_hw = NULL;
	}
	return ret;
}

static const struct dev_pm_ops ccci_modem_pm_ops = {
	.suspend = ccci_modem_pm_suspend,
	.resume = ccci_modem_pm_resume,
	.freeze = ccci_modem_pm_suspend,
	.thaw = ccci_modem_pm_resume,
	.poweroff = ccci_modem_pm_suspend,
	.restore = ccci_modem_pm_resume,
	.restore_noirq = ccci_modem_pm_restore_noirq,
};

#ifdef CONFIG_OF
static const struct of_device_id ccci_modem_of_ids[] = {
	{.compatible = "mediatek,mddriver-mt6779",},
	{}
};
#endif

static struct platform_driver ccci_modem_driver = {

	.driver = {
		   .name = "driver_modem",
#ifdef CONFIG_OF
		   .of_match_table = ccci_modem_of_ids,
#endif

#ifdef CONFIG_PM
		   .pm = &ccci_modem_pm_ops,
#endif
		   },
	.probe = ccci_modem_probe,
	.remove = ccci_modem_remove,
	.shutdown = ccci_modem_shutdown,
	.suspend = ccci_modem_suspend,
	.resume = ccci_modem_resume,
};

static int __init modem_cd_init(void)
{
	int ret;

	ret = platform_driver_register(&ccci_modem_driver);
	if (ret) {
		((void)0);
		return ret;
	}
	return 0;
}

module_init(modem_cd_init);

MODULE_AUTHOR("CCCI");
MODULE_DESCRIPTION("CCCI modem driver v0.1");
MODULE_LICENSE("GPL");

