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

extern struct bootblk_bbt_page *boot_bbt_page_info;
extern struct bootblk_meta_page *boot_meta_page_info;

struct bootblk_bbt_page {
	u32 magic_dw[NUM_OF_MW_DWORDS];
	u16 bbt[CFG_NAND_BLOCK_NUM][CFG_NAND_LUN_NUM];
};

struct bootblk_meta_page {
	u32 magic_dw[NUM_OF_MW_DWORDS];
	enum power_down_flag pdf;
	geo_ppa bbt_page_address;	// current address, ch page
	geo_ppa meta_page_address;
	u8 bbt_ch_iter[4];
	u8 meta_ch_iter[4];
};

static inline geo_ppa bootblk_get_bbt_pos(void)
{
	return boot_meta_page_info->bbt_page_address;
}

static inline geo_ppa bootblk_get_meta_pos(void)
{
	return boot_meta_page_info->meta_page_address;
}

int bootblk_page_init(void);
void bootblk_page_exit(void);

int bootblk_recovery_meta_page(struct nvm_exdev *dev);
int bootblk_recovery_bbt_page(struct nvm_exdev *dev);
void bootblk_flush_meta_page(struct nvm_exdev *dev, enum power_down_flag flag);
void bootblk_flush_bbt_page(struct nvm_exdev *dev);

#endif
