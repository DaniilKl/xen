/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#ifndef ASM__X86__SLAUNCH_H
#define ASM__X86__SLAUNCH_H

#include <xen/types.h>

/* Indicates an active Secure Launch boot. */
extern bool slaunch_active;

/*
 * Holds physical address of SLRT.  Use slaunch_get_slrt() to access SLRT
 * instead of mapping where this points to.
 */
extern uint32_t slaunch_slrt;

/*
 * Retrieves pointer to SLRT.  Checks table's validity and maps it as necessary.
 */
struct slr_table *slaunch_get_slrt(void);

#endif /* ASM__X86__SLAUNCH_H */
