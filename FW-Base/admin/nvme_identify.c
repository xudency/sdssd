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
 * nvme admin format command implement
 *
 */


#define MAX_NSID  15     // 1 2 ... 15,  nsid=0 is controller

struct nvme_id_ns gat_identify_namespaces[MAX_NSID];

struct nvme_id_ctrl gat_identify_controller;


struct nvme_id_ns *get_identify_ns(u32 nsid)
{
	assert(nsid);
	
	return gat_identify_namespaces + nsid-1;
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
	ctrl_data->nn = MAX_NSID;
}

void nvme_ns_info_init(u32 nsid)
{
	int i;
	struct nvme_id_ns *ns_data = get_identify_ns(nsid);
	
    ns_data->nsze 				= MAX_LBA + 1; 		        
    ns_data->ncap 				= MAX_LBA + 1;		    
    ns_data->nuse 				= MAX_LBA + 1;
	
	// set LBA Format support para
	for (i=0; i < 16; i++) {
		ns_data->lbaf[i].ms = METADATA_16B;
		ns_data->lbaf[i].ds = 12;    // 2^12  4KB
		ns_data->lbaf[i].rp = NVME_LBAF_RP_GOOD;
		ns_data->lbaf[i].cphs = 16;  // CPH
	}
}

// return a 4KB data buffer that describes info about the NVM subsystem
int handle_admin_identify(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_identify *idn = &host_cmd_entry->sqe.identify;
	u8 cns = idn->cns;
	u8 tag = host_cmd_entry->cmd_tag;
	u64 cbuff;
	void *ctx = host_cmd_entry;

	switch (cns) {
	case NVME_ID_CNS_NS:
		cbuff = (u64)get_identify_ns(idn->nsid);
		wdma_cbuff_to_host_dptr(idn->dptr, cbuff, NVME_IDENTIFY_DATA_SIZE, tag, 
								host_cmd_rwdma_completion, ctx);
		break;
		
	case NVME_ID_CNS_CTRL:
		cbuff = (u64)get_identify_ctrl();
		wdma_cbuff_to_host_dptr(idn->dptr, cbuff, NVME_IDENTIFY_DATA_SIZE, tag, 
								host_cmd_rwdma_completion, ctx);
		break;
				
	case NVME_ID_CNS_NS_ACTIVE_LIST:
		xxxxxxxx;
		break;

	}

	return 0;
}


