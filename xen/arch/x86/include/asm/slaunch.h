/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#ifndef _ASM_X86_SLAUNCH_H_
#define _ASM_X86_SLAUNCH_H_

#include <xen/types.h>

extern bool slaunch_active;

/*
 * Retrieves pointer to SLRT.  Checks table's validity and maps it as necessary.
 */
struct slr_table *slaunch_get_slrt(void);

#endif /* _ASM_X86_SLAUNCH_H_ */
