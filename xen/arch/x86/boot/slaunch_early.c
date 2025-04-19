/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#include <xen/slr_table.h>
#include <xen/types.h>
#include <asm/intel_txt.h>

struct early_init_results
{
    uint32_t mbi_pa;
    uint32_t slrt_pa;
} __packed;

void slaunch_early_init(uint32_t load_base_addr,
                        uint32_t tgt_base_addr,
                        uint32_t tgt_end_addr,
                        struct early_init_results *result)
{
    void *txt_heap;
    struct txt_os_mle_data *os_mle;
    struct slr_table *slrt;
    struct slr_entry_intel_info *intel_info;

    txt_heap = txt_init();
    os_mle = txt_os_mle_data_start(txt_heap);

    result->slrt_pa = os_mle->slrt;
    result->mbi_pa = 0;

    slrt = (struct slr_table *)(uintptr_t)os_mle->slrt;

    intel_info = (struct slr_entry_intel_info *)
        slr_next_entry_by_tag(slrt, NULL, SLR_ENTRY_INTEL_INFO);
    if ( intel_info == NULL || intel_info->hdr.size != sizeof(*intel_info) )
        return;

    result->mbi_pa = intel_info->boot_params_base;
}
