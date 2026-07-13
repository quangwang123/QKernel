// SPDX-License-Identifier: GPL-2.0
/*
 * Reclaim the bootloader log buffer on low-memory devices. Persistent kernel
 * logs are handled by pstore/ramoops, so retaining this second buffer after
 * boot only removes usable pages from Linux.
 */

#include <linux/init.h>
#include <linux/memblock.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>

static int __init mtk_log_store_reclaim(struct reserved_mem *rmem)
{
	phys_addr_t base = rmem->base;
	phys_addr_t size = rmem->size;

	if (!base || !size || !memblock_is_region_memory(base, size)) {
		pr_warn("log_store: cannot reclaim non-RAM region %pa+%pa\n",
			&base, &size);
		return -EINVAL;
	}

	if (memblock_free(base, size)) {
		pr_warn("log_store: failed to release region %pa+%pa\n",
			&base, &size);
		return -EINVAL;
	}

	pr_info("log_store: reclaimed %pa bytes at %pa\n", &size, &base);
	rmem->base = 0;
	rmem->size = 0;
	return 0;
}

RESERVEDMEM_OF_DECLARE(mtk_log_store_reclaim, "mediatek,log_store",
			       mtk_log_store_reclaim);
