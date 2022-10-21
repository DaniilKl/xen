/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2022-2025 3mdeb Sp. z o.o. All rights reserved.
 */

#ifndef X86_TPM_H
#define X86_TPM_H

#include <xen/types.h>

#define TPM_TIS_BASE  0xfed40000U
#define TPM_TIS_SIZE  0x00010000U

void tpm_hash_extend(unsigned loc, unsigned pcr, const uint8_t *buf,
                     unsigned size, uint32_t type, const uint8_t *log_data,
                     unsigned log_data_size);

#endif /* X86_TPM_H */
