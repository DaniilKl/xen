/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#include <xen/bug.h>
#include <xen/init.h>
#include <xen/lib.h>
#include <xen/types.h>
#include <asm/e820.h>
#include <asm/intel-txt.h>
#include <asm/slaunch.h>

static uint64_t __initdata txt_heap_base, txt_heap_size;

void __init txt_map_mem_regions(void)
{
    int rc;

    rc = slaunch_map_l2(TXT_PRIV_CONFIG_REGS_BASE, TXT_CONFIG_SPACE_SIZE);
    BUG_ON(rc != 0);

    txt_heap_base = txt_read(TXTCR_HEAP_BASE);
    BUG_ON(txt_heap_base == 0);

    txt_heap_size = txt_read(TXTCR_HEAP_SIZE);
    BUG_ON(txt_heap_size == 0);

    rc = slaunch_map_l2(txt_heap_base, txt_heap_size);
    BUG_ON(rc != 0);
}

/* Mark RAM region as RESERVED if it isn't marked that way already. */
static int __init mark_ram_as(struct e820map *map, uint64_t start,
                              uint64_t end, uint32_t type)
{
    unsigned int i;
    uint32_t from_type = E820_RAM;

    for ( i = 0; i < map->nr_map; i++ )
    {
        uint64_t rs = map->map[i].addr;
        uint64_t re = rs + map->map[i].size;

        /* The entry includes the range. */
        if ( start >= rs && end <= re )
            break;

        /* The entry intersects the range. */
        if ( end > rs && start < re )
        {
            /* Fatal failure. */
            return 0;
        }
    }

    /*
     * If the range is not included by any entry and no entry intersects it,
     * then it's not listed in the memory map.  Consider this case as a success
     * since we're only preventing RAM from being used and unlisted range should
     * not be used.
     */
    if ( i == map->nr_map )
        return 1;

    /*
     * e820_change_range_type() fails if the range is already marked with the
     * desired type.  Don't consider it an error if firmware has done it for us.
     */
    if ( map->map[i].type == type )
        return 1;

    /* E820_ACPI or E820_NVS are really unexpected, but others are fine. */
    if ( map->map[i].type == E820_RESERVED ||
         map->map[i].type == E820_UNUSABLE )
        from_type = map->map[i].type;

    return e820_change_range_type(map, start, end, from_type, type);
}

void __init txt_reserve_mem_regions(void)
{
    int rc;
    uint64_t sinit_base, sinit_size;

    /* TXT Heap */
    BUG_ON(txt_heap_base == 0);
    printk("SLAUNCH: reserving TXT heap (%#lx - %#lx)\n", txt_heap_base,
           txt_heap_base + txt_heap_size);
    rc = mark_ram_as(&e820_raw, txt_heap_base, txt_heap_base + txt_heap_size,
                     E820_RESERVED);
    BUG_ON(rc == 0);

    sinit_base = txt_read(TXTCR_SINIT_BASE);
    BUG_ON(sinit_base == 0);

    sinit_size = txt_read(TXTCR_SINIT_SIZE);
    BUG_ON(sinit_size == 0);

    /* SINIT */
    printk("SLAUNCH: reserving SINIT memory (%#lx - %#lx)\n", sinit_base,
           sinit_base + sinit_size);
    rc = mark_ram_as(&e820_raw, sinit_base, sinit_base + sinit_size,
                     E820_RESERVED);
    BUG_ON(rc == 0);

    /* TXT Private Space */
    rc = mark_ram_as(&e820_raw, TXT_PRIV_CONFIG_REGS_BASE,
                     TXT_PRIV_CONFIG_REGS_BASE + TXT_CONFIG_SPACE_SIZE,
                     E820_UNUSABLE);
    BUG_ON(rc == 0);
}
