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
 * nvme admin command, set/get feature
 *
 */


nvme_feature_cfg gat_nvme_current_feature;

nvme_feature_cfg gat_nvme_default_feature;


//Features that can be configured with Set Features
set_feature_fn gat_set_feature_func[] = 
{
	[NVME_FEAT_UNDEFID] 		= NULL,
	[NVME_FEAT_ARBITRATION] 	= xx1,
	[NVME_FEAT_POWER_MGMT]		= xx2,
	[NVME_FEAT_LBA_RANGE] 		= xx3,
	[NVME_FEAT_TEMP_THRESH] 	= xx4,
	[NVME_FEAT_ERR_RECOVERY]	= xx5,
	[NVME_FEAT_VOLATILE_WC] 	= xx6,
	[NVME_FEAT_NUM_QUEUES]		= setft_queue_number,
	[NVME_FEAT_IRQ_COALESCE]	= xx8,
	[NVME_FEAT_IRQ_CONFIG]		= xx9,
	[NVME_FEAT_WRITE_ATOMIC]	= xxa,
	[NVME_FEAT_ASYNC_EVENT] 	= xxb,
	[NVME_FEAT_AUTO_PST]		= xxc,
	[NVME_FEAT_HOST_MEM_BUF]	= xxd,
	[NVME_FEAT_TIMESTAMP] 		= xxe,
	[NVME_FEAT_KATO]			= xxf,
};

//Features that can be retrieved with Get Features
get_feature_fn gat_get_feature_func[] = 
{
	[NVME_FEAT_UNDEFID] 		= NULL,
	[NVME_FEAT_ARBITRATION] 	= xx1,
	[NVME_FEAT_POWER_MGMT]		= xx2,
	[NVME_FEAT_LBA_RANGE] 		= setft_lba_range_type,
	[NVME_FEAT_TEMP_THRESH] 	= xx4,
	[NVME_FEAT_ERR_RECOVERY]	= xx5,
	[NVME_FEAT_VOLATILE_WC] 	= xx6,
	[NVME_FEAT_NUM_QUEUES]		= getft_queue_number,
	[NVME_FEAT_IRQ_COALESCE]	= xx8,
	[NVME_FEAT_IRQ_CONFIG]		= xx9,
	[NVME_FEAT_WRITE_ATOMIC]	= xxa,
	[NVME_FEAT_ASYNC_EVENT] 	= xxb,
	[NVME_FEAT_AUTO_PST]		= xxc,
	[NVME_FEAT_HOST_MEM_BUF]	= xxd,
	[NVME_FEAT_TIMESTAMP] 		= xxe,
	[NVME_FEAT_KATO]			= xxf,
};


/////////////////////////////////////////Set Feature function/////////////////////////////////////////

int setft_lbart_rdma_completion(void *para)
{
	return 0;
}


int setft_lba_range_type(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;

	u8 nlr = nvme_cmd->dword11 & NVME_LBART_NUM_MASK;   		// 0-base

	u64 cbuff_addr = &gat_nvme_current_feature.lba_range[0];

	fw_send_rdma_req(nvme_cmd->dptr.prp1, cbuff_addr, (1+nlr)*NVME_LBART_ENTRY_SIZE, 
					 host_cmd_entry->cmd_tag, setft_lbart_rdma_completion, host_cmd_entry);

	// wait response
	return NVME_REQUEST_PROCESSING;
}

int setft_queue_number(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;

	u16 ncqr = upper_16_bits(nvme_cmd->dword11);
	u16 nsqr = lower_16_bits(nvme_cmd->dword11);

	// this is IO queue number,NOT include Admin queue
	if (ncqr > 0xfffe || nsqr > 0xfffe) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_INVALID_FIELD);
		return NVME_REQUEST_INVALID;
	}

	writel(ncqr, NCQR);
	writel(nsqr, NSQR);

	return NVME_REQUEST_COMPLETE;
}

int handle_admin_set_feature(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;
	u8 featueid = nvme_cmd->fid.sf_dw10.fid;

	if (featueid >= NVME_FEAT_ARBITRATION && featueid <= NVME_FEAT_KATO) {
		return gat_set_feature_func[featueid](host_cmd_entry);
	} else {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_INVALID_FIELD);
		return NVME_REQUEST_INVALID;
	}
}

/////////////////////////////////////////Get Feature function/////////////////////////////////////////

int getft_lbart_wdma_completion(void *para)
{
	host_nvme_cmd_entry *host_cmd_entry = (host_nvme_cmd_entry *)para;

	// result is command specific
	host_cmd_entry->result = NVME_LBART_MAX_ENTRYS;

	return 0;
}

int getft_lba_range_type(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;

	//u8 nlr = nvme_cmd->dword11 & NVME_LBART_NUM_MASK;   		// 0-base
	u64 cbuff_addr;

	// sel is supported? ctrl.ONCS.bit4
	if (nvme_cmd->fid.gf_dw10.sel == NVME_FEAT_SEL_CURRENT) {
		cbuff_addr = &gat_nvme_current_feature.lba_range[0];
	} else {
		cbuff_addr = &gat_nvme_default_feature.lba_range[0];
	}

	// BEWARE, dptr is 4K size and phisically contiguous
	fw_send_wdma_req(nvme_cmd->dptr.prp1, cbuff_addr, NVME_LBART_TBL_SIZE, host_cmd_entry->cmd_tag, 
					getft_lbart_wdma_completion, host_cmd_entry);

	// wait response
	return NVME_REQUEST_PROCESSING;
}

int getft_queue_number(host_nvme_cmd_entry * host_cmd_entry)
{
	
}

// refer NVMe spec-1_3c  5.21
int handle_admin_get_feature(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;
	u8 featueid = nvme_cmd->fid.gf_dw10.fid;

	if (featueid >=NVME_FEAT_ARBITRATION && featueid <= NVME_FEAT_KATO) {
		return gat_get_feature_func[featueid](host_cmd_entry);
	} else {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_INVALID_FIELD);
		return NVME_REQUEST_INVALID;
	}
}


