#ifndef _BOOTBLK_MNGR_
#define _BOOTBLK_MNGR_

#include "../build_recovery/power.h"
#include "../hwcfg/cfg/flash_cfg.h"

#define NUM_OF_MW_DWORDS 4

#define BOOTBLK_META_MDW0 0x6E657875
#define BOOTBLK_META_MDW1 0x73707269
#define BOOTBLK_META_MDW2 0x6d617279
#define BOOTBLK_META_MDW3 0X626F6F74

#define BOOTBLK_BBT_MDW0 0x6261646c
#define BOOTBLK_BBT_MDW1 0x6c6f636b
#define BOOTBLK_BBT_MDW2 0x73626974
#define BOOTBLK_BBT_MDW3 0x6d617073

// here the PAGE metapage bbtpage means all EP*PL i.e. 32KB/64KB
#define BOOTBLK_PORTION_SIZE (CFG_NAND_EP_SIZE*CFG_DRIVE_LINE_NUM)

struct bootblk_bbt_page {
	u32 magic_dw[NUM_OF_MW_DWORDS];
	u16 bbt[CFG_NAND_BLOCK_NUM][CFG_NAND_LUN_NUM];
};

struct bootblk_meta_page {
	u32 magic_dw[NUM_OF_MW_DWORDS];
};

int bootblk_page_init(void);
void bootblk_page_exit(void);

int bootblk_recovery_meta_page(void);
void bootblk_flush_meta_page(enum power_down_flag flag);
void bootblk_flush_bbt(void);
void bootblk_recovery_bbt(void);

#endif
