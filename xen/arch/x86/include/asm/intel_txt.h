/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * TXT configuration registers (offsets from TXT_{PUB, PRIV}_CONFIG_REGS_BASE)
 */
#define TXT_PUB_CONFIG_REGS_BASE        0xfed30000
#define TXT_PRIV_CONFIG_REGS_BASE       0xfed20000

/*
 * The same set of registers is exposed twice (with different permissions) and
 * they are allocated continuously with page alignment.
 */
#define NR_TXT_CONFIG_SIZE \
    (TXT_PUB_CONFIG_REGS_BASE - TXT_PRIV_CONFIG_REGS_BASE)

/* Offsets from pub/priv config space. */
#define TXTCR_STS                       0x0000
#define TXTCR_ESTS                      0x0008
#define TXTCR_ERRORCODE                 0x0030
#define TXTCR_CMD_RESET                 0x0038
#define TXTCR_CMD_CLOSE_PRIVATE         0x0048
#define TXTCR_DIDVID                    0x0110
#define TXTCR_VER_EMIF                  0x0200
#define TXTCR_CMD_UNLOCK_MEM_CONFIG     0x0218
#define TXTCR_SINIT_BASE                0x0270
#define TXTCR_SINIT_SIZE                0x0278
#define TXTCR_MLE_JOIN                  0x0290
#define TXTCR_HEAP_BASE                 0x0300
#define TXTCR_HEAP_SIZE                 0x0308
#define TXTCR_SCRATCHPAD                0x0378
#define TXTCR_CMD_OPEN_LOCALITY1        0x0380
#define TXTCR_CMD_CLOSE_LOCALITY1       0x0388
#define TXTCR_CMD_OPEN_LOCALITY2        0x0390
#define TXTCR_CMD_CLOSE_LOCALITY2       0x0398
#define TXTCR_CMD_SECRETS               0x08e0
#define TXTCR_CMD_NO_SECRETS            0x08e8
#define TXTCR_E2STS                     0x08f0

/*
 * Secure Launch Defined Error Codes used in MLE-initiated TXT resets.
 *
 * TXT Specification
 * Appendix I ACM Error Codes
 */
#define SLAUNCH_ERROR_GENERIC                0xc0008001
#define SLAUNCH_ERROR_TPM_INIT               0xc0008002
#define SLAUNCH_ERROR_TPM_INVALID_LOG20      0xc0008003
#define SLAUNCH_ERROR_TPM_LOGGING_FAILED     0xc0008004
#define SLAUNCH_ERROR_REGION_STRADDLE_4GB    0xc0008005
#define SLAUNCH_ERROR_TPM_EXTEND             0xc0008006
#define SLAUNCH_ERROR_MTRR_INV_VCNT          0xc0008007
#define SLAUNCH_ERROR_MTRR_INV_DEF_TYPE      0xc0008008
#define SLAUNCH_ERROR_MTRR_INV_BASE          0xc0008009
#define SLAUNCH_ERROR_MTRR_INV_MASK          0xc000800a
#define SLAUNCH_ERROR_MSR_INV_MISC_EN        0xc000800b
#define SLAUNCH_ERROR_INV_AP_INTERRUPT       0xc000800c
#define SLAUNCH_ERROR_INTEGER_OVERFLOW       0xc000800d
#define SLAUNCH_ERROR_HEAP_WALK              0xc000800e
#define SLAUNCH_ERROR_HEAP_MAP               0xc000800f
#define SLAUNCH_ERROR_REGION_ABOVE_4GB       0xc0008010
#define SLAUNCH_ERROR_HEAP_INVALID_DMAR      0xc0008011
#define SLAUNCH_ERROR_HEAP_DMAR_SIZE         0xc0008012
#define SLAUNCH_ERROR_HEAP_DMAR_MAP          0xc0008013
#define SLAUNCH_ERROR_HI_PMR_BASE            0xc0008014
#define SLAUNCH_ERROR_HI_PMR_SIZE            0xc0008015
#define SLAUNCH_ERROR_LO_PMR_BASE            0xc0008016
#define SLAUNCH_ERROR_LO_PMR_SIZE            0xc0008017
#define SLAUNCH_ERROR_LO_PMR_MLE             0xc0008018
#define SLAUNCH_ERROR_INITRD_TOO_BIG         0xc0008019
#define SLAUNCH_ERROR_HEAP_ZERO_OFFSET       0xc000801a
#define SLAUNCH_ERROR_WAKE_BLOCK_TOO_SMALL   0xc000801b
#define SLAUNCH_ERROR_MLE_BUFFER_OVERLAP     0xc000801c
#define SLAUNCH_ERROR_BUFFER_BEYOND_PMR      0xc000801d
#define SLAUNCH_ERROR_OS_SINIT_BAD_VERSION   0xc000801e
#define SLAUNCH_ERROR_EVENTLOG_MAP           0xc000801f
#define SLAUNCH_ERROR_TPM_NUMBER_ALGS        0xc0008020
#define SLAUNCH_ERROR_TPM_UNKNOWN_DIGEST     0xc0008021
#define SLAUNCH_ERROR_TPM_INVALID_EVENT      0xc0008022

#define SLAUNCH_BOOTLOADER_MAGIC             0x4c534254

#ifndef __ASSEMBLY__

#include <xen/slr_table.h>

/* Need to differentiate between pre- and post paging enabled. */
#ifdef __EARLY_SLAUNCH__
#include <xen/macros.h>
#define _txt(x) _p(x)
#else
#include <xen/types.h>
#include <asm/page.h>   // __va()
#define _txt(x) __va(x)
#endif

/*
 * Always use private space as some of registers are either read-only or not
 * present in public space.
 */
static inline uint64_t read_txt_reg(int reg_no)
{
    volatile uint64_t *reg = _txt(TXT_PRIV_CONFIG_REGS_BASE + reg_no);
    return *reg;
}

static inline void write_txt_reg(int reg_no, uint64_t val)
{
    volatile uint64_t *reg = _txt(TXT_PRIV_CONFIG_REGS_BASE + reg_no);
    *reg = val;
    /* This serves as TXT register barrier */
    (void)read_txt_reg(TXTCR_ESTS);
}

static inline void txt_reset(uint32_t error)
{
    write_txt_reg(TXTCR_ERRORCODE, error);
    write_txt_reg(TXTCR_CMD_NO_SECRETS, 1);
    write_txt_reg(TXTCR_CMD_UNLOCK_MEM_CONFIG, 1);
    write_txt_reg(TXTCR_CMD_RESET, 1);
    while (1);
}

/*
 * Secure Launch defined OS/MLE TXT Heap table
 */
struct txt_os_mle_data {
    uint32_t version;
    uint32_t reserved;
    uint64_t slrt;
    uint64_t txt_info;
    uint32_t ap_wake_block;
    uint32_t ap_wake_block_size;
    uint8_t mle_scratch[64];
} __packed;

/*
 * TXT specification defined BIOS data TXT Heap table
 */
struct txt_bios_data {
    uint32_t version; /* Currently 5 for TPM 1.2 and 6 for TPM 2.0 */
    uint32_t bios_sinit_size;
    uint64_t reserved1;
    uint64_t reserved2;
    uint32_t num_logical_procs;
    /* Versions >= 3 && < 5 */
    uint32_t sinit_flags;
    /* Versions >= 5 with updates in version 6 */
    uint32_t mle_flags;
    /* Versions >= 4 */
    /* Ext Data Elements */
} __packed;

/*
 * TXT specification defined OS/SINIT TXT Heap table
 */
struct txt_os_sinit_data {
    uint32_t version;       /* Currently 6 for TPM 1.2 and 7 for TPM 2.0 */
    uint32_t flags;         /* Reserved in version 6 */
    uint64_t mle_ptab;
    uint64_t mle_size;
    uint64_t mle_hdr_base;
    uint64_t vtd_pmr_lo_base;
    uint64_t vtd_pmr_lo_size;
    uint64_t vtd_pmr_hi_base;
    uint64_t vtd_pmr_hi_size;
    uint64_t lcp_po_base;
    uint64_t lcp_po_size;
    uint32_t capabilities;
    /* Version = 5 */
    uint64_t efi_rsdt_ptr;  /* RSD*P* in versions >= 6 */
    /* Versions >= 6 */
    /* Ext Data Elements */
} __packed;

/*
 * TXT specification defined SINIT/MLE TXT Heap table
 */
struct txt_sinit_mle_data {
    uint32_t version;  /* Current values are 6 through 9 */
    /* Versions <= 8, fields until lcp_policy_control must be 0 for >= 9 */
    uint8_t bios_acm_id[20];
    uint32_t edx_senter_flags;
    uint64_t mseg_valid;
    uint8_t sinit_hash[20];
    uint8_t mle_hash[20];
    uint8_t stm_hash[20];
    uint8_t lcp_policy_hash[20];
    uint32_t lcp_policy_control;
    /* Versions >= 7 */
    uint32_t rlp_wakeup_addr;
    uint32_t reserved;
    uint32_t num_of_sinit_mdrs;
    uint32_t sinit_mdrs_table_offset;
    uint32_t sinit_vtd_dmar_table_size;
    uint32_t sinit_vtd_dmar_table_offset;
    /* Versions >= 8 */
    uint32_t processor_scrtm_status;
    /* Versions >= 9 */
    /* Ext Data Elements */
} __packed;

/*
 * Functions to extract data from the Intel TXT Heap Memory. The layout
 * of the heap is as follows:
 *  +------------------------------------+
 *  | Size of Bios Data table (uint64_t) |
 *  +------------------------------------+
 *  | Bios Data table                    |
 *  +------------------------------------+
 *  | Size of OS MLE table (uint64_t)    |
 *  +------------------------------------+
 *  | OS MLE table                       |
 *  +--------------------------------    +
 *  | Size of OS SINIT table (uint64_t)  |
 *  +------------------------------------+
 *  | OS SINIT table                     |
 *  +------------------------------------+
 *  | Size of SINIT MLE table (uint64_t) |
 *  +------------------------------------+
 *  | SINIT MLE table                    |
 *  +------------------------------------+
 *
 *  NOTE: the table size fields include the 8 byte size field itself.
 */
static inline uint64_t txt_bios_data_size(void *heap)
{
    return *((uint64_t *)heap) - sizeof(uint64_t);
}

static inline void *txt_bios_data_start(void *heap)
{
    return heap + sizeof(uint64_t);
}

static inline uint64_t txt_os_mle_data_size(void *heap)
{
    return *((uint64_t *)(txt_bios_data_start(heap) +
                          txt_bios_data_size(heap))) -
        sizeof(uint64_t);
}

static inline void *txt_os_mle_data_start(void *heap)
{
    return txt_bios_data_start(heap) + txt_bios_data_size(heap) +
        sizeof(uint64_t);
}

static inline uint64_t txt_os_sinit_data_size(void *heap)
{
    return *((uint64_t *)(txt_os_mle_data_start(heap) +
                          txt_os_mle_data_size(heap))) -
        sizeof(uint64_t);
}

static inline void *txt_os_sinit_data_start(void *heap)
{
    return txt_os_mle_data_start(heap) + txt_os_mle_data_size(heap) +
        sizeof(uint64_t);
}

static inline uint64_t txt_sinit_mle_data_size(void *heap)
{
    return *((uint64_t *)(txt_os_sinit_data_start(heap) +
                          txt_os_sinit_data_size(heap))) -
        sizeof(uint64_t);
}

static inline void *txt_sinit_mle_data_start(void *heap)
{
    return txt_os_sinit_data_start(heap) + txt_os_sinit_data_size(heap) +
        sizeof(uint64_t);
}

static inline void *txt_init(void)
{
    void *txt_heap;

    /* Clear the TXT error register for a clean start of the day. */
    write_txt_reg(TXTCR_ERRORCODE, 0);

    txt_heap = _p(read_txt_reg(TXTCR_HEAP_BASE));

    if ( txt_os_mle_data_size(txt_heap) < sizeof(struct txt_os_mle_data) ||
         txt_os_sinit_data_size(txt_heap) < sizeof(struct txt_os_sinit_data) )
        txt_reset(SLAUNCH_ERROR_GENERIC);

    return txt_heap;
}

static inline int is_in_pmr(struct txt_os_sinit_data *os_sinit, uint64_t base,
                            uint32_t size, int check_high)
{
    /* Check for size overflow. */
    if ( base + size < base )
        txt_reset(SLAUNCH_ERROR_INTEGER_OVERFLOW);

    /* Low range always starts at 0, so its size is also end address. */
    if ( base >= os_sinit->vtd_pmr_lo_base &&
         base + size <= os_sinit->vtd_pmr_lo_size )
        return 1;

    if ( check_high && os_sinit->vtd_pmr_hi_size != 0 )
    {
        if ( os_sinit->vtd_pmr_hi_base + os_sinit->vtd_pmr_hi_size <
             os_sinit->vtd_pmr_hi_size )
            txt_reset(SLAUNCH_ERROR_INTEGER_OVERFLOW);
        if ( base >= os_sinit->vtd_pmr_hi_base &&
             base + size <= os_sinit->vtd_pmr_hi_base +
                            os_sinit->vtd_pmr_hi_size )
            return 1;
    }

    return 0;
}

static inline void txt_verify_pmr_ranges(struct txt_os_mle_data *os_mle,
                                         struct txt_os_sinit_data *os_sinit,
                                         struct slr_entry_intel_info *info,
                                         uint32_t load_base_addr,
                                         uint32_t tgt_base_addr,
                                         uint32_t xen_size)
{
    int check_high_pmr = 0;

    /* Verify the value of the low PMR base. It should always be 0. */
    if ( os_sinit->vtd_pmr_lo_base != 0 )
        txt_reset(SLAUNCH_ERROR_LO_PMR_BASE);

    /*
     * Low PMR size should not be 0 on current platforms. There is an ongoing
     * transition to TPR-based DMA protection instead of PMR-based; this is not
     * yet supported by the code.
     */
    if ( os_sinit->vtd_pmr_lo_size == 0 )
        txt_reset(SLAUNCH_ERROR_LO_PMR_SIZE);

    /* Check if regions overlap. Treat regions with no hole between as error. */
    if ( os_sinit->vtd_pmr_hi_size != 0 &&
         os_sinit->vtd_pmr_hi_base <= os_sinit->vtd_pmr_lo_size )
        txt_reset(SLAUNCH_ERROR_HI_PMR_BASE);

    /* All regions accessed by 32b code must be below 4G. */
    if ( os_sinit->vtd_pmr_hi_base + os_sinit->vtd_pmr_hi_size <=
         0x100000000ull )
        check_high_pmr = 1;

    /*
     * ACM checks that TXT heap and MLE memory is protected against DMA. We have
     * to check if MBI and whole Xen memory is protected. The latter is done in
     * case bootloader failed to set whole image as MLE and to make sure that
     * both pre- and post-relocation code is protected.
     */

    /* Check if all of Xen before relocation is covered by PMR. */
    if ( !is_in_pmr(os_sinit, load_base_addr, xen_size, check_high_pmr) )
        txt_reset(SLAUNCH_ERROR_LO_PMR_MLE);

    /* Check if all of Xen after relocation is covered by PMR. */
    if ( load_base_addr != tgt_base_addr &&
         !is_in_pmr(os_sinit, tgt_base_addr, xen_size, check_high_pmr) )
        txt_reset(SLAUNCH_ERROR_LO_PMR_MLE);

    /*
     * If present, check that MBI is covered by PMR. MBI starts with 'uint32_t
     * total_size'.
     */
    if ( info->boot_params_base != 0 &&
         !is_in_pmr(os_sinit, info->boot_params_base,
                    *(uint32_t *)(uintptr_t)info->boot_params_base,
                    check_high_pmr) )
        txt_reset(SLAUNCH_ERROR_BUFFER_BEYOND_PMR);

    /* Check if TPM event log (if present) is covered by PMR. */
    /*
     * FIXME: currently commented out as GRUB allocates it in a hole between
     * PMR and reserved RAM, due to 2MB resolution of PMR. There are no other
     * easy-to-use DMA protection mechanisms that would allow to protect that
     * part of memory. TPR (TXT DMA Protection Range) gives 1MB resolution, but
     * it still wouldn't be enough.
     *
     * One possible solution would be for GRUB to allocate log at lower address,
     * but this would further increase memory space fragmentation. Another
     * option is to align PMR up instead of down, making PMR cover part of
     * reserved region, but it is unclear what the consequences may be.
     *
     * In tboot this issue was resolved by reserving leftover chunks of memory
     * in e820 and/or UEFI memory map. This is also a valid solution, but would
     * require more changes to GRUB than the ones listed above, as event log is
     * allocated much earlier than PMRs.
     */
    /*
    if ( os_mle->evtlog_addr != 0 && os_mle->evtlog_size != 0 &&
         !is_in_pmr(os_sinit, os_mle->evtlog_addr, os_mle->evtlog_size,
                    check_high_pmr) )
        txt_reset(SLAUNCH_ERROR_BUFFER_BEYOND_PMR);
    */
}

#endif /* __ASSEMBLY__ */
