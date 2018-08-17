/*
 * FW main to init pherial, power on
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>

static int __init fw_init(void)
{
	ddr_init();

	sram_init();

	nandflash_init();

	cpu_init();

	hw_init();

	printk("FW start run ...\n");
	
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
