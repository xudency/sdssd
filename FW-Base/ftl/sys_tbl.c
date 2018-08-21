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

// test if the 2 ppa belong the same Page
// the same Page has the same PG_TYPE
/*bool is_ppa_in_the_same_page(ppa_t p1, ppa_t p2)
{
	// ignore cp, other is all the same
	return ((p1 >> CP_BITS) == (p2 >> CP_BITS))
}*/

// test if the 2 ppa belong the Die
bool is_ppa_in_the_same_die(ppa_t p1, ppa_t p2)
{
	// blk ch lun, ignore Page
	return ((p1.nand.blk == p2.nand.blk) && \
		    (PPA_TO_DIE(p1) == PPA_TO_DIE(p2)));
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


///////////////////////////////////////////////below is raif page//////////////////////////////////////////////

// when raif not enable, never call this
// return value:
//     0:  NOT raif page
//    -1:  error RAIF MODE config
//	   1:  this is a raif1 page
//	   2:  this is a raif2 page
int is_raif_page(ppa_t ppa)
{
#if (RAIF_MODE == RAIF_DISABLE)
	// no any raif page
	return 0;

#elif ((RAIF_MODE == RAIF1_ENABLE) || (RAIF_MODE == RAIF2_ENABLE))
	u8 number = RAIF_MODE;
	ppa_t res[RAIF_MODE];
	u8 current_die;
	u8 raif1_die = 0xff;
	u8 raif2_die = 0xff;
	
	u8 start_die  = rpu_start_die(ppa);
	u8 end_die = start_die + RAIF_DIES_NUM - 1;
	u8 start_lun = DIE_TO_LUN(start_die);
	u8 start_ch  = DIE_TO_CH(start_die);
	u8 end_lun = DIE_TO_LUN(end_die);
	u8 end_ch  = DIE_TO_CH(end_die);

	get_lastn_good_die_within_range(ppa.nand.blk, start_lun, start_ch, 
									end_lun, end_ch, number, res);

	//if (is_ppa_in_the_same_die(ppa, res[0/1]))
	if (number == 1) {
		// raif1
		raif1_die = PPA_TO_DIE(res[0]);
	} else if (number == 2) {
		// raif 2
		raif2_die = PPA_TO_DIE(res[0]);		
		raif1_die = PPA_TO_DIE(res[1]);		
	} else {
		// NEVER run to here
	}
	
	current_die = PPA_TO_DIE(ppa);
	if (current_die == raif1_die)
		return 1;
	else if (current_die == raif1_die)
		return 2;
	else
		return 0;
#else
	#error "RAIF Mode Invalid"
	return -1;
#endif

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

pg_type get_page_type_fast(ppa_t ppa)
{
	bmi_t *bmi = GET_BMI(ppa.nand.blk);

	u8 type = bmi->pgtype[ppa.nand.lun][ppa.nand.ch];
	
	//bmi->pgtye only identify (normal, badblock, raif1, raif2)
	//to short pgtype size in bmi, we assign the same type for a CPL
	//actually, mostly the same CPL is for the same data type
	//while first page only use 1 cp, ftl log only use PPA_PER_LOG_PAGE
	//so we need do some check and type convert

	// in bmi->type it is mark as NORMAL
	if (is_first_ppa(ppa)) {
		type = FIRST_PAGE;
	}

	// in bmi->type it is mark as NORMAL	
	if (is_ftl_log_page(ppa)) {
		type = FTL_LOG_PAGE;
	}

	// other don't need convert

	return type;
}

int move_die_fwd(u8 lun, u8 ch, u8 n)
{
	// move the die foward to the good die which gap to this die =n
}

//build a pg to type Map table, each entry is the type of a quad plane
//so the map table entry num is (CH X LUN X BLK), each entry is u8
//so the table size is about 94KB, simple to manage, we integrate it in bmi
//with this map table, in wdp, fw can get page type much fastly.
void update_bmi_page_type(ppa_t new_bb)
{
	// when new bb grow, the r-block data distribution will changed
	// if new bb is raif/ftl log, we should move fwd die

	bool r;
	bmi_t *bmi = GET_BMI(new_bb.nand.blk);
	set_page_type(new_bb, BADBLK_PAGE);
	

	// TODO:  FTL_LOG RAIF1/RAIF2 need adjust
	r = is_ppa_in_the_same_die(new_bb, bmi->log_page);
	if (r) {
		move_die_fwd(u8 lun, u8 ch, 1);
		bmi->log_page = ;
	}
	
	r = is_ppa_in_the_same_die(new_bb, bmi->raif1);
	if (r) {

	}

	//is_ftl_log_page(new_bb) {}
		
}

// this ppa is for host data oe insert sys data or badblock
// delete, this is too slow, cpu-consume, 
// because page type not change frequently, very time to re-cal is not worth
/*pg_type get_page_type(ppa_t ppa)
{
	int type;
	
	if (is_bad_block(ppa)) {
		return BADBLK_PAGE;
	}

	if (is_first_ppa(ppa)) {
		return FIRST_PAGE;
	}

	if (is_ftl_log_page(ppa)) {
		return FTL_LOG_PAGE;
	}

	type = is_raif_page(ppa);
	if (type == 0) {
		return NORMAL_PAGE;
	} else if (type == 1) {
		return RAIF1_PAGE;
	} else if (type == 2) {
		return RAIF2_PAGE;
	}
}
*/

pg_mode sys_tbl_get_pg_mode(ppa_t ppa)
{
	// LP UP XP
}


