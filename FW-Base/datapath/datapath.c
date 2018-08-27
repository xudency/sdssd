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


// HW notify CPU method
//    1. HW interrupt ---> FW check Event register what happen ---> FW handle it
//    2. FW Polling check Event register what happen ---> if yes handle it

// Process Host Admin command 
void handle_nvme_admin_command(hdc_nvme_cmd *cmd)
{
	u8 opcode = cmd->sqe.common.opcode;

	switch (opcode) {
	
	}


	return;
}

// Process Host IO Comamnd
void handle_nvme_io_command(hdc_nvme_cmd *cmd)
{
	u8 opcode = cmd->sqe.common.opcode;
	u32 start_lba = cmd->sqe.rw.slba;
	u32 nlba = cmd->sqe.rw.length;

	

	switch (opcode) {
	case nvme_io_read:
		host_read_lba();
		break;
	case nvme_io_write:
		host_write_lba();
		break;
	case nvme_io_flush:
		break;
	default:
		print_err("Opcode:0x%x Invalid", opcode);
		break;
	}


	return;
}

// when command process completion, this is the hook callback routine
void process_completion_task(void *para)
{
	// TODO: process all to Post CQE to Host

	yield_task(HDC, completion_prio);

	return;
}

// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd

// taskfn demo
void hdc_host_cmd_task(void *para)
{
	// HW need a Event Notifier Register: EVENTF
	// EVENTF
	//u32 event = readl(EVENTF);
	
	//if (!bit_test(event, 0)) 
		//return;   // no host cmd need process, this task exit 

	// HW fetch and fwd the Host CMD to a fix position
	// queue tail head is managed by HW
	//hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)para;
	hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)HOST_CMD_SPM;

	if (cmd->header.sqid == 0) {
		// admin queue, this is admin cmd
		handle_nvme_admin_command(cmd);
	} else {
		// io command
		handle_nvme_io_command(cmd);
	}

	// clear the bit,thus hw can get the next cmd from CMD Table
	//bit_clear(event, 0);

	return;
}

