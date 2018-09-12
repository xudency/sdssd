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
	struct nvme_create_cq *nvme_cmd = &host_cmd_entry->sqe.create_cq;

	// max_nvme_queue_id is set in set feature command nsqr ncqr
	if (nvme_cmd->cqid == 0 || nvme_cmd->cqid > max_nvme_queue_id) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_QID_INVALID);
		return NVME_REQUEST_INVALID;
	}

	// TODO: check if this cq is exist

	if (nvme_cmd->qsize == 0 || nvme_cmd->qsize > max_nvme_queue_size) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_QUEUE_SIZE);
		return NVME_REQUEST_INVALID;
	}
	
	if (!(nvme_cmd->cq_flags & NVME_QUEUE_PHYS_CONTIG)) {
		//SQ address physical not contigously
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_CQ_INVALID);
		return NVME_REQUEST_INVALID;
	}

	// TODO:: config cq related register  
	//1.CQ Base Address   
	//2.CQ SIZE  
	//3.CQ Interrupt vector

	return NVME_REQUEST_COMPLETE;
}


int handle_admin_create_sq(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_create_sq *nvme_cmd = &host_cmd_entry->sqe.create_sq;

	if (nvme_cmd->sqid==0 || nvme_cmd->sqid > max_queue_id) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_QID_INVALID);
		return NVME_REQUEST_INVALID;
	}

	// check if the associated CQ has created.
	if (nvme_cmd->cqid) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_QID_INVALID);
		return NVME_REQUEST_INVALID;
	}

	// TODO:: check if this sq is exist

	
	// check if the SQ address is physical contigously
	if (!(nvme_cmd->sq_flags & NVME_QUEUE_PHYS_CONTIG)) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_CQ_INVALID);
		return NVME_REQUEST_INVALID;
	}

	// TODO:: config sq related register  
	//1.SQ Base Address   
	//2.SQ SIZE   
	//3.SQ PRIO and associated CQ



	set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_SUCCESS);

	return NVME_REQUEST_COMPLETE;
}


int handle_admin_delete_sq(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_delete_queue *nvme_cmd = &host_cmd_entry->sqe.delete_queue;
	
	if (nvme_cmd->qid==0 || nvme_cmd->qid > max_nvme_queue_id) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_QID_INVALID);
		return NVME_REQUEST_INVALID;
	}

	
	// TODO:: check this sq is exist(has created).



	// TODO:: Disable this SQ, stop HW to fetch cmd from this SQ, meanwhile tell HW to clean this SQ



	// TODO:: if there is outstanding command of this sq(Not Clear)
	// 1. pending this command, push it to a queue, yield cpu to other task
	// 2. while-wait here, until clear bit done


	// TODO:: if this sq is in clear status then clear this sq related register
	//1.SQ Base Address
	//2.SQ SIZE
	//3.SQ PRIO and Associated CQ

	return NVME_REQUEST_COMPLETE;
}


int handle_admin_delete_cq(host_nvme_cmd_entry *host_cmd_entry)
{
	struct nvme_delete_queue *nvme_cmd = &host_cmd_entry->sqe.delete_queue;
	
	if (nvme_cmd->qid==0 || nvme_cmd->qid > max_nvme_queue_id) {
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_CMD_SPECIFIC, NVME_SC_QID_INVALID);
		return NVME_REQUEST_INVALID;
	}

	
	// TODO:: check this cq is exist(has created).


	// TODO:: all sq associated to this cq must has delete ahead


	// TODO:: clear cq related register
	//1.CQ Base Address   
	//2.CQ SIZE  
	//3.CQ Interrupt vector

	
	set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_SUCCESS);

	return NVME_REQUEST_COMPLETE;
}



