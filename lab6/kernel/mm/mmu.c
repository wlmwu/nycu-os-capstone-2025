#include "mmu.h"
#include "utils.h"
#include <stdint.h>

void mmu_init() {
    uint64_t *pgd = (uint64_t*)(PGTABLE_START_ADDR + PAGE_SIZE * 0);
    uint64_t *pud = (uint64_t*)(PGTABLE_START_ADDR + PAGE_SIZE * 1);
    uint64_t *pmd = (uint64_t*)(PGTABLE_START_ADDR + PAGE_SIZE * 2);

    memset(pgd, 0, PAGE_SIZE);
    memset(pud, 0, PAGE_SIZE);
    memset(pmd, 0, PAGE_SIZE);

    /*
    Setup Tables: 
    0x00000000 ~ 0x3f000000: Normal
    0x3f000000 ~ 0x40000000: Device
    0x40000000 ~ 0x80000000: Device
    */

    pgd[0] = ((uintptr_t)pud) | PD_NXT_TABLE;                                           // Point to PUD
    
    pud[0] = ((uintptr_t)pmd) | PD_NXT_TABLE;                                           // Point to PMD
    pud[1] = (1 << 30) | PD_ACCESS | PD_MAIR_DEVICE_nGnRnE | PD_NXT_BLOCK;              // Point to 1GB block (Device)
    
    uint64_t total_block = (1 << 30) / SECTION_SIZE;
    uint64_t total_device_block = 0x3F000000 / SECTION_SIZE;
    // Point to 2MB block (Normal)
    for (int i = 0; i < total_block - total_device_block; ++i) {
        pmd[i] = (i * SECTION_SIZE) | PD_ACCESS | PD_MAIR_NORMAL_NOCACHE | PD_NXT_BLOCK;
    }
    // Point to 2MB block (Device)
    for (int i = total_block - total_device_block; i < total_block; ++i) {
        pmd[i] = (i * SECTION_SIZE) | PD_ACCESS |  PD_MAIR_DEVICE_nGnRnE | PD_NXT_BLOCK;
    }

    asm volatile("msr ttbr0_el1, %0" :: "r"((uintptr_t)pgd));
    asm volatile("msr ttbr1_el1, %0" :: "r"((uintptr_t)pgd));
}