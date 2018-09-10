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

int handle_admin_set_feature(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;


	return NVME_REQUEST_COMPLETE;
}

// refer NVMe spec-1_3c  5.21
int handle_admin_get_feature(host_nvme_cmd_entry * host_cmd_entry)
{
	struct nvme_features *nvme_cmd = &host_cmd_entry->sqe.features;


	return NVME_REQUEST_COMPLETE;
}

