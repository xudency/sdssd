#ifndef _BOOTBLK_MNGR_
#define _BOOTBLK_MNGR_

#include "../build_recovery/power.h"

// TODO:: MOve to new file bootblk/bootblk.c bootblk.h
int bootblk_recovery_primary_page(void);
void bootblk_flush_primary_page(enum power_down_flag flag);
void bootblk_flush_bbt(void);
void bootblk_recovery_bbt(void);

#endif
