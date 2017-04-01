#ifndef __CONTROLLER_REGS_H__
#define __CONTROLLER_REGS_H__

#include "../../nvme/host/nvme.h"

#define NSC_SCRAMBLE		0x409d4 
#define NVC_SCRAMBLE		0x02110
#define NSC_SET_BIT_COUNT       0x43008
#define ECC_ERR_CNT_REG		0x40A10   //ECC error cnt enable register 

#define ENABLE_ECC_ERR_CNT 	(1 << 1)
#define ENABLE_SET_BIT_CNT 	(1 << 0)

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

void ctrl_register_config(struct nvme_ctrl * ctrl);

#endif
