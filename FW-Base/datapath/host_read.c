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
			setup_phif_cmd_req(&req, host_cmd_entry);
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


