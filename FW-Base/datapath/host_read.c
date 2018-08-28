


int host_read_lba(hdc_nvme_cmd *cmd)
{
	u32 start_lba = cmd->sqe.rw.slba;
	u32 nlba = cmd->sqe.rw.length;
	u32 nsid = cmd->sqe.rw.nsid;
	dsm_dw13_t dsmgmt;
	dsmgmt.dw13 = cmd->sqe.rw.dsmgmt;
	
	//list_add(, cmd);

	//cmd = list_head

	// cmd sanity check

	// opcode depend filed fill outside
	phif_cmd_req req;
	memset(&req, 0, sizeof(req));

	//QW0
	req.header.cnt = 2;
	req.header.dstfifo = ;   // dst subdst
	req.header.dst = MSG_NID_PHIF;
	req.header.prio = 0;
	req.header.msgid = MSGID_PHIF_CMD_REQ;
	req.header.tag = cmd->header.tag;
	req.header.ext_tag = 0;
	req.header.src = MSG_NID_HDC;
	req.header.vfa = 0;
	req.header.port = 0;
	req.header.vf = 0;
	req.header.sqid = cmd->header.sqid;
	req.header.hxts_mode = 0;   // KEY
	
	//QW1	
	req.cpa = start_lba / 1;     // LBA - > CPA
	req.hmeta_size = cfg;
	req.cph_size = cfg;    //read from config
	req.lb_size = cfg;
	req.crc_en = 1;
	req.dps = mode;
	req.flbas = mode;
	req.cache_en = 0;

	// FQMGR, 9 queue/die, Latency Control
	if (dsm & NVME_RW_DSM_LATENCY_IDLE) {
		req.band_rdtype = FQMGR_HOST_READ2; 
	}
		

	//QW2
	req.elba = (nsid<<32) | start_lba;

	// ask hw to export these define in .hdl or .rdl

	send_phif_cmd_req(&req);
}

