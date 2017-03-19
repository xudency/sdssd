#ifndef _FSCFTL_POWER_H_
#define _FSCFTL_POWER_H_
#include "../hwcfg/cfg/flash_cfg.h"
#include "../../nvme/host/nvme.h"

enum power_down_flag {
    POWER_DOWN_SAFE   = 0x8866,      // last time power down success
    POWER_DOWN_UNSAFE = 0x7755,      // last time not do power down, need do crash recovery
};

int crash_recovery(void);
int do_manufactory_init(struct nvm_exdev * exdev);
int try_recovery_systbl(void);
void flush_down_systbl(void);

#endif
