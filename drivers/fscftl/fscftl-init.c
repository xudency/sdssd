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

#include "linux/lightnvm.h"
#include "hwcfg/cfg/flash_cfg.h"
#include "../nvme/host/nvme.h"

static char exdev_name[8] = "nvme0";
module_param_string(name, exdev_name, sizeof(exdev_name), S_IRWXUGO);


// Move to fscftl.h
enum {
	NVME_PLANE_SNGL	= 0,
	NVME_PLANE_DUAL	= 1,
	NVME_PLANE_QUAD	= 2,
};

enum {
	NVM_OP_WRPPA		= 0x91,
	NVM_OP_RDPPA		= 0x92,
	NVM_OP_ESPPA		= 0x90,
	NVM_OP_WRRAW		= 0x95,
	NVM_OP_RDRAW		= 0x96,
};

struct nvme_ppa_command {
	__u8			opcode;
	__u8			flags;
	__u16			command_id;
	__le32			nsid;
	__u64			rsvd2;
	__le64			metadata;
	__le64			prp1;
	__le64			prp2;
	__le64			ppalist;
	__le16			nlb;
	__le16			control;
	__le32			dsmgmt;
	__le64			resv;
};

// TODO don't define fix value, should read from HW register
#define NAND_RAW_SIZE 		304
#define NAND_META_SIZE 		16


void set_bb_tbl(u32 blk, u32 lun, u32 ch)
{
	// TODO , BMI-TBL
	return;
}

int rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
                      u16 nppa, u16 ctrl, void *databuf, void *metabuf)
{
	struct nvme_ppa_command ppa_cmd;
	struct request_queue *q;		// How get from

	ppa_cmd.opcode = NVM_OP_RDRAW;
	ppa_cmd.nsid = 1;
	ppa_cmd.metadata = cpu_to_le64(metabuf);  // DMA-Address
	ppa_cmd.ppalist = cpu_to_le64(ppa);    	  // DMA-Address
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
	printk("NandFlash type: %s\n", FLASH_TYPE);

    struct nvm_exdev *exdev;
   
    exdev = nvm_find_exdev(exdev_name);
    if (exdev == NULL)
        return -ENODEV;

    fscftl_bbt_discovery(exdevd);

	return 0;
}

static void __exit fscftl_module_exit(void)
{
	return;
}

module_init(fscftl_module_init);
module_exit(fscftl_module_exit);
MODULE_AUTHOR("Dengcai Xu <dxu@cnexlabs.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Host-Based Full Stack Control FTL for NVMe SSDs");

