/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef _H_DDP_LOG_
#define _H_DDP_LOG_
#ifdef CONFIG_MTK_AEE_FEATURE
#include "mt-plat/aee.h"
#endif
#include "display_recorder.h"
#include "ddp_debug.h"
#include "disp_drv_log.h"

#ifndef LOG_TAG
#define LOG_TAG
#endif

#define DDPSVPMSG(fmt, args...) ((void)0)

#define DISP_LOG_I(fmt, args...)	((void)0)

#define DISP_LOG_V(fmt, args...)	((void)0)

#define DISP_LOG_D(fmt, args...)	((void)0)

#define DISP_LOG_W(fmt, args...)    ((void)0)

#define DISP_LOG_E(fmt, args...)	((void)0)

#define DDPIRQ(fmt, args...)	((void)0)

#define DDPDBG(fmt, args...) ((void)0)

#define DDPMSG(fmt, args...) ((void)0)

#define DDPWRN(fmt, args...) ((void)0)

#define DDPERR(fmt, args...) ((void)0)

#define DDPDUMP(fmt, ...)	((void)0)

#ifndef ASSERT
#define ASSERT(expr)	((void)0)
#endif

#ifdef CONFIG_MTK_AEE_FEATURE
#define DDPAEE(string, args...)	((void)0)
#else
#define DDPAEE(string, args...)	((void)0)
#endif

#endif
