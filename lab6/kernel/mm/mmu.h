#ifndef MMU_H_
#define MMU_H_

#define TCR_CONFIG_REGION_48bit     (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB              ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT          (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE          0b00000000
#define MAIR_NORMAL_NOCACHE         0b01000100
#define MAIR_IDX_DEVICE_nGnRnE      0
#define MAIR_IDX_NORMAL_NOCACHE     1

#define PAGE_SHIFT                  12                                  // # of bits per page (4KB)
#define TABLE_SHIFT                 9                                   // # of bits per table index 
#define SECTION_SHIFT               (PAGE_SHIFT + TABLE_SHIFT)          // # of bits per section (i.e., # bits of PTE + # bits of page) (2MB)

#define PAGE_SIZE                   (1 << PAGE_SHIFT)
#define PGTABLE_START_ADDR          0x1000
#define SECTION_SIZE                (1 << SECTION_SHIFT)

#define PD_MAIR_DEVICE_nGnRnE       (MAIR_IDX_DEVICE_nGnRnE << 2)       // MAIR index ([4:2])
#define PD_MAIR_NORMAL_NOCACHE      (MAIR_IDX_NORMAL_NOCACHE << 2)
#define PD_TYPE_TABLE               0b11                               // Next level ([1:0])
#define PD_TYPE_BLOCK               0b01
#define PD_TYPE_PAGE                0b11
#define PD_AF                       (1 << 10)                           // Access flag ([10])
#define PD_AP_USER                  (0b01 << 6)                         // Access permission ([7:6])

#ifndef __ASSEMBLER__

#include <stdint.h>

#define VA_BASE                     0xffff000000000000 
#define VA_TO_PA(addr)              ((uint64_t)addr - VA_BASE)
#define PA_TO_VA(addr)              ((uint64_t)addr + VA_BASE)

void mmu_init();

static inline void mmu_switch_to(uint64_t pgd) {
    asm volatile (
        "dsb    ish             \n"     // Ensure write has completed
        "msr    ttbr0_el1, %0   \n"     // Switch translation based address.
        "tlbi   vmalle1is       \n"     // Invalidate all TLB entries
        "dsb    ish             \n"     // Ensure completion of TLB invalidatation
        "isb                    \n"
        :: "r"(pgd)
    );
}

#endif //__ASSEMBLER__

#endif // MMU_H_