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
 * host read datapath  rdp
 *
 */


// TODO: optimized when message send fail due to part busy, not re-constructured phif_cmd_req
// TODO: enqueue it in another queue host_nvme_cmd_wait_port_q, so next schedule, we only need send it directly.
void read_datapath_hdc(host_nvme_cmd_entry *host_cmd_entry)
{
	phif_cmd_cpl *cpl;

	switch (host_cmd_entry->state) 
	{
		case READ_FLOW_STATE_QUEUED:
			phif_cmd_req req;
			dp_setup_phif_cmd_req(&req, host_cmd_entry);
			host_cmd_entry->state = READ_FLOW_STATE_PHIF_REQ_READY;

		case READ_FLOW_STATE_PHIF_REQ_READY:
			if (send_phif_cmd_req(&req)) {
				// message port not available
				enqueue_front(&host_nvme_cmd_pend_q, host_cmd_entry->next);
				host_cmd_entry->state = READ_FLOW_STATE_QUEUED;	
				break;
			} else {
				host_cmd_entry->state = READ_FLOW_STATE_PHIF_REQ_SENDOUT;	
			}
			
		case READ_FLOW_STATE_PHIF_REQ_SENDOUT:
			host_cmd_entry->state = READ_FLOW_STATE_WAIT_PHIF_RSP;
			
		case READ_FLOW_STATE_WAIT_PHIF_RSP:
			break;
			
		case READ_FLOW_STATE_HAS_PHIF_RSP:
			cpl = __get_host_cmd_cpl_entry(host_cmd_entry->cmd_tag);
			setup_phif_cmd_cpl(cpl, host_cmd_entry);
			host_cmd_entry->state = READ_FLOW_STATE_PHIF_CPL_READY;

		case READ_FLOW_STATE_PHIF_CPL_READY		
			cpl = __get_host_cmd_cpl_entry(host_cmd_entry->cmd_tag);
			if (send_phif_cmd_cpl(cpl)) {
				// message Port BUSY
				enqueue_front(&host_nvme_cpl_pend_q, host_cmd_entry->next);
				host_cmd_entry->state = READ_FLOW_STATE_PHIF_CPL_READY;				
				break;
			} else {
				host_cmd_entry->state = READ_FLOW_STATE_PHIF_CPL_SENDOUT;
			}
			
		case READ_FLOW_STATE_PHIF_CPL_SENDOUT:
			// this host cmd is complete,tag will be released
			host_cmd_entry->state = READ_FLOW_STATE_COMPLETE;
			
		case READ_FLOW_STATE_COMPLETE:
			host_cmd_entry = get_next_host_cmd_entry();
			if (host_cmd_entry) {
				read_datapath_hdc(host_cmd_entry);
			} else {
				// there is no pending cmd need process
				break;
			}			
	}

	return;
}

/*
cqsts host_read_ingress(hdc_nvme_cmd *cmd)
{
	cqsts status = {0};	// status, default no error
	u64 start_lba = cmd->sqe.rw.slba;
	u16 nlb = cmd->sqe.rw.length;
	u32 nsid = cmd->sqe.rw.nsid;
	host_nvme_cmd_entry *host_cmd_entry;

	// para check
	if ((start_lba + nlb) > MAX_LBA) {
		print_err("the write LBA Range[%lld--%lld] exceed max_lba:%d", start_lba, start_lba+nlb, MAX_LBA);
		status.bits.sct = NVME_SCT_GENERIC;
		status.bits.sc = NVME_SC_LBA_RANGE;
		return status;	
	}

	if (nsid > MAX_NSID) {
		print_err("NSID:%d is Invalid", nsid);
		status.bits.sct = NVME_SCT_GENERIC;
		status.bits.sc = NVME_SC_INVALID_NS;
		return status;	
	}

	// tag is assigned by PHIF, it can guarantee this tag is free,
	// we use the tag as array index, so this host_cmd_entry is free,we 
	host_cmd_entry = __get_host_cmd_entry(cmd->header.tag);
	if (host_cmd_entry == NULL) {
		// it should never, else we should enlarge the gat array
	} else {
		// fill it from hdc_nvme_cmd
		save_in_host_cmd_entry(host_cmd_entry, cmd);
		enqueue(&host_nvme_cmd_pend_q, host_cmd_entry->next);		
		host_cmd_entry->state = READ_FLOW_STATE_QUEUED;
	}
	
	host_cmd_entry = dequeue(&host_nvme_cmd_pend_q);

	assert(host_cmd_entry);
	
	read_datapath_hdc(host_cmd_entry);

	return 0;
}*/

