


int host_read_lba(hdc_nvme_cmd *cmd)
{
	u8 access_lat;
	u8 fua;
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

	// FUA is too bypass Cache
	fua = control.bits.fua;
	req.cache_en = !fua;

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

