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
 * system band data define and function
 *
 */

 #include "sys_tbl.h"


///////////////////////////////////////////////XXXXXXXXXXXXX//////////////////////////////////////////////

void __set_page_type(u16 blk, u8 lun, u8 ch, u8 type)
{
	bmi_t *bmi = GET_BMI(blk);
	bmi->pgtype[lun][ch] = type;
}

void set_page_type(ppa_t ppa, u8 type)
{
	__set_page_type(ppa.nand.blk, ppa.nand.lun, ppa.nand.ch, type);
}

u8 __get_page_type(u16 blk, u8 lun, u8 ch)
{
	bmi_t *bmi = GET_BMI(blk);
	return bmi->pgtype[lun][ch];
}

u8 get_page_type(ppa_t ppa)
{
	return __get_page_type(ppa.nand.blk, ppa.nand.lun, ppa.nand.ch);
}

//lookup table and do some simple convert, it is fast
//to short pgtype size in bmi, we assign the same type for a CPL
//actually, mostly the same CPL is for the same data type
//while first page only use 1 cp, ftl log only use PPA_PER_LOG_PAGE
//so there are 2 case need convert after lookup table 
//    case 1.normal in first page -> first page
//	  case 2.ftl log page in front part CPL -> normal
u8 lookup_page_type_fast(ppa_t ppa)
{
	u8 type = get_page_type(ppa);

	// case1 convert
	if (type == NORMAL_PAGE) {
		if (is_first_ppa(ppa)) 
			type = FIRST_PAGE;
	}

	// case2 convert
	if (type == FTL_LOG_PAGE) {
		u8 cpl = (ppa.all & CPL_MASK);
		if (cpl >= LOG_PAGE_START_CPL)
			type = NORMAL_PAGE;
	}

	return type;
}

//build a page to type Map table, each entry is the type of a quad plane
//so the map table entry num is (CH X LUN X BLK), each entry is u8
//so the table size is about 94KB, simple to manage, we integrate it in bmi
//with maintain this map table, fw can lookup it and get page type much fastly.
// TODO: moce this to MCP.c
void sys_build_page_type(void)
{
	u16 blk;
	u8 lun, ch, page_type;
	bool rc;

	for_each_blk(blk) {
		for_each_lun(lun) {
			for_each_ch(ch) {
				//set page type according bbt-page
				rc = __get_bbt(blk, lun, ch);
				page_type = rc ? BADBLK_PAGE:NORMAL_PAGE;
				__set_page_type(blk, lun, ch, page_type);
			}
		}
		
		block_dist_recalibrate(blk);
	}
}

// when new bb grow, the r-block data distribution will changed
// if new bb is raif/ftl log, we should move fwd die
// this fn should called after bbt page has updated
void block_dist_recalibrate(u16 blk)
{	
	//set_page_type(new_bb, BADBLK_PAGE);   // called together with set_bbt fn

	raif_die_recalibrate(blk);

	ftl_log_page_die_recalibrate(blk);
	
	return;
}





