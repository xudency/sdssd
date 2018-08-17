/*
 * Copyright (C) 2018-2020 NET-Swift.
 * Initial release: Dengcai Xu <dengcaixu@net-swift.com>
 *
 * ALL RIGHTS RESERVED. These coded instructions and program statements are
 * copyrighted works and confidential proprietary information of NET-Swift Corp.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part.
 *
 * function: 
 * boot block management
 *
 */


// primary page
boot_blk_primary_page *g_primary_page = NULL;

// bbt page
boot_blk_bbt_page *g_bbt_page = NULL;

// u8 ch cp pl lun 
// u16 page, blk


bool flush_primary_page()
{
	return true;
}


bool flush_bbt_page()
{
	return true;
}

// boot block is Ping-Pong design, when one BootLUN write to the last Boot page
// we switch to another, when switch write complete, erase the previous one
bool boot_blk_recycle(u16 bootlun)
{
	u8 ch;
	ppa_t ppalist[CFG_NAND_CH_NUM];

	for_each_ch(ch) {
		// QUAD Plane erase, so ignore plane
		ppalist[ch].nand.lun = bootlun;
		ppalist[ch].nand.ch = ch;		
		ppalist[ch].nand.blk = BOOT_BLK0;
	}
	
	erase_ppa_nowait(ppalist, CFG_NAND_CH_NUM, NAND_MULT_PLANE);
}




