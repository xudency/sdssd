#include "power.h"
#include "../bootblk/bootblk_mngr.h"

int rebuild_systbl(void)
{
    // L2->L1

    // bmitbl

    // vpctbl

    // open blk ftllog
    
    // l2ptbl

	return 0;
}

void flush_down_systbl(void)
{
    // l2ptbl

    // bmitbl

    // L1
    
    // openblk ftllog

    // 8 padding page of dummy data to stable the User data
    
    bootblk_flush_bbt();
    bootblk_flush_primary_page(POWER_DOWN_SAFE);

    return;
}

// return 0: rebuild systbl success, or rebuild fail
int try_recovery_systbl(void)
{
    int result;
    enum power_down_flag power_flag = POWER_DOWN_UNSAFE;

    result = bootblk_recovery_primary_page();
    if (unlikely(result)) {
        printk("Don't find primary page in bootblk\n");
        printk("do your sure you has do manufactory?\n");
        return 1;
    }
    
    if (power_flag == POWER_DOWN_SAFE) {
        result = rebuild_systbl();
    } else if (power_flag == POWER_DOWN_UNSAFE) {
        result = crash_recovery();
    } else {
        printk("primary page power_flag:%d invalid\n", power_flag);
    }

    bootblk_flush_bbt();
    bootblk_flush_primary_page(POWER_DOWN_UNSAFE);

    return result;
}

