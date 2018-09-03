

#define MAX_NSID  15     // 0 1 2 ... 15
#define TOTAL_NS_NUM  (MAX_NSID + 1)


struct nvme_id_ns gat_identify_namespaces[TOTAL_NS_NUM];

struct nvme_id_ctrl gat_identify_controller;


struct nvme_id_ns *get_identify_ns(u32 nsid)
{
	return gat_identify_namespaces + nsid;
}

void identify_data_init(void)
{
	int i;
	
	for (i = 0; i < TOTAL_NS_NUM; i++) {
		identify_namespace_init(i);
	}

	identify_controller_init();

	return 0;
}

// TODO: init when power on
void identify_namespace_init(u32 nsid)
{
	struct nvme_id_ns *nsdata = get_identify_ns(nsid);

	nsdata->nsze = ;
	nsdata->ncap = ;
}


append



// return a 4KB data buffer that describes info about the NVM subsystem
cqsts handle_admin_identify(hdc_nvme_cmd *cmd)
{
	u8 cns = cmd->sqe.identify.cns;	

	switch (cns) {
	case NVME_ID_CNS_NS:

		u32 nsid = cmd->sqe.identify.nsid;	

		struct nvme_id_ns *nsinfo = get_identify_ns(nsid);
		// copy it to host, the host address is indicated in dptr
		phif_wdma_req_mandatory m;

		// TODO: tag allocated?
		msg_header_filled(&m.header, 6, MSG_NID_PHIF, MSG_NID_PHIF, MSGID_PHIF_WDMA_REQ, 
						u8 tag, u8 ext_tag, MSG_NID_HDC, 0);

		m.control.blen = SZ_4K;			// length
		m.control.pld_qwn = 1;

		// TODO: if prp1 is not 4K align, prp2 will used
		m.hdata_addr = cmd->sqe.identify.dptr.prp1;    // host data address

		//QW_ADDR
		phif_wdma_req_optional o;
		o.cbuff_addr = &gat_identify_namespaces[nsid];   // cbuff address

		u8 valid = WDMA_QW_ADDR;
		send_phif_wdma_req(&m, &o, valid);
		
		break;

	case NVME_ID_CNS_CTRL:
	}

	return;
}


// fwdata means FW define specific data, include sysdata and manage data
// host read, data in sram is exclusived
wdma_fwdata_cbuff_to_host()
{

}
