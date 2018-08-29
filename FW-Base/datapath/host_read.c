


int host_read_lba(hdc_nvme_cmd *cmd)
{
	u8 access_lat;
	//u8 fua;
	dsm_dw13_t dsmgmt;
	//ctrl_dw12h_t control;
	u32 start_lba = cmd->sqe.rw.slba;
	u32 nlba = cmd->sqe.rw.length;
	u32 nsid = cmd->sqe.rw.nsid;
	dsmgmt.dw13 = cmd->sqe.rw.dsmgmt;
	//control.ctrl = cmd->sqe.rw.control;
	//list_add(, cmd);

	//cmd = list_head

	// cmd sanity check

	// opcode depend filed fill outside
	phif_cmd_req req;
	memset(&req, 0, sizeof(req));

	// TODO: move the common init to send_phif_cmd_req, regardless of opcode

	//QW0
	req.header.cnt = 2;  // common
	req.header.dstfifo = MSG_NID_PHIF;   // common
	req.header.dst = MSG_NID_PHIF; // common
	req.header.prio = 0; // common
	req.header.msgid = MSGID_PHIF_CMD_REQ;// common

	// for host cmd, we support max 256 outstanding commands, tag 8 bit is enough
	// EXTAG no used, must keep it as 0,   tag = seq.cmdid
	// tag is assign by PHIF in hdc_nvme_cmd, phif_req_cmd copy from it
	req.header.tag = cmd->header.tag; // common
	req.header.ext_tag = PHIF_EXTAG;// common
	
	req.header.src = MSG_NID_HDC;// common
	req.header.vfa = 0;// common
	req.header.port = 0;// common
	req.header.vf = 0;// common
	req.header.sqid = cmd->header.sqid;// common
	req.header.hxts_mode = 0;
	
	//QW1	
	req.cpa = start_lba / 1;     // LBA - > CPA
	req.hmeta_size = ns_cfg;   // common
	req.cph_size = ns_cfg;    // common
	req.lb_size = ns_cfg;// common
	req.crc_en = 1;// common
	req.dps = mode;
	req.flbas = mode;
	req.cache_en = 1;

	//Latency Control via Queue schedule in FQMGR(9 queue per die)
	access_lat = dsmgmt.bits.dsm_latency;
	switch (access_lat) {
	case NVME_RW_DSM_LATENCY_NONE:
		// XXX: if no latency info provide, add to which queue ?
		req.band_rdtype = FQMGR_HOST_READ1;
		break;	
	case NVME_RW_DSM_LATENCY_IDLE:
		req.band_rdtype = FQMGR_HOST_READ2;
		break;
	case NVME_RW_DSM_LATENCY_NORM:
		req.band_rdtype = FQMGR_HOST_READ1;
		break;
	case NVME_RW_DSM_LATENCY_LOW:
		req.band_rdtype = FQMGR_HOST_READ0;
		break;
	default:
		print_err("access latency Invalid");
		return - EINVAL;
	}

	//QW2
	req.elba = (nsid<<32) | start_lba;

	// ask hw to export these define in .hdl or .rdl

	send_phif_cmd_req(&req);
}

