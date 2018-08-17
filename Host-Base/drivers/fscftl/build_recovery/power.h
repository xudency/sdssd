#ifndef _FSCFTL_POWER_H_
#define _FSCFTL_POWER_H_

#include "../../nvme/host/nvme.h"

enum power_down_flag {
    POWER_DOWN_SAFE   = 0x8866,      // last time power down success
    POWER_DOWN_UNSAFE = 0x7755,      // last time not do power down, need do crash recovery
};

static inline void print_pdf(enum power_down_flag flag)
{
	if (flag == POWER_DOWN_SAFE)
		printk("POWER_DOWN_SAFE\n");
	else if (flag == POWER_DOWN_UNSAFE)
		printk("POWER_DOWN_UNSAFE\n");
	else
		printk("POWER_DOWN FLAG Invalid\n");
}

int crash_recovery(struct nvm_exdev * exdev);
int do_manufactory_init(struct nvm_exdev * exdev);
int try_recovery_systbl(struct nvm_exdev * exdev);
void flush_down_systbl(struct nvm_exdev * exdev);

#endif

