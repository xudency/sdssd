

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


// return a 4KB data buffer that describes info about the NVM subsystem
cqsts handle_admin_identify(hdc_nvme_cmd *cmd)
{
	u8 cns = cmd->sqe.identify.cns;	
	struct nvme_identify *idn = &cmd->sqe.identify;

	switch (cns) {
	case NVME_ID_CNS_NS:
		u64 cbuff = (u64)get_identify_ns(idn->nsid);
		u64 prp1 = idn->dptr.prp1;
		u64 prp2 = idn->dptr.prp2;
		u64 prp1_offset = page_offset(prp1);
		u16 length = SZ_4K - prp1_offset;

		// Concer case handle 1.cmdtag alloc fail  2.wdma_req_spm busy
		if (!prp1_offset) {
			// PRP1 is 4K align
			wdma_read_fwdata_to_host(prp1, cbuff, SZ_4K);
		} else {
			// PRP1 not 4K align, PRP2 is used
			wdma_read_fwdata_to_host(prp1, cbuff, length);
			wdma_read_fwdata_to_host(prp2, cbuff+length, prp1_offset);
		}

		break;

	case NVME_ID_CNS_CTRL:
	}

	return;
}

// move cbuff data to host complete
void phif_wdma_response_to_hdc(void)
{
	phif_wdma_rsp *rsp = (phif_wdma_rsp *)PHIF_WDMA_RSP_SPM;

	// release this HDC cmd_tag allocated before
	u8 cmd_tag = rsp->tag;

	hdc_free_cmdtag(cmd_tag);

	// prp1 part and prp2 part all response, structured a phif_cmd_cpl to PHIF
	tag->hookfn();
}

