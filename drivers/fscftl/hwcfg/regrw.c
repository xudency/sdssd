#include "regrw.h"
#include "../../nvme/host/nvme.h"


// set HW controller, Refernce Controller Vendor Program Guide  
// Actually this should do in Firmware
void ctrl_reg_setup(struct nvme_ctrl *ctrl)
{
	u32 regval;

	regval = reg_read32(ctrl, 0x00);

	printk("regval:0x%x\n", regval);

	//reg_write32(ctrl, regaddr, regval);
}
