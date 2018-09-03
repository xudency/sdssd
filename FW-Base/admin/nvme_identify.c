

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

void identify_namespace_init(u32 nsid)
{
	struct nvme_id_ns *nsdata = get_identify_ns(nsid);

	nsdata->nsze = ;
	nsdata->ncap = ;

	// TODO:
}

// return a 4KB data buffer that describes info about the NVM subsystem
cqsts handle_admin_identify(hdc_nvme_cmd *cmd)
{
	u8 cns = cmd->sqe.identify.cns;	

	switch (cns) {
	case NVME_ID_CNS_NS:
		struct nvme_id_ns *nsinfo = get_identify_ns(cmd->sqe.identify.nsid);
		// copy it to host, the host address is indicated in dptr
		wdma


	
		send_phif_wdma_req
		wait_for_phif_wdma_rsp
		
		break;

	case NVME_ID_CNS_CTRL:
	}
}

