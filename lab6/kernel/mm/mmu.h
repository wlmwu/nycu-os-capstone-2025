#ifndef MMU_H_
#define MMU_H_

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define PAGE_SIZE 4096
#define PGTABLE_START_ADDR 0x1000
#define SECTION_SIZE (1 << 21)      // 2MB

#define PD_MAIR_DEVICE_nGnRnE (MAIR_IDX_DEVICE_nGnRnE << 2)         // MAIR index ([4:2])
#define PD_MAIR_NORMAL_NOCACHE (MAIR_IDX_NORMAL_NOCACHE << 2)
#define PD_NXT_TABLE 0b11                                           // Next level ([1:0])
#define PD_NXT_BLOCK 0b01
#define PD_ACCESS (1 << 10)                                         // Access flag ([10])

#endif // MMU_H_