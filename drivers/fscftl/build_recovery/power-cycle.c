#include "power.h"

// TODO:: MOve to new file bootblk/bootblk.c bootblk.h
bootblk_recovery_primary_page();
bootblk_flush_primary_page(power_down_flag flag);
bootblk_flush_bbt();
bootblk_recovery_bbt();

int rebuild_systbl(void)
{
    // L2->L1

    // bmitbl

    // vpctbl

    // open blk ftllog
    
    // l2ptbl
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
    u32 power_flag;

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

