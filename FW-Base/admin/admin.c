
// WDMA:  Host Memory to CBUFF
// RDMA:  CBUFF to Host Memory

/*cbuff_to_host()
{

}*/



// Process Host Admin command 
cqsts handle_nvme_admin_command(hdc_nvme_cmd *cmd)
{
	cqsts status = {0};
	u8 opcode = cmd->sqe.common.opcode;
	u8 flags = cmd->sqe.common.flags;

	// Admin command PSDT = 0, it only support PRP
	if (flags & NVME_CMD_SGL_ALL) {
		print_err("SGLs shall not be used for Admin commands in NVMe Over PCIe implement");
		status.status = NVME_SC_INVALID_FIELD;
		return status;
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
}

