
// WDMA:  Host Memory to CBUFF
// RDMA:  CBUFF to Host Memory

/*cbuff_to_host()
{

}*/


// Process Host Admin command 
int handle_nvme_admin_command(host_nvme_cmd_entry *host_cmd_entry)
{
	u8 opcode = host_cmd_entry->sqe.common.opcode;
	u8 flags = host_cmd_entry->sqe.common.flags;

	// Admin command PSDT = 0, it only support PRP
	if (flags & NVME_CMD_SGL_ALL) {
		print_err("SGLs shall not be used for Admin commands in NVMe Over PCIe implement");
		host_cmd_entry->sta_sc  = NVME_SC_INVALID_FIELD;
		return -1;
	}

	//enqueue(&host_nvme_cmd_pend_q, host_cmd_entry->next);

	switch (opcode) {
	case nvme_admin_identify:
		handle_admin_identify(host_cmd_entry);
		break;
	case nvme_admin_set_features:
		xxxxx;
		break;
	case nvme_admin_get_features:
		xxxxx;
		break;
	
	}
	
	return 0;
		
}

