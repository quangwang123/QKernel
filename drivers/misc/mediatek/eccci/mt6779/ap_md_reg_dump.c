// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */
#include "ccci_core.h"
#include "ccci_platform.h"

#include "md_sys1_platform.h"
#include "cldma_reg.h"
#include "modem_reg_base.h"
#include "modem_secure_base.h"
#include "ap_md_reg_dump.h"

#define TAG "mcd"

#define RAnd2W(a, b, c)  ccci_write32(a, b, (ccci_read32(a, b)&c))

/*
 * This file is generated.
 * From 20181114_Latife_MDReg_remap.xlsx
 * With ap_md_reg_dump_code_gentool.py v0.1
 * Date 2018-11-14 13:02:08.882000
 */
static void internal_md_dump_debug_register(unsigned int md_index)
{
	void __iomem *dump_reg0;

	/* dump AP_MDSRC_REQ */
	dump_reg0 = ioremap_nocache(0x10006434, 0x4);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	iounmap(dump_reg0);

	/* PC Monitor */
	dump_reg0 = ioremap_nocache(0x0D0D9000, 0x1360);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	/* Stop PCMon */
	mdreg_write32(MD_REG_PC_MONITOR, 0x222);
	((void)0);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001000), 0x100, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001100), 0x60, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001200), 0x60, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001300), 0x60, false);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x400, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000400), 0x400, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000800), 0x400, false);
	/* Re-Start PCMon */
	mdreg_write32(MD_REG_PC_MONITOR, 0x111);
	iounmap(dump_reg0);

	/* PLL reg (clock control) */
	dump_reg0 = ioremap_nocache(0x0D0C3800, 0x1C85C);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00012800), 0x110, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00012A00), 0x20, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00013700), 0x8, false);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00010800), 0x68, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00010900), 0x30, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00010A00), 0x8, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00010B00), 0x20, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00010C00), 0x60, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00010D00), 0xD0, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011400), 0x48, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011500), 0x8, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011700), 0x14, false);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x1C, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000110), 0x20, false);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011800), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011890), 0x80, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011A00), 0x80, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011B00), 0x70, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00011F00), 0x50, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00012000), 0x30, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00012100), 0x8, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00012500), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00012700), 0x8, false);
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x0001C850), 0xC, false);
	iounmap(dump_reg0);

	/* BUS */
	dump_reg0 = ioremap_nocache(0x0D0C7000, 0x19098);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0xE0, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002000), 0x110, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00019000), 0x98, false);
	iounmap(dump_reg0);

	/* BUSMON  */
	dump_reg0 = ioremap_nocache(0x0D0C6000, 0x291C);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x104, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000200), 0x1C, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000220), 0x30, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000280), 0x1C, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x000002A0), 0x30, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000400), 0x51C, false);
	/* [Pre-Action] Disable bus his rec & select entry 0 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 1 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x100010);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 2 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x200020);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 3 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x300030);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 4 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x400040);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 5 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x500050);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 6 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x600060);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	/* [Pre-Action] Select entry 7 */
	mdreg_write32(MD_REG_MDMCU_BUSMON, 0x700070);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000860), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002000), 0x104, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002200), 0x1C, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002220), 0x30, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002280), 0x1C, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x000022A0), 0x30, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002400), 0x51C, false);
	/* [Pre-Action] Disable bus his rec & select entry 0 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 1 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x100010);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 2 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x200020);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 3 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x300030);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 4 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x400040);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 5 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x500050);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 6 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x600060);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	/* [Pre-Action] Select entry 7 */
	mdreg_write32(MD_REG_MDINFRA_BUSMON, 0x700070);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002830), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00002860), 0xC, false);
	iounmap(dump_reg0);

	/* ECT */
	dump_reg0 = ioremap_nocache(0x0D0CC130, 0x1EE8);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000004), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001000), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001004), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001EE4), 0x4, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001EDC), 0x4, false);
	iounmap(dump_reg0);

	/* TOPSM reg */
	dump_reg0 = ioremap_nocache(0x0D0D0000, 0x8E4);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x8E4, false);
	iounmap(dump_reg0);

	/* MD RGU reg */
	dump_reg0 = ioremap_nocache(0x0D0D2100, 0x25C);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0xCC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000200), 0x5C, false);
	iounmap(dump_reg0);

	/* OST status */
	dump_reg0 = ioremap_nocache(0x0D0D1000, 0x208);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0xF0, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000200), 0x8, false);
	iounmap(dump_reg0);

	/* CSC reg */
	dump_reg0 = ioremap_nocache(0x0D0D3000, 0x214);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x214, false);
	iounmap(dump_reg0);

	/* ELM reg */
	dump_reg0 = ioremap_nocache(0x20350000, 0x52C);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
#if defined(__MD_DEBUG_DUMP__)
	((void)0);
#endif
#if defined(__MD_DEBUG_DUMP__)
	/* This dump might cause bus hang so enable it only when needed */
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x52C, false);
#endif
	iounmap(dump_reg0);

	/* USIP */
	dump_reg0 = ioremap_nocache(0x0D0C4400, 0x3500);
	if (dump_reg0 == NULL) {
		((void)0);
		return;
	}
	((void)0);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000000), 0x100, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00000210), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001000), 0x100, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00001210), 0xC, false);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00003400), 0x100, false);
	/* [Pre-Action] config usip bus dbg sel 8 */
	mdreg_write32(MD_REG_USIP, 0x20001F);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00003400), 0xA0, false);
	/* [Pre-Action] config usip bus dbg sel 9 */
	mdreg_write32(MD_REG_USIP, 0x24001F);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00003400), 0xA0, false);
	/* [Pre-Action] config usip bus dbg sel 10 */
	mdreg_write32(MD_REG_USIP, 0x28001F);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00003400), 0xA0, false);
	/* [Pre-Action] config usip bus dbg sel 11 */
	mdreg_write32(MD_REG_USIP, 0x2C001F);
	print_hex_dump_debug("ccci: ", DUMP_PREFIX_OFFSET, 16, 4,
			     (dump_reg0 + 0x00003400), 0xA0, false);
	iounmap(dump_reg0);
}

void md_dump_register_6779(unsigned int md_index)
{

	internal_md_dump_debug_register(md_index);
}
