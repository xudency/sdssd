/*
 * Copyright (C) 2017 Group XX
 * Initial release: 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * fscftl initialization.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

#include "hwcfg/cfg/flash_cfg.h"
#include "../nvme/host/nvme.h"
#include "fscftl.h"

static char exdev_name[8] = "nvme0";
module_param_string(devname, exdev_name, 8, 0);

void set_bb_tbl(u32 blk, u32 lun, u32 ch)
{
	// TODO , BMI-TBL
	return;
}

int rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
                      u16 nppa, u16 ctrl, void *databuf, void *metabuf)
{
	struct nvme_ppa_command ppa_cmd;
	struct request_queue *q = exdev->ctrl->admin_q;		// How get from

	ppa_cmd.opcode = NVM_OP_RDRAW;
	ppa_cmd.nsid = 1;
	ppa_cmd.metadata = cpu_to_le64(metabuf);  			// DMA-Address
	ppa_cmd.ppalist = cpu_to_le64(ppa);    	  			// DMA-Address
	ppa_cmd.nlb = cpu_to_le16(nppa - 1);

	nvme_submit_sync_cmd(q, (struct nvme_command *)&ppa_cmd,
						 databuf, nppa * CFG_NAND_EP_SIZE);

	return 0;
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
	
	rdpparaw_sync(exdev, ppalist, CFG_NAND_PLANE_NUM, NVME_PLANE_SNGL, databuf, metabuf);

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

static int __init fscftl_module_init(void)
{
	u32 regval;
    struct nvm_exdev *exdev;
	struct nvme_ctrl *ctrl;

	printk("NandFlash type:%s\n", FLASH_TYPE);
	printk("goto find expose nvme device:%s\n", exdev_name);

	exdev = nvm_find_exdev(exdev_name);
    if (exdev == NULL)
        return -ENODEV;

	printk("find exdev:%s  magic_dw:0x%x\n", exdev->name, exdev->magic_dw);

	ctrl = exdev->ctrl;
	ctrl->ops->reg_read32(ctrl, 0x00, &regval);

	printk("regval:0x%x\n", regval);

	nvm_create_exns(exdev);

    //fscftl_bbt_discovery(exdev);

	return 0;
}

static void __exit fscftl_module_exit(void)
{
	struct nvm_exdev *exdev = nvm_find_exdev(exdev_name);

	nvm_delete_exns(exdev);

	return;
}

module_init(fscftl_module_init);
module_exit(fscftl_module_exit);
MODULE_AUTHOR("Dengcai Xu <dxu@cnexlabs.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Host-Based Full Stack Control FTL for NVMe SSDs");

