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
 * NVMe Spec relative 
 *
 */


// NVMe Over PCIe
//     admin command only support PRP mode
//     IO command SGL/PRP is optional 

// NVMe Over Fabrics, admin and io all only support SGL

nvme_sgl_parse(struct nvme_sgl_desc sgl)
{
	
}

// XXXX: nsid = 0
bool nvme_nsid_valid(u32 nsid)
{
	struct nvme_id_ctrl *ctrl_data = get_identify_ctrl();

	if ((nsid > ctrl_data->nn) || (nsid == 0))
		return false

	return true;
}

