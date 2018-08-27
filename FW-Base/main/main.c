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


viod handle_hw_event(u8 cpu)
{
	int bit;
	TCB *task;
    TCB **tasks = gat_tasks_ctl_ctx.task_array[cpu];
	u32 hw_event = readl(HW_EVENTF);   // read this register, it will clear

	for_each_set_bit(bit, &hw_event, REG32_BITS) {
		task = tasks[bit];
		if (task && task->state == TASK_STATE_READY) run_task(task);
	}
}

viod handle_fw_event(u8 cpu)
{
	int bit;
	TCB *task;
    TCB **tasks = gat_tasks_ctl_ctx.task_array[cpu];
	u32 fw_event = readl(FW_EVENTF);   // read this register, it will clear

	for_each_set_bit(bit, &fw_event, REG32_BITS) {
		task = tasks[bit];
		if (task && task->state == TASK_STATE_READY) run_task(task);
	}

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

	os_task_create();

	os_start();   //Linux start_kernel. start the scheduler to pick task to run one by one
	
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
