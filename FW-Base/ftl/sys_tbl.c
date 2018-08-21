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

// test if the 2 ppa belong the same Page
// the same Page has the same PG_TYPE
bool is_ppa_in_the_same_page(ppa_t p1, ppa_t p2)
{
	// ignore cp, other is all the same
	return ((p1 >> CP_BITS) == (p2 >> CP_BITS))
}

bool is_ppa_in_the_same_die(ppa_t p1, ppa_t p2)
{
	return ((p1 >> DIE_BITS) == (p2 >> DIE_BITS))
}

bool is_ftl_log_page(ppa_t ppa)
{
	int i;
	bmi_t *bmi = GET_BMI(ppa.nand.blk);
	ppa_t tmp = bmi->log_page;

	if (is_ppa_in_the_same_die(ppa, tmp)) {
		// most likely PPA_PER_LOG_PAGE < PPA_PER_DIE
		u8 cpl = (ppa.all & CPL_MASK);
		if (cpl >= LOG_PAGE_START_CPL)
			return true;
	}

	return false;
}

pg_type sys_tbl_get_pg_type(ppa_t ppa)
{
	// bbt, particular pos
}

pg_mode sys_tbl_get_pg_mode(ppa_t ppa)
{
	// LP UP XP
}


