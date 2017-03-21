#ifndef __WCB_MNGR_
#define __WCB_MNGR_
#include "../hwcfg/cfg/flash_cfg.h"

// Write Cache Size=16*4*4*8*10(4KB) = 80MB
#define PADDING_PAGE_NUM  8
#define READ_CB_UNITS  	  PADDING_PAGE_NUM
#define WRITE_CB_UNITS 	  2
#define TOTAL_CB_UNITS	(WRITE_CB_UNITS + READ_CB_UNITS)
#define RAID_LUN_SEC_NUM (CFG_NAND_CHANNEL_NUM * CFG_NAND_EP_NUM * CFG_NAND_PLANE_NUM)
#define RAID_PAGE_SEC_NUM (RAID_LUN_SEC_NUM * CFG_NAND_LUN_NUM)
#define WRITE_CACHE_SEC_NUM (RAID_PAGE_SEC_NUM * TOTAL_CB_UNITS)

int write_cache_alloc(struct nvm_exdev *exdev);
int write_cache_free(struct nvm_exdev *exdev);

#endif

