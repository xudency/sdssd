

// restore MAP base CKPT, Only scan the last flight LOG Pages of each Band
bool ckpt_restore_map()
{
	bool brk;
	time_t timestamp;
	u32 *map_index2 = g_primary_page->page.map_l2_index;

	// load the base MAP table 
	load_map_index(map_index2, MAP_INDEX2_ENTRY_NUM);
	load_map(g_map_index, MAP_INDEX_ENTRY_NUM);
	

	// merge to MAP from LOG Pages in order of timestamp
	// these LOG Pages are commit after last CKPT start
	// flight_log_pages=[start_log, end_log] saved in each band_info
	log_page_t *flight_log_pages[BAND_NUM];

	load_flight_log_pages(flight_log_pages);

	u8 min_ts_band = find_min_timestamp_in_log_page();
	log_page_t *log_page_iter = flight_log_pages[min_ts_band];
	for (; ;) {
		u8 band = log_page_iter.band;
		brk = merge_log2map(log_page_iter, &timestamp);
		if (!brk) {
			// this LOG Page reach to end, let swap next Log Page of this Band
			flight_log_pages[band] = get_next_log_page_mem();  // XXX: list or contious.

			// this band's last LOG Page && last band.  merge last LOG Page completely
			//if (flight_log_pages[band] == NULL && band == g_primary_page->page.recov_end_band)
				//break;
			if (flight_log_pages[band] == NULL)
				set_bit(flag, band);

			if (bits_is_all_set(flag, 0, BAND_NUM-1))
				break;
		}

		log_page_iter = get_timestamp_in_log_page(timestamp, flight_log_pages);
	}

}


bool fast_crash_recovery()
{
	ckpt_restore_map();

	//bmi index -> bmi

	//rdc index -> rdc

	//vpc need sanity check

	//
	
	return 0;
}

// this is very slowlym, it need scan all blk's LOG Pages
// crash recovery in this mode due to
// a. CKPT Disable
// b. fast crash recovery fail
bool slow_crash_recovery()
{
	return 0;
}

bool crash_recovery()
{
	bool ret;
		

	if (ckpt_enable) {
		ret = fast_crash_recovery();
		if (ret == SUCCESS)
			return 0;
	}

	return slow_crash_recovery();
}

