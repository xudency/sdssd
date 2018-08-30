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
 * read/write datapath, ingress of commmand handler
 *
 */

#include "nvme_spec.h"
#include "msg_fmt.h"


host_nvme_cmd_entry    gat_host_nvme_cmd_array[HOST_NVME_CMD_ENTRY_CNT]       = {{{0}}};


// HW notify CPU method
//    1. HW interrupt ---> FW check Event register what happen ---> FW handle it
//    2. FW Polling check Event register what happen ---> if yes handle it


// Process Host IO Comamnd
int handle_nvme_io_command(hdc_nvme_cmd *cmd)
{
	int res = 0;
	
	u8 opcode = cmd->sqe.common.opcode;

	switch (opcode) {
	case nvme_io_read:
		res = host_read_lba(cmd);
		break;
	case nvme_io_write:
		res = host_write_lba(cmd);
		break;
	case nvme_io_flush:
		break;
	default:
		res = -EINVAL;
		print_err("Opcode:0x%x Invalid", opcode);
		break;
	}

	return res;
}

// when command process completion, this is the hook callback routine
int process_completion_task(void *para)
{
	// TODO: process all to Post CQE to Host

	yield_task(HDC, completion_prio);

	return 0;
}


int send_phif_cmd_req(phif_cmd_req *req)
{
	//common filed filled

	if (port_is_available()) {
		memcpy(PHIF_CMD_REQ_SPM, req, sizeof(req)); 
		return 0;
	} else {
		/*break, CPU schedule other task*/
		return PHIF_PORT_BUSY;
	}
}

// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd

// taskfn demo
int hdc_host_cmd_task(void *para)
{
	// HW fetch and fwd the Host CMD to a fix position
	// queue tail head is managed by HW
	//hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)para;
	hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)HDC_NVME_CMD_SPM;

	if (cmd->header.sqid == 0) {
		// admin queue, this is admin cmd
		return handle_nvme_admin_command(cmd);
	} else {
		// io command
		return handle_nvme_io_command(cmd);
	}

}

