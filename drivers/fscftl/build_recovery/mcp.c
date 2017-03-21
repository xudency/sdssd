#include "power.h"
#include "../fscftl.h"
#include "../datapath/ppa-ops.h"

// Move to systbl/
void set_bb_tbl(u32 blk, u32 lun, u32 ch)
{
	// TODO , BMI-TBL
	return;
}

void sweepup_disk(struct nvm_exdev *exdev)
{

}

/* in bbt, pl is invisible, i.e. any one pl is bb, we regard all pl is bb 
 * return  0:  goodblock
 * return  1:  badblock
 * return -1:  Error
 */
int micron_flash_bb_eval(struct nvm_exdev *exdev, u32 blk, u32 lun, u32 ch)
{
	int result = 0;
	u32 pl;
	void *databuf;
	void *metabuf;
	u32 offt = CFG_NAND_EP_SIZE - 3 * NAND_RAW_SIZE; 
	struct physical_address ppalist[CFG_NAND_PLANE_NUM];

	memset(ppalist, 0x00, sizeof(ppalist));

	databuf = kzalloc((CFG_NAND_EP_SIZE + NAND_RAW_SIZE) * CFG_NAND_PLANE_NUM, 
						GFP_KERNEL);
	if (databuf == NULL) {
		printk("bbt malloc fail\n");
		return -1;
	}
	
	metabuf = databuf + (CFG_NAND_EP_SIZE * CFG_NAND_PLANE_NUM);
	
	for (pl = 0; pl < CFG_NAND_PLANE_NUM; pl++) {
		ppalist[pl].nand.pl = pl;
		ppalist[pl].nand.blk = blk;
		ppalist[pl].nand.lun = lun;
		ppalist[pl].nand.ch = ch;

		// Micron bbt check (page0, page3)
		ppalist[pl].nand.sec = CFG_NAND_EP_NUM - 1; 			
		ppalist[pl].nand.pg = 0;
	}
	
	nvm_rdpparaw_sync(exdev, ppalist, CFG_NAND_PLANE_NUM, NVM_IO_SNGL_ACCESS, 
					  databuf, metabuf);

	for(pl = 0; pl < CFG_NAND_PLANE_NUM; pl++) {
		u8 *data = (u8 *)((u8 *)databuf + pl*CFG_NAND_EP_SIZE);
		if (*(data + offt) != 0xff) {
			// this is bb
			set_bb_tbl(blk, lun, ch);
			result = 1;
			break;
		} else {
			// check next pl
		}
	}

	kfree(databuf);
	return result;
}

void fscftl_bbt_discovery(struct nvm_exdev *exdev)
{
	u32 blk, lun, ch;
	
	for (blk = 0; blk < CFG_NAND_BLOCK_NUM; blk++) {
		for(lun = 0; lun < CFG_NAND_LUN_NUM; lun++) {			
			for(ch = 0; ch < CFG_NAND_CHANNEL_NUM; ch++) {
				if (!strcmp(FLASH_TYPE, "2TB_MC_L95B")) {
					micron_flash_bb_eval(exdev, blk, lun, ch);
				} else if (!strcmp(FLASH_TYPE, "2TB_TH58TFG9DFK")) {
					//tsb_flash_bb_eval(exdev, blk, lun, ch);
				} else {
					printk("flashtype:%s invalid", FLASH_TYPE);
				}				
			}
		}
	}
}

int do_manufactory_init(struct nvm_exdev * exdev)
{
	printk("start %s\n", __FUNCTION__);

    sweepup_disk(exdev);

    fscftl_bbt_discovery(exdev);  //set bootblk bbt && bmi->bb

    // Now all systbl is clean, don't need Flush down

    bootblk_flush_bbt();
    bootblk_flush_primary_page(POWER_DOWN_UNSAFE);

	printk("complete %s\n", __FUNCTION__);

    return 0;
}
