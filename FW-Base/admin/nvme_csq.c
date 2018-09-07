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
 * nvme admin create sq/cq delete sq/cq command implement
 *
 */

int handle_admin_create_cq(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_create_cq *sqe = host_cmd_entry->sqe.create_cq;

	// TODO: para check, 1.qid  2.q_size   ,check by Host ....?
	
	
	if (!(sqe->cq_flags & NVME_QUEUE_PHYS_CONTIG)) {
		//SQ address physical not contigously
		host_cmd_entry->sta_sc = NVME_SC_CQ_INVALID
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_CQ_INVALID);
		return -1;
	}

	// TODO:: config cq related register  1.CQBA   2.CQSIZE  3.CQINT

	return 0;
}

