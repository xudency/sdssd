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
 * host write datapath  wdp
 *
 */


//Assign PPA for each CPAs[scpa: scpa+nppas-1]   nppas is 1-base
//assign more extra ppa to skip bb and log, so that the next io can get normal quickly 
bool atc_assign_ppa(u8 band, u32 scpa, u16 nppas, ppa_t *ppalist)
{
	u8 pg_type;
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
		pg_type = lookup_page_type_fast(current_ppa);

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

// TODO: rate limiter, add this cmd to a pendimg list
// do some check, then generate a phif_cmd_req fwd to phif.chunk
int host_write_lba(hdc_nvme_cmd *cmd)
{
	u32 start_lba = cmd->sqe.rw.slba;
	u32 nlba = cmd->sqe.rw.length;

	//list_add(, cmd);

	//cmd = list_head

	// cmd sanity check

	// rate limiter, throttle host write when GC too slowly
	// s_queue_t stack_t ops

	// opcode depend filed fill outside
	phif_cmd_req req;
	memset(&req, 0, sizeof(req));

	//QW0
	req.header.cnt = 2;
	req.header.dstfifo = ;
	req.header.dst = PHIF_ID;
	req.header.prio = ;
	req.header.msgid = PHIF_CMD_REQ;
	req.header.tag = cmd->header.tag;
	req.header.ext_tag = 0;
	req.header.src = HDC_ID;
	req.header.vfa = 0;
	req.header.port = ;
	req.header.vf = 0;
	req.header.sqid = cmd->header.sqid;
	req.header.hxts_mode = ;
	

	//QW1	
	req.cpa = start_lba / 8;     // LBA - > CPA
	req.hmeta_size = cfg;
	req.cph_size = cfg;    //read from config
	req.lb_size = cfg;
	req.crc_en = 1;
	req.dps = mode;
	req.flbas = mode;
	req.cache_en = 1;
	req.band_rdtype = HOSTBAND;

	//QW2
	req.elba = ;

	// ask hw to export these define in .hdl or .rdl

	send_phif_cmd_req(&req);
}

/*
LBA2CPA(u32 lba, u32 nsid)
{

}
*/
