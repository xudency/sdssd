

#define MAX_NSID  15     // 0 1 2 ... 15
#define TOTAL_NS_NUM  (MAX_NSID + 1)


struct nvme_id_ns gat_identify_namespaces[TOTAL_NS_NUM];

struct nvme_id_ctrl gat_identify_controller;


struct nvme_id_ns *get_identify_ns(u32 nsid)
{
	return gat_identify_namespaces + nsid;
}

struct nvme_id_ctrl *get_identify_ctrl()
{
	return &gat_identify_controller;
}

// TODO: init when power on
void nvme_ctrl_info_init(void)
{
	struct nvme_id_ctrl *ctrl_data = get_identify_ctrl();
	ctrl_data->acl = ;
}

void nvme_ns_info_init(u32 nsid)
{
	struct nvme_id_ns *ns_data = get_identify_ns(nsid);

	ns_data->nsze = ;
	ns_data->ncap = ;
}

// return a 4KB data buffer that describes info about the NVM subsystem
cqsts handle_admin_identify(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_identify *idn = &host_cmd_entry->sqe.identify;
	u8 cns = idn->cns;
	u8 tag = host_cmd_entry->cmd_tag;
	u64 cbuff;

	switch (cns) {
	case NVME_ID_CNS_NS:
		cbuff = (u64)get_identify_ns(idn->nsid);
		wdma_cbuff_to_host_dptr(idn->dptr, cbuff, SZ_4K, tag, host_cmd_wdma_completion);
		break;
		
	case NVME_ID_CNS_CTRL:
		cbuff = (u64)get_identify_ctrl();
		wdma_cbuff_to_host_dptr(idn->dptr, cbuff, SZ_4K, tag, host_cmd_wdma_completion);
		break;
				
	case NVME_ID_CNS_NS_ACTIVE_LIST:
		xxxxxxxx;
		break;

	}

	return;
}


