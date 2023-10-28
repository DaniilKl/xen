#ifndef _ASM_X86_TPM_H_
#define _ASM_X86_TPM_H_

#include <asm/bootinfo.h>
#include <xen/types.h>

#define TPM_TIS_BASE  0xFED40000
#define TPM_TIS_SIZE  0x00010000

void tpm_hash_extend(unsigned loc, unsigned pcr, uint8_t *buf, unsigned size,
                     uint32_t type, uint8_t *log_data, unsigned log_data_size);

/* Measures essential parts of SLR table before making use of them. */
void tpm_measure_slrt(void);

/* Takes measurements of DRTM policy entries except for MBI and SLRT which
 * should have been measured by the time this is called. Also performs sanity
 * checks of the policy and panics on failure. In particular, the function
 * verifies that DRTM is consistent with modules obtained from MultibootInfo
 * (MBI) and written to struct boot_info in setup.c. */
void tpm_process_drtm_policy(const struct boot_info *bi);

#endif /* _ASM_X86_TPM_H_ */
