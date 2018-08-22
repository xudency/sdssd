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

// each band has 1 first_page in memory, the sizeis 4KB/Band 
// this is not access frequently, reside in DDR
first_page_t *g_first_page[BAND_NUM] = {NULL};

// ftl log page in Write Data Path
// each band has 2 ftl_log_page in memory(Ping-Pong switch), 36KB*2/band
log_page_t *g_wdp_ftl_log[BAND_NUM][LOG_PAGES_PER_BAND] = {NULL};

// a fix pattern data for padding, all 0x0000, assign a special 4KB for it 
void *dummy_date_buff = NULL;

// RAIF is implement by HW, the result is saved in a fixed address in CBUFF, 
// the address is depend on HW design, for fw, we only need assign the base address 
void *raif1_buff = NULL;
void *raif2_buff = NULL;

page_type

///////////////////////////////////////////////below is first page //////////////////////////////////////////////
void first_page_buff_init(void)
{
	u8 band;

	for_each_band(band) {
		g_first_page[band] = kmalloc(FIRST_PAGE_SIZE, GFP_KERNEL);

		memset(g_first_page[band], 0x00, FIRST_PAGE_SIZE);
	}

}

void first_page_buff_exit(void)
{
	u8 band;
	
	for_each_band(band) {
		if (g_first_page[band])
			kfree(g_first_page[band]);
	}
}

bool is_first_ppa(ppa_t ppa)
{
	return ((ppa.nand.pg == 0) && (ppa.nand.lun == 0) && (ppa.nand.pl == 0) && (ppa.nand.cp == 0))
}


// when this R-Block Open, init it from bmi and bbt page.
void update_first_page(u16 blk, u8 band)
{
	assert(band < BAND_NUM);
	assert(blk < CFG_NAND_BLK_NUM);

	u8 lun;
	u16 *bbt;
    first_page_t *firstpage = GET_FIRST_PAGE(band);
    bmi_t* bmi = GET_BMI(blk);

	// beware the first-page stuff is the previos Closed R-Block
	// we need init it ahead, then to filled with the new open R-Block.
	// XXX
	first_page_reinit(band);
	
	first_page->blk = blk;
	first_page->band = band;
	first_page->cri = bmi->cri;
	first_page->sequence = ;
	first_page->timestamp = current_timestamp();
	first_page->pecycle = bmi->pecycle;
	first_page->bb_cnt = bmi->bb_cnt;

	// bad block from bbt-page
	bbt = get_blk_bbt_base(blk);
	for_each_lun(lun) {
		//first_page->bbt[lun] = (bbt + lun);
		first_page->bbt[lun] = bbt[lun];
	}

    return;
}

/////////////////////////////////////////////below is ftl log page////////////////////////////////////////////

void ftl_log_page_buff_init(void)
{
	u8 band;
	log_page_t *lp;

	
	for_each_band(band) {
		lp = kmalloc(LOG_PAGE_SIZE*LOG_PAGES_PER_BAND, GFP_KERNEL);
		memset(lp, 0xffff, LOG_PAGE_SIZE*LOG_PAGES_PER_BAND);
		g_wdp_ftl_log[band][0] = lp;
		g_wdp_ftl_log[band][1] = (log_page_t *)((void *)lp + LOG_PAGE_SIZE);
	}
}

void ftl_log_page_buff_exit(void)
{
	u8 band;
	
	for_each_band(band) {
		if (g_wdp_ftl_log[band][0]) {
			kfree(g_wdp_ftl_log[band][0]);
		}
	}
}

// TODO: gc_ftl_log_buff


/* calculate the offset(index) in 36KB LOG Page
 * a LOG Page contian a R-Page's cp_log_t, get the specified CP's cp_log_t offset
 */
u16 calc_ftl_log_cp_index(ppa_t ppa)
{
	//g_wdp_ftl_log[band][];
	// ignore blk, pg

	return ppa.nand.cp + \
		   ppa.nand.pl*CFG_NAND_CP_NUM + \
		   ppa.nand.ch*(CFG_NAND_CP_NUM*CFG_NAND_PL_NUM) + \
		   ppa.nand.lun(CFG_NAND_CP_NUM*CFG_NAND_PL_NUM*CFG_NAND_CH_NUM);
	
}

// when the ppa is assigned by ppa allocatoe in WDP, record log in log page from WPB
void wdp_update_ftl_log(u8 band, ppa_t ppa, u32 cpa)
{
	u16 idx = calc_ftl_log_cp_index(ppa);

	// TODO: which half
	log_page_t *lp; // = g_wdp_ftl_log[band][0/1];

	lp->cpats[idx].cpa = cpa;
	lp->cpats[idx].timestamp = current_timestamp();

}

/*bool is_ftl_log_page(ppa_t ppa)
{
	u8 cpl = (ppa.all & CPL_MASK);
	if (cpl >= LOG_PAGE_START_CPL)
		return true;

	return false;
}*/

// ftl log page move forward
//   1.log page is wort-out to be BADBLK, fwd to next good
//   2.RAIF1 worn-out it occupy log page, so we need fwd to next good closed raif1
//   3.RAIF2 worn-out it occupy raif1, so raif1 fwd occupy log page
//   4.RAIF2 and RAIF1 all worn-out
// anyway, Log Page is Close the last RPU's Raif1, so after raif die has recalibrate
// call this fn to recalibrate Log Page die
void ftl_log_page_die_recalibrate(u16 blk)
{
	u8 dies[RAIF_MODE+1];
	// the last RPU Die range
	u8 start_die = CFG_NAND_DIE_NUM-RPU_DIES_NUM_CFG;
	u8 end_die = start_die + RPU_DIES_NUM_CFG - 1;
	u8 start_lun = DIE_TO_LUN(start_die);
	u8 start_ch  = DIE_TO_CH(start_die);
	u8 end_lun = DIE_TO_LUN(end_die);
	u8 end_ch	= DIE_TO_CH(end_die);
	u8 ch, lun;

	//bmi_t *bmi = GET_BMI(blk);

	// ftl log page is closed raif1(if it is enable)
	get_lastn_good_die_within_range(blk, start_lun, start_ch, end_lun, end_ch, RAIF_MODE+1, dies);
	
	lun = DIE_TO_LUN(dies[RAIF_MODE]);
	ch = DIE_TO_CH(dies[RAIF_MODE]);
	__set_page_type(blk, lun, ch, FTL_LOG_PAGE);

	return;
}


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
sys_build_page_type()
{
	// only in MCP
	for_each_blk(blk) {
		for_each_lun(lun) {
			for_each_ch(ch) {
				//set page type according bbt-page
			}
		}
	}
}

// when new bb grow, the r-block data distribution will changed
// if new bb is raif/ftl log, we should move fwd die
void block_dist_recalibrate(ppa_t new_bb)
{
	u16 blk = new_bb.nand.blk;

	set_page_type(new_bb, BADBLK_PAGE);

	raif_die_recalibrate(blk);

	ftl_log_page_die_recalibrate(blk)
	
	return;
}

/*pg_mode sys_tbl_get_pg_mode(ppa_t ppa)
{
	// LP UP XP
}*/


