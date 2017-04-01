#include "regrw.h"
#include "../../nvme/host/nvme.h"


// set HW controller, Refernce Controller Vendor Program Guide  
// Actually this should do in Firmware
void ctrl_register_config(struct nvme_ctrl *ctrl)
{
	u32 regval;

	regval = reg_read32(ctrl, NVC_SCRAMBLE);
	reg_write32(ctrl, NVC_SCRAMBLE, regval | 1);

	/* when how many bit is set, it's a empty page */
	reg_write32(ctrl, NSC_SET_BIT_COUNT, 0xfa0);

	/* metadata first u32 is used by HW report bit_err_cnt */
	regval = reg_read32(ctrl, ECC_ERR_CNT_REG);
	reg_write32(ctrl, ECC_ERR_CNT_REG, regval | ENABLE_ECC_ERR_CNT);
}
