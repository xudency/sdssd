#ifndef __CONTROLLER_REGS_H__
#define __CONTROLLER_REGS_H__

#include "../../nvme/host/nvme.h"

#define NSC_SCRAMBLING      0x409d4 


static inline u32 reg_read32(struct nvme_ctrl *ctrl, u32 regaddr)
{
    u32 regval;
    
	ctrl->ops->reg_read32(ctrl, regaddr, &regval);

    return regval;
}

static inline void reg_write32(struct nvme_ctrl *ctrl, u32 regaddr, u32 regval)
{
	ctrl->ops->reg_write32(ctrl, regaddr, regval);
}

void ctrl_reg_setup(struct nvme_ctrl * ctrl);

#endif
