/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#ifndef _ASM_X86_SLAUNCH_H_
#define _ASM_X86_SLAUNCH_H_

#include <xen/slr_table.h>
#include <xen/types.h>

extern bool slaunch_active;

/*
 * evt_log is assigned a physical address and the caller must map it to
 * virtual, if needed.
 */
static inline void find_evt_log(struct slr_table *slrt, void **evt_log,
                                uint32_t *evt_log_size)
{
    struct slr_entry_log_info *log_info;

    log_info = (struct slr_entry_log_info *)
        slr_next_entry_by_tag(slrt, NULL, SLR_ENTRY_LOG_INFO);
    if ( log_info != NULL )
    {
        *evt_log = _p(log_info->addr);
        *evt_log_size = log_info->size;
    }
    else
    {
        *evt_log = NULL;
        *evt_log_size = 0;
    }
}

/*
 * Retrieves pointer to SLRT.  Checks table's validity and maps it as necessary.
 */
struct slr_table *slaunch_get_slrt(void);

/*
 * Prepares for accesses to essential data structures setup by boot environment.
 */
void slaunch_map_mem_regions(void);

/* Marks regions of memory as used to avoid their corruption. */
void slaunch_reserve_mem_regions(void);

/*
 * This helper function is used to map memory using L2 page tables by aligning
 * mapped regions to 2MB. This way page allocator (which at this point isn't
 * yet initialized) isn't needed for creating new L1 mappings. The function
 * also checks and skips memory already mapped by the prebuilt tables.
 *
 * There is no unmap_l2() because the function is meant to be used by the code
 * that accesses DRTM-related memory soon after which Xen rebuilds memory maps,
 * effectively dropping all existing mappings.
 */
int slaunch_map_l2(unsigned long paddr, unsigned long size);

#endif /* _ASM_X86_SLAUNCH_H_ */
