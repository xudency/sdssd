


// Process Host Admin command 
void handle_nvme_admin_command(hdc_nvme_cmd *cmd)
{
	u8 opcode = cmd->sqe.common.opcode;

	switch (opcode) {
	case nvme_admin_identify:
		xxxxx;
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
