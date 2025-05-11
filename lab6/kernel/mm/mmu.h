#ifndef MMU_H_
#define MMU_H_

#define TCR_CONFIG_REGION_TSZ       (((64 - 48) << 0) | ((64 - 48) << 16))      // T0SZ, T1SZ:  ttbr size
#define TCR_CONFIG_4KB              ((0b00 << 14) |  (0b10 << 30))              // TG0,  TG1:   ttbr granule size
#define TCR_CONFIG_DEFAULT          (TCR_CONFIG_REGION_TSZ | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE          0b00000000
#define MAIR_NORMAL_NOCACHE         0b01000100
#define MAIR_IDX_DEVICE_nGnRnE      0
#define MAIR_IDX_NORMAL_NOCACHE     1

#define PAGE_SHIFT                  12                                  // Number of bits per page (4KB)
#define TABLE_SHIFT                 9                                   // Number of bits per table index 
#define SECTION_SHIFT               (PAGE_SHIFT + TABLE_SHIFT)          // Number of bits per section (i.e., # bits of PTE + # bits of page) (2MB)

#define PAGE_SIZE                   (1 << PAGE_SHIFT)                   // Size of a page in bytes
#define TABLE_SIZE                  (1 << TABLE_SHIFT)                  // Number of entries per table
#define SECTION_SIZE                (1 << SECTION_SHIFT)                // Size of a section in bytes

#define PAGE_MASK                   (((1ULL << (TABLE_SHIFT * 4 + PAGE_SHIFT)) - 1) & ~(PAGE_SIZE - 1))       // Extract a page ([47:12]) from a table entry 

#define PGTABLE_START_ADDR          0x1000

#define PD_MAIR_DEVICE_nGnRnE       (MAIR_IDX_DEVICE_nGnRnE << 2)       // MAIR index ([4:2])
#define PD_MAIR_NORMAL_NOCACHE      (MAIR_IDX_NORMAL_NOCACHE << 2)
#define PD_TYPE_TABLE               0b11                                // Next level ([1:0])
#define PD_TYPE_BLOCK               0b01
#define PD_TYPE_PAGE                0b11
#define PD_AF                       (1 << 10)                           // Access flag ([10])
#define PD_AP_RW_EL0                (0b01 << 6)                         // Access permission ([7:6]) (ARMv8-A Address Translation, page 22)
#define PD_AP_RO_EL0                (0b11 << 6)
#define PD_AP_NONE_EL0              (0b00 << 6)
#define PD_UXN                      (1ULL << 54)                        // Unprivileged (EL0) Execute Never

#ifndef __ASSEMBLER__

#include <stdint.h>

#define VA_BASE                     0xffff000000000000 
#define VA_TO_PA(addr)              ((uint64_t)addr - VA_BASE)
#define PA_TO_VA(addr)              ((uint64_t)addr + VA_BASE)

void mmu_init();

static inline void tlb_flush_all() {
    asm volatile ("tlbi vmalle1is" ::: "memory");       // Invalidate all TLB entries
    asm volatile ("dsb  ish      " ::: "memory");       // Ensure the completion of TLB invalidatation
    asm volatile ("isb           " ::: "memory");       // Ensure that the processor fetches new instructions
}

static inline void tlb_flush_page(uint64_t va) {
    asm volatile ("dsb  ishst      " ::: "memory");                 // The barrier only waits for store ("st")
    asm volatile ("tlbi	vale1is, %0" ::  "r" (VA_TO_PA(va)));       // Invalidate all TLB entries
    asm volatile ("dsb  ish        " ::: "memory");                 // Ensure the completion of TLB invalidatation
    asm volatile ("isb             " ::: "memory");                 // Ensure that the processor fetches new instructions
}

static inline void mmu_switch_to(uint64_t pgd) {
    asm volatile (
        "dsb    ish             \n"     // Ensure write has completed
        "msr    ttbr0_el1, %0   \n"     // Switch translation based address.
        :: "r" (pgd)
    );
    tlb_flush_all();
}

#endif //__ASSEMBLER__

#endif // MMU_H_