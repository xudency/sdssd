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


int handle_admin_format_nvm(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_format_cmd *fmtnvm = &host_cmd_entry->sqe.format;
	u32 nsid = fmtnvm->nsid;
	struct nvme_id_ns *ns_data = get_identify_ns(nsid);
	//struct nvme_lbaf *lbaf

	if (nvme_nsid_valid(nsid)) {	
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_NS_ID_UNAVAILABLE);
		return -1;
	}

	fmt_dw10_t fmt_ctrl;
	fmt_ctrl.dw10 = fmtnvm->cdw10;

	// set 1.LB/Meta SIZE choose from 1 of 16 type  2.DIF DIX
	ns_data->flbas = fmt_ctrl.bits.lbaf | (fmt_ctrl.bits.mset << 4);

	// set 3.PI type  4. PI Locaton in meta  
	ns_data->dps = fmt_ctrl.bits.pi | (fmt_ctrl.bits.pil << 3);

	return 0;
}


