/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#ifndef _ASM_X86_TPM_H_
#define _ASM_X86_TPM_H_

#include <xen/types.h>

#define TPM_TIS_BASE  0xFED40000
#define TPM_TIS_SIZE  0x00010000

void tpm_hash_extend(unsigned loc, unsigned pcr, const uint8_t *buf,
                     unsigned size, uint32_t type, const uint8_t *log_data,
                     unsigned log_data_size);

#endif /* _ASM_X86_TPM_H_ */
