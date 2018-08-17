////////////////////////////////////////////////////////power down


wpb_roundup(u16 band)
{
	// 1. if slot NOT fillup a page, fillup it

	// 2. if this wpb need program with other WPB(UP/XP), constructed pair WPB
}

// 1) UP XP program Pair
// 2) pad more extra page, e.g. 8, make the MRPG stable
void page_padding(ppa_t ppa, int num)
{
	/*pg_mode mode = sys_get_pg_mode(ppa);
	switch (mode) {
	case LP:
		//fn1();
		break;
	case UP:
		//fn2();
		break;
	case XP:
		//fn3();
		break;
	default:
		BUG();
		break;
	}*/

	// Flush Extra dummy Page to make last program paghe stable
	// 1. close_partial_rpage();
	// 2. flush_dummy_pages(num);

	return;
}


persisit_flight_hdata(u16 padding_num)
{
	//2 Flush User Data in Cache
	flush_cache();

	u8 band;
	// guarantee, the band is stable
	for_each_band(hband)   // Host Band
	{
		
		band_info_t *bandinfo = get_band_info(band);
	
		//2 scan all WPB of each Band, for the partial Page, supply it
		wpb_roundup(band);

		//2 For TLC UP/XP should program together, if current_ppa is UP, pad XP
		page_padding(bandinfo->current_ppa, padding_num);
	}

}

// Flush whole FTL system data?
persisi_flight_sdata(bool whole, u16 padding_num)
{
	u8 sband;

	if (whole) {
		flush_map();
		flush_map_index();
	}

	//2 2.1 Flush RDC table, pos in primary.rdc_index
	flush_rdctbl();

	//2 2.2 Flush BMI table,  pos in primary.bmi_index
	flush_bmitbl();

	//2 2.3 Flush partial R-Page LOG Page,  pos in bandindo.last_log_page
	flush_partial_log_page(log_page_t *buf);

	wpb_roundup(sband);

	page_padding(sbandinfo->current_ppa, padding_num);   // system band 

}


void power_down()
{

//1 0. wait until sys IDLE
	//2 0.1  if free_blk_cnt<THR,  continue do gc	
	gc_arbiter(timeout);
	//recycle_engine();

	//2 0.2 wait until system idle
	//3 1) Host IDLE
	//3 2) GC/WL IDLE
	//3 3) read scrubbing IDLE
	//3 4) CKPT IDLE
	//3 5) Error-Handle IDLE
	request_system_idle();

//1 1.Host Band
	persisit_flight_hdata(CFG_NAND_PADDING_PAGES);

//1 2.System Band
	persisi_flight_sdata(true, CFG_NAND_PADDING_PAGES);

//1 3.Boot Block
	flush_boot_bbt_page();
	g_primary_page->page.power_down_state = POWER_DOWN_STATE_SAFE;
	flush_boot_primary_page();
	
	return;
}

