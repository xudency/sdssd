#include "power.h"
#include "../bootblk/bootblk_mngr.h"

int rebuild_systbl(void)
{
	printk("start %s\n", __FUNCTION__);

	// L2->L1

	// bmitbl

	// vpctbl

	// open blk ftllog

	// l2ptbl
	printk("complete %s\n", __FUNCTION__);

	return 0;
}

void flush_down_systbl(void)
{
	printk("start %s\n", __FUNCTION__);

	// l2ptbl

	// bmitbl

	// L1

	// openblk ftllog

	// 8 padding page of dummy data to stable the User data

	bootblk_flush_bbt();
	bootblk_flush_meta_page(POWER_DOWN_SAFE);

	printk("complete %s\n", __FUNCTION__);

	return;
}

// return 0: rebuild systbl success, or rebuild fail
int try_recovery_systbl(void)
{
	int result;
	enum power_down_flag power_flag = POWER_DOWN_SAFE;

	printk("start %s\n", __FUNCTION__);

	result = bootblk_recovery_meta_page();
	if (unlikely(result)) {
		printk("Don't find primary page in bootblk\n");
		printk("do your sure you has did manufactory?\n");
		return 1;
	}

	print_pdf(power_flag);

	if (power_flag == POWER_DOWN_SAFE) {
		result = rebuild_systbl();
	} else if (power_flag == POWER_DOWN_UNSAFE) {
		result = crash_recovery();
	} else {
		printk("primary page power_flag:%d invalid\n", power_flag);
		return 1;
	}

	bootblk_flush_bbt();
	bootblk_flush_meta_page(POWER_DOWN_UNSAFE);

	printk("complete %s\n", __FUNCTION__);

	return result;
}

