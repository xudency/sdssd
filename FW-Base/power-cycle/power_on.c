


// rebuild MAP bmi vpc etc.... through boot_page_primary_info
// if failed goto do crasg recovery 
bool power_on()
{
	u32 *map_index2 = g_primary_page->page.map_l2_index;

	//index2 -> MAP index
	load_map_index(map_index2, MAP_INDEX2_ENTRY_NUM);

	//MAP index -> MAP	
	load_map(g_map_index, MAP_INDEX_ENTRY_NUM);

	//rdc index -> rdc
	load_rdctbl();

	//bmi index -> bmi
	load_bmitbl();

	u16 i;
	u16 blk;
	bmi_item_t *bmi;
	for (i=0; i<2; i++ {
		bmi = g_primary_page->page.bmi[i];
		blk = bmi->blk;
		memcpy(g_bmi_tbl[blk], bmi, sizeof(bmi_item_t));
	}

	//each band partial LOG Pages restore, it's safe power down
	// bandinfo.first_log_page=bandinfo.last_log_page
	// this LOG page is in sband rather than in hband particular PPA
	for_each_band() {
		load_log_page(bandinfo.last_log_page);
	}

	//each band current_fpa

	return 0;
}

