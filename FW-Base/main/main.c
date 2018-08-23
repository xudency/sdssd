/*
 * FW main to init pherial, power on
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>


// Check we didin't inadvertently grow the command struct and valid configuration
// XXX, can it check when complie, thus can save time for power on
bool validity_check()
{
	//data struct size check
	//config validate check
}

typedef struct host_nvme_cmd {
	//header;
	u16 sqid;
	struct nvme_command sqe;
} host_cmd_t;


///////////////////////////////event_handler.c
// HW notify CPU method
//    1. HW interrupt ---> FW check Event register what happen ---> FW handle it
//    2. FW Polling check Event register what happen ---> if yes handle it

// Process Host Admin command 
void handle_nvme_admin_command(host_cmd_t *cmd)
{
	u8 opcode = cmd->sqe.common.opcode;

	switch (opcode) {
	
	}


	return;
}

// Process Host IO Comamnd
void handle_nvme_io_command(host_cmd_t *cmd)
{
	u8 opcode = cmd->sqe.common.opcode;

	switch (opcode) {
	
	}


	return;
}


// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd

// taskfn demo
void hdc_host_cmd_fn(void *para)
{
	host_cmd_t *cmd = (host_cmd_t *)para;

	if (cmd->sqid == 0) {
		// admin queue, this is admin cmd
		handle_nvme_admin_command(cmd);
	} else {
		// io command
		handle_nvme_io_command(cmd);
	}

	return;
}



int fw_tasks_create(void)
{
	
	create_task(PHIF_NVME_CMD, hdc_host_cmd_fn);
}



// this is FW entrance, Main
static int __init fw_init(void)
{
	printk("FW start run ...\n");


	/////////////////////////////BSP///////////////////////////
	//ddr_init();

	//sram_init();

	// XXX: is this in FSQ-EM6 micro-code? 
	//nandflash_init();

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