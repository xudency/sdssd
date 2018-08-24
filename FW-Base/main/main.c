/*
 * FW main to init pherial, power on
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>

#include "tsk_sched.h"

// Check we didin't inadvertently grow the command struct and valid configuration
// XXX, can it check when complie, thus can save time for power on
bool validity_check()
{
	//data struct size check
	//config validate check
}


///////////////////////////////event_handler.c
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

	switch (opcode) {
	
	}


	return;
}




// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd

// taskfn demo
void hdc_host_cmd_task(void *para)
{
	// HW need a Event Notifier Register: EVENTF
	// EVENTF
	u32 event = readl(EVENTF);
	
	if (!bit_test(event, 0)) 
		return;   // no host cmd need process, this task exit 

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
	bit_clear(event, 0);

	return;
}

/* SPA is a fix memory located in CPU's DCCM */


int fw_tasks_create(void)
{
	create_task(hdc_host_cmd_task, NULL, HDC, 0);

	//create_task(taskfn handler, void * para, HDC, 1);
}



// this is FW entrance, Main
static int __init fw_init(void)
{
	printk("FW start run ...\n");


	/////////////////////////////BSP///////////////////////////
	//ddr_init();

	//sram_init();

	// XXX: is this in FSQ-EM6 micro-code? 
	//nand_init();

	//cpu_init();    //clock interrupt timer AHB

	// uart gpio iic spi watchdog
	//peripheral_init();   // CPU APB 

	//hw_init();   // register config

	
	/////////////////////////////FTL///////////////////////////
	// CBUFF flat address partition
	//ftl_sdata_allocated_init();

	//some ctl_ctx init
	//wpd_init();
	
	//rdp_init();

	//recycle_init();

	//ckpt_init();

	
	/////////////////////////////OS///////////////////////////
	os_init();   // a micro-kernel, only contain task scheduler 

	fw_tasks_create();

	os_start();   //Linux start_kernel
	
	return 0;
}

static void __exit fw_exit(void)
{
	printk("FW exit\n");

	return;
}


module_init(fw_init);
module_exit(fw_exit);
MODULE_AUTHOR("Dengcai Xu <xudc14@163.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("fw-base FTL simulation on Linux");
