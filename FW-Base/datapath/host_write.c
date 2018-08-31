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

/* each host command need saved, because SPM is read clear*/
host_nvme_cmd_entry *get_host_cmd_entry_from_free_q(void)
{
	struct qnode *node = dequeue(&host_nvme_cmd_free_q);

	if (node) {
		return (host_nvme_cmd_entry *)container_of(node, host_nvme_cmd_entry, next);
	} else {
		return NULL;
	}
}

void saved_to_host_cmd_entry(host_nvme_cmd_entry *entry, hdc_nvme_cmd *cmd)
{
	entry->cmd_tag = cmd->header.tag;
	entry->state = WRITE_FLOW_STATE_INITIAL;
	entry->sqid = cmd->header.sqid;

	entry->vfa = cmd->header.vfa;
	entry->port = cmd->header.port;
	entry->vf = cmd->header.vf;

	entry->sqe = cmd->sqe;
}


setup_phif_cmd_req(phif_cmd_req *req, host_nvme_cmd_entry *host_cmd_entry)
{
	u8 flbas, lbaf_type;
	u16 lba_size;	// in byte
	ctrl_dw12h_t control;
	u64 start_lba = host_cmd_entry->sqe.rw.slba;
	u32 nsid = host_cmd_entry->sqe.rw.nsid;
	struct nvme_lbaf *lbaf;
	control.ctrl = host_cmd_entry->sqe.rw.control;
	struct nvme_id_ns *ns_info;
	host_nvme_cmd_entry *host_cmd_entry;

	// TODO: rate limiter, add this cmd to a pendimg list
	// rate limiter, throttle host write when GC too slowly

	//QW0
	req->header.cnt = 2; 	// phif_cmd_req, the length is fixed on 3 QW
	req->header.dstfifo = MSG_NID_PHIF;
	req->header.dst = MSG_NID_PHIF;
	req->header.prio = 0;
	req->header.msgid = MSGID_PHIF_CMD_REQ;

	// for host cmd, we support max 256 outstanding commands, tag 8 bit is enough
	// EXTAG no used, must keep it as 0, tag copy from hdc_nvme_cmd which is assign by PHIF
	req->header.tag = host_cmd_entry->cmd_tag;
	req->header.ext_tag = PHIF_EXTAG;
	req->header.src = MSG_NID_HDC;
	req->header.vfa = host_cmd_entry->vfa;
	req->header.port = host_cmd_entry->port;
	req->header.vf = host_cmd_entry->vf;
	req->header.sqid = host_cmd_entry->sqid;

	// TODO: Reservation check, if this is reservation conflict
	
	
	// TODO: TCG support, LBA Range, e.g. LBA[0 99] key1,  LBA[100 199] key2 .....
	req->header.hxts_mode = enabled | key; 

	
	ns_info = get_identify_ns(nsid);
	//QW1	
	req->cpa = start_lba / 1;	 // LBA - > CPA

	flbas = ns_info->flbas;
	lbaf_type = flbas & NVME_NS_FLBAS_LBA_MASK;
	lbaf = &ns_info->lbaf[lbaf_type];

	req->hmeta_size = lbaf->ms / 8;
	req->cph_size = lbaf->cphs;

	lba_size = 1 << lbaf->ds;
	if (lba_size == 512) {
		req->lb_size = 0;
	} else if (lba_size == 4096) {
		req->lb_size = 1;
	} else if (lba_size == 16384) {
		req->lb_size = 2;
	} else {
		print_err("LBA Size:%d not support", lba_size);
		//status.bits.sct = NVME_SCT_GENERIC;
		//status.bits.sc = NVME_SC_INVALID_FORMAT;
		//return status;
	}
	
	req->crc_en = 1;

	// PI in last8/first8 / type 0(disable)  1	2  3  
	req->dps = ns_info->dps;

	// DIX or DIF / format LBA size use which (1 of the 16) 
	req->flbas = flbas;

	req->cache_en = !control.bits.fua;

	// TODO:: according frequency, latency, stream to place host data to  HOSTBAND or WLBAND
	req->band_rdtype = HOSTBAND;  // FQMGR, 9 queue/die

	//QW2
	req->elba = (nsid<<32) | start_lba;

	// ask hw to export these define in .hdl or .rdl
}

// statemachine
void host_write_lba(void)
{
	host_nvme_cmd_entry *host_cmd_entry = current_host_cmd_entry;
	
	switch (host_cmd_entry->state) 
	{
		case WRITE_FLOW_STATE_INITIAL:
			phif_cmd_req req;
			setup_phif_cmd_req(req, host_cmd_entry);
			host_cmd_entry->state = WRITE_FLOW_STATE_PHIF_REQ_READY;

		case WRITE_FLOW_STATE_PHIF_REQ_READY:
			if (send_phif_cmd_req(&req)) {
				enqueue_front(host_nvmd_cmd_pend_q, host_cmd_entry->next);
				break;
			} else {
				host_cmd_entry->state = WRITE_FLOW_STATE_PHIF_REQ_SENDOUT;				
			}
			
		case WRITE_FLOW_STATE_PHIF_REQ_SENDOUT:
			// wait phif_cmd_rsp

	}

	return;
}

cqsts handle_nvme_write(hdc_nvme_cmd *cmd)
{
	cqsts status = {0};	// status, default no error
	u64 start_lba = cmd->sqe.rw.slba;
	u16 nlb = cmd->sqe.rw.length;
	u32 nsid = cmd->sqe.rw.nsid;
	host_nvme_cmd_entry *host_cmd_entry;

	if ((start_lba + nlb) > MAX_LBA) {
		print_err("the write LBA Range[%lld--%lld] exceed max_lba:%d", start_lba, start_lba+nlb, MAX_LBA);
		status.bits.sct = NVME_SCT_GENERIC;
		status.bits.sc = NVME_SC_LBA_RANGE;
		return status;	
	}

	if (nsid > MAX_NSID) {
		print_err("NSID:%d is Invalid", nsid);
		status.bits.sct = NVME_SCT_GENERIC;
		status.bits.sc = NVME_SC_INVALID_NS;
		return status;	
	}

	host_cmd_entry = get_host_cmd_entry_from_free_q();
	if (host_cmd_entry == NULL) {
		// it should never, else we should enlarge the gat array
		//goto retry;
	} else {
		// fill it from hdc_nvme_cmd
		saved_to_host_cmd_entry(host_cmd_entry, cmd);
		enqueue(&host_nvmd_cmd_pend_q, host_cmd_entry->next);
	}
	
	if (current_host_cmd_entry == NULL) {
		current_host_cmd_entry = dequeue(&host_nvmd_cmd_pend_q);
	} else {
		// continue process the previous one
	}
	
	host_write_lba();
}

