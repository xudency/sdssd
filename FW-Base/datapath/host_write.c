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

// HDC do some check, then generate a phif_cmd_req fwd to phif.chunk
int host_write_lba(hdc_nvme_cmd *cmd)
{
	u8 fua, access_lat, access_freq;
	dsm_dw13_t dsmgmt;
	ctrl_dw12h_t control;
	u32 start_lba = cmd->sqe.rw.slba;
	u32 nlba = cmd->sqe.rw.length;
	u32 nsid = cmd->sqe.rw.nsid;
	dsmgmt.dw13 = cmd->sqe.rw.dsmgmt;
	control.ctrl = cmd->sqe.rw.control;

	//list_add(, cmd);

	//cmd = list_head

	// cmd sanity check

	// TODO: rate limiter, add this cmd to a pendimg list

	// rate limiter, throttle host write when GC too slowly
	// s_queue_t stack_t ops

	// opcode depend filed fill outside
	phif_cmd_req req;
	memset(&req, 0, sizeof(req));

	//QW0
	req.header.cnt = 2;		// phif_cmd_req, the length is fixed on 3 QW
	req.header.dstfifo = MSG_NID_PHIF;   // dst subdst
	req.header.dst = MSG_NID_PHIF;
	req.header.prio = 0;
	req.header.msgid = MSGID_PHIF_CMD_REQ;

	// for host cmd, we support max 256 outstanding commands, tag 8 bit is enough
	// EXTAG no used, must keep it as 0,   tag = seq.cmdid
	//
	req.header.tag = cmd->header.tag;	//write:0-63     64-256:read
	req.header.ext_tag = PHIF_EXTAG;
	req.header.src = MSG_NID_HDC;   // common
	req.header.vfa = cmd->header.vfa;
	req.header.port = cmd->header.port;
	req.header.vf = cmd->header.vf;
	req.header.sqid = cmd->header.sqid;

	// LBA Range, e.g. LBA[0 99] key1,  LBA[100 199] key2 ....., refer TCG Spec
	req.header.hxts_mode = 0; 
	
	//QW1	
	req.cpa = start_lba / 1;     // LBA - > CPA

	// check namespace data struct, this is generate by namespace format admin command
	// and will retrive to host by identify admin command
	ns_info = get_namespace(nsid);
	req.hmeta_size = ns_info.xx;
	req.cph_size = ns_info.yy;    //read from config
	req.lb_size = ns_info.zz;

	req.crc_en = 1;

	// namespace config info 1.E2E enable/diable  2. PI type ...
	req.dps = mode;  //sqe.prinfo
	req.flbas = mode;

	// FUA is too bypass Cache
	fua = control.bits.fua;
	req.cache_en = !fua;

	// TODO:: according frequency, latency, stream to place host data to  HOSTBAND or WLBAND
	req.band_rdtype = HOSTBAND;  // FQMGR, 9 queue/die

	//QW2
	req.elba = (nsid<<32) | start_lba;

	// ask hw to export these define in .hdl or .rdl

	return send_phif_cmd_req(&req);
}

/*
LBA2CPA(u32 lba, u32 nsid)
{

}
*/
