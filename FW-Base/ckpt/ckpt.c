// TODO:
// FIXME:
// XXX:
// CAUT: 

////////////////////////////////////////////// global variable

ckpt_cb *g_ckpt_ctx;

// DDR
ppa_t *g_map_tbl;
ppa_t *g_map_index;



//////////////////////////////////////////////////////////////////////////
// PPA Interface
// Read/Write the Physical Page Address
//////////////////////////////////////////////////////////////////////////

// num is 0-base
int submit_rdppa_async(ppa_t *ppalist, int num, void *buffer)
{

}

int submit_wrppa_async(ppa_t *ppalist, int num, void* buffer)
{

}


//////////////////////////////////////////////////////////////////////////
// CPA Interface
// Read/Write the Logical Page Address
//////////////////////////////////////////////////////////////////////////

// For system data, we assign a extend CPA for each 4Ksegment
// via these interface, to load/flush MAP MAP_index BMI etc table
write_cpa()
{

}

read_cpa()
{

}

//////////////////////////////////////////////////////////////////////////
// Slot Interface
// write the data-slot(in DDR) to NAND
//////////////////////////////////////////////////////////////////////////

//
flush_slot(u32 slot)
{

}

// flush all in flight data in Cache
flush_cache()
{

}


flush_map_index()
{

}

flush_map()
{

}

//bandinfo->current_log_page
flush_partial_log_page(log_page_t *buf)
{
	//flush_slot();
}


flush_bmitbl();
{
	// g_bmi_tbl
	//flush_slot();
}


flush_rdctbl();
{
	// g_rdc_tbl
	//flush_slot();
}


// load MAP Index table from NAND according MAP index2
load_map_index(u32 *map_index2, u32 num)
{

}

// Load MAP table from NAND according MAP index
load_map()
{

}

load_log_page()
{

}


load_band_flight_log_pages(u8 band, log_page_t *buffer)
{
	band_info_t bandinfo = g_primary_page->page.bandinfo[band];
	ppa_t logppa;

	/*for (logppa=bandinfo.first_log_page; logppa <= bandinfo.last_log_page) {
		logppa.nand.pg+2;

		if (blk switch) {
			blk is NOT ++, get from bmi->next
		}
	}*/
	
}


load_flight_log_pages(log_page_t **flight_log_pages)
{
	int i;

	for (i = 0; i < BAND_NUM; i++) {
		load_band_flight_log_pages(i, flight_log_pages[i]);
	}
}

// merge log page to MAP in order of timestamp, from log_page.pos
//     1) if find timestamp is discontinuity in this log page,
// 		  halt merge in this Log Page set pos++ for next round merge, return 1
// 	   2) if this LOG page is continuity until to end, return 0
// when switch band, timestamp settd, it is the next hope to merge.
bool merge_log2map(log_page_t *log_page, time_t *timestamp)
{
	//log_page->cpats[log_page->pos]
	//switch ()
	//	1.  0-MAX_CPA merge to g_map_tbl[log_page->cpats[log_page->pos].cpa]
	//  2.  BADB: skip to next
	//  3.  RAIF: skip to next
	//	4.  Pad: not Gather Multi-Plane, this is all 000, skip to next

	log_page->pos;   // step herein
}

u8 find_min_timestamp_in_log_page()
{
	return band;
}

log_page_t *get_timestamp_in_log_page(time_t timestamp, log_page_t **log_pages)
{
#if 0
	int i;

	for (i = 0; i < BAND_NUM; i++) {
		if (log_pages[i].cpats[log_pages.pos].timestamp == timestamp)
			return log_pages[i];
	}

	return NULL;   // NEVER
#else
	find_min_timestamp_in_log_page();
#endif
}

bool ftl_system_restore()
{
	bool ret;

	//1. search primary page in boot block and read it to sram point by g_primary_page
	ret = get_boot_primary_page();
	POWER_DOWN_STATE state = g_primary_page->page.power_down_state;

	switch (state) {
	case POWER_DOWN_STATE_SAFE:
		ret = power_on();
		if (ret == 0) {
			// safe power down last cycle, and power on success
			break;
		}
				
	case POWER_DOWN_STATE_PFAIL:
		ret = crash_recovery();
		break;
	}
	
	return ret;
}


bool ftl_system_save(POWER_DOWN_STATE state)
{
	
}


///////////////////////////////////////////////////////////////////////////////////////////
///// Apply CKPT
///////////////////////////////////////////////////////////////////////////////////////////

ppa_t get_prev_log_page_ppa(u8 band, ppa_t ppa)
{
	if (ppa.nand.pg < 2) {
		ppa.nand.bl = g_bmi_tbl[ppa.nand.bl].prev;
		ppa.nand.pg = PG_NUM-1;
	} else {
		ppa.nand.pg -= 2;
	}

	// ep=0  pl=0  ch=12  lun=15 

	return ppa;
}


ppa_t get_next_log_page_ppa(u8 band, ppa_t ppa)
{
	if (ppa.nand.pg > PG_NUM-3) {
		ppa.nand.bl = g_bmi_tbl[ppa.nand.bl].next;
		ppa.nand.pg = 2;
	} else {
		ppa.nand.pg += 2;
	}
}


// recovery need Scan-Merge LOG Pages window fwd
void ckpt_start_log_page_anchor()
{
	int i;
	ppa_t ppa;
	band_info_t *bandinfo;

	for (i=0; i < BAND_NUM; i++) {
		bandinfo = get_band_info(i);

		// CAUT: each band current_ppa is in partial Log-Page
		bandinfo->anchor_log_page.log_ppa = get_prev_log_page_ppa(i, bandinfo->current_ppa);
		bandinfo->anchor_log_page.pos = 0; 

		//bandinfo->current_log_page.ppa;
		//bandinfo->current_log_page.pos;
		//bandinfo->first_log_page = KEEP;  // the init value is gived in power-on, the partial log page ppa
	}
}

// when this round CKPT Done, forward the recovery Scan-Merge LOG Pages window
void ckpt_end_log_page_fwd()
{
	int i;
	band_info_t *bandinfo;

	for (i=0; i < BAND_NUM; i++) {
		bandinfo = get_band_info(i);

		// some band may not flush any LOG Pages during 2 CKPT
		// when Power-fail occur.   flush the LOG Page at bandinfo.last_log_page.ppa
		// here, last_log_page.ppa < first_log_page(it suppose in end of R2-Page)
		// set last = first, only need Scan-Merge one Log Pages due to this band not fwd.
	
		bandinfo->base_log_page.log_ppa = bandinfo->anchor_log_page.log_ppa;
		//bandinfo->base_log_page.log_ppa = get_next_log_page_ppa(i, bandinfo->anchor_log_page.log_ppa);
		bandinfo->base_log_page.pos = 0;

		bandinfo->anchor_log_page.log_ppa = 0xffffeeee;
	}
}


// statemachine
void apply_ckpt()
{

start:
	printf("CKPT commit Start\n");

	//g_ckpt_ctx->log_pages_num = 0;
	//g_ckpt_ctx->ongoing = true;

	// prepare the dirty segment
	u64 *temp = g_ckpt_ctx->psgmt_ckpt_lock;
	g_ckpt_ctx->psgmt_ckpt_lock = g_ckpt_ctx->psgmt_path_mutate;
	g_ckpt_ctx->psgmt_path_mutate = temp;

	// it will reset when this dirty sgmt flushed, so mutate MUST all 0000 here
	//memset(&g_ckpt_ctx->psgmt_path_mutate, 0x00, );

	ckpt_start_log_page_anchor();
	
	
flush_ckpt_data:
	u32 i;
	void *map_sgmt_buffer;
	for_each_set_bit(i, g_ckpt_ctx->psgmt_ckpt_lock, MAP_SGMT_NUM) {
		map_sgmt_buffer = (void *)(g_map_tbl + i*1024);
		flush_ep_slice(map_sgmt_buffer);
		clear_bit(i, g_ckpt_ctx->psgmt_ckpt_lock);    // so next swap, it don't need init
	}

	// CAUT: here now dirty MAP segment flush cmd has sendout, MAP index has updated
	flush_map_index();

complete:
	// when all CKPT command complete
	g_ckpt_ctx->ongoing = false;
	ckpt_end_log_page_fwd();
	printf("CKPT commit Done\n");
	
}


// this is ONLY called in Host write datapath.
bool ckpt_monitor()
{
	g_ckpt_ctx->log_pages_num++;
	

	if (g_ckpt_ctx->ongoing) {
		// if log_pages_num > 128, it indicate CKPT is too slow, need accerate
		DEBUG("log_pages_num:%d", g_ckpt_ctx->log_pages_num);
	}
	

	if ((!g_ckpt_ctx->ongoing) && (g_ckpt_ctx->log_pages_num >= 128)) {

		g_ckpt_ctx->ongoing = true;
		g_ckpt_ctx->log_pages_num = 0;
	
		apply_ckpt();
		return 1;
	} else {
		return 0;
	}
}


/* Assign FPA for each CPAs[scpa: scpa+nppas-1]   nppas is 1-base
 * assign more extra ppa to skip bb and log, so that the next io can get normal quickly 
 */
bool atc_assign_fpa(u8 band, u32 scpa, u16 nppas, ppa_t *ppalist)
{
	pg_type;
	u16 idx = 0;
	u32 cpa = scpa;
	bool flush_log_page = false;
	bool flush_raif1 = false;
	bool flush_raif2 = false;

	band_info_t *bandinfo = get_band_info(band);
	//ASSERT(bandinfo)

	// assign FPA from current_ppa
	ppa_t current_ppa = bandinfo->current_ppa;

	while(1) {
		pg_type = get_page_type(current_ppa);

		if ((pg_type == NORMAL_PAGE) && (idx >= nppas))
			break;   // guarantee the fpa next kick in is Normal
	
		switch (pg_type) {
		case NORMAL_PAGE:
			ppalist[idx] = current_ppa;
			cpa = scpa + idx;
			idx++;
			break;
		case BADBLK_PAGE:
			cpa = SPECIAL_CPA_INVALID;
			break;
		case FTL_LOG_PAGE:
			cpa = SPECIAL_CPA_FTLLOG;
			flush_log_page = true;
			break;
		case RAIF1_PAGE:
			cpa = SPECIAL_CPA_RAIF1;
			break;
		case RAIF2_PAGE:
			cpa = SPECIAL_CPA_RAIF2;			
			break;
		}


		// LOG Page is offset by fpa:
		// write the cpa and timestamp, CPH
		update_log_page();

		// special Page Process
		if (flush_log_page) {
			flush_log_page = false;
			// flush LOG Page to Host Band
			commit_log_page();

			// if CKPT Enable
			ckpt_monitor();		
		}

		if (flush_raif1) {
			flush_raif1 = false;
			commit_raif1();
		}

		if (flush_raif2) {
			flush_raif2 = false;
			commit_raif2();
		}

		//XXX: ep pl ch lun pg blk, when  switch blk, get from free_rbtree
		increase_ppa(current_ppa);
	}

	bandinfo->current_ppa = current_ppa;

	return 0;
}


