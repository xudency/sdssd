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


///////////////////////////////event_handler.c
// HW notify CPU method
//    1. HW interrupt ---> FW check Event register what happen ---> FW handle it
//    2. FW Polling check Event register what happen ---> if yes handle it


// Process Host Admin command 
handle_nvme_admin_command()
{

}

// Process Host IO Comamnd
handle_nvme_io_command()
{

}


hdc_fetch_command_phif()
{
	
}

// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd
// CPU(HDC) 
void hdc_nvme_cmd()
{
	//process();
	//phif_cmd_request_to_chunk();

	
}


/*void listening_loop(void)
{
	while (1) 
	{
		schedule();
	}
}*/


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
	// CBUFF Address 
	//ftl_sdata_allocated_init();

	//wpd_init();
	
	//rdp_init();

	//recycle_init();

	//ckpt_init();

	//schedule_init();  // a micro-kernel, only contain task scheduler 

	// Now FW HW has all ready to handle host request
	//listening_loop();
	schedule();
	
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
