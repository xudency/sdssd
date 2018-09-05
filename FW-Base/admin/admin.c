
// WDMA:  Host Memory to CBUFF
// RDMA:  CBUFF to Host Memory

/*cbuff_to_host()
{

}*/


// Process Host Admin command 
void handle_nvme_admin_command(hdc_nvme_cmd *cmd)
{
	//cqsts status = {0};
	u8 opcode = cmd->sqe.common.opcode;
	u8 flags = cmd->sqe.common.flags;
	u8 tag = cmd->header.tag;
	
	// tag is assigned by PHIF, it can guarantee this tag is free,
	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(tag);

	save_in_host_cmd_entry(host_cmd_entry, cmd);

	// Admin command PSDT = 0, it only support PRP
	if (flags & NVME_CMD_SGL_ALL) {
		print_err("SGLs shall not be used for Admin commands in NVMe Over PCIe implement");
		host_cmd_entry->sta_sc  = NVME_SC_INVALID_FIELD;
		goto cmd_quit;
	}

	switch (opcode) {
	case nvme_admin_identify:
		handle_admin_identify(cmd);
		break;
	case nvme_admin_set_features:
		xxxxx;
		break;
	case nvme_admin_get_features:
		xxxxx;
		break;
	
	}
	
	return;
	
	// this NVMe Command is Invalid, post CQE to host immediately
	phif_cmd_cpl *cpl = __get_host_cmd_cpl_entry(tag)
	setup_phif_cmd_cpl(cpl, host_cmd_entry);
	if (send_phif_cmd_cpl(cpl)) {
		enqueue_front(host_nvme_cpl_pend_q, host_cmd_entry->next);
	}

cmd_quit:
	
}

