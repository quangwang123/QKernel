/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015 MediaTek Inc.
 */

#ifndef __CCCI_UTIL_LOG_H__
#define __CCCI_UTIL_LOG_H__

#define CCCI_UTIL_DBG_MSG(fmt, args...) \
	pr_debug("ccci0/util: " fmt, ##args)
#define CCCI_UTIL_INF_MSG(fmt, args...) \
	pr_debug("ccci0/util: " fmt, ##args)
#define CCCI_UTIL_ERR_MSG(fmt, args...) \
	pr_err("ccci0/util: " fmt, ##args)

#define CCCI_UTIL_DBG_MSG_WITH_ID(id, fmt, args...) \
	pr_debug("ccci%d/util: " fmt, (id) + 1, ##args)
#define CCCI_UTIL_INF_MSG_WITH_ID(id, fmt, args...) \
	pr_debug("ccci%d/util: " fmt, (id) + 1, ##args)
#define CCCI_UTIL_NOTICE_MSG_WITH_ID(id, fmt, args...) \
	pr_notice("ccci%d/util: " fmt, (id) + 1, ##args)
#define CCCI_UTIL_ERR_MSG_WITH_ID(id, fmt, args...) \
	pr_err("ccci%d/util: " fmt, (id) + 1, ##args)
#endif /*__CCCI_UTIL_LOG_H__ */
