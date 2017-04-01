#include "bootblk_mngr.h"
#include "../fscftl.h"
#include "../datapath/ppa-ops.h"

struct bootblk_bbt_page *boot_bbt_page_info;
struct bootblk_meta_page *boot_meta_page_info;

int bootblk_page_init(void)
{
	int i;
	
	BUILD_BUG_ON(sizeof(struct bootblk_bbt_page) > BOOTBLK_PORTION_SIZE);
	BUILD_BUG_ON(sizeof(struct bootblk_meta_page) > BOOTBLK_PORTION_SIZE);

	// bbtpage
	boot_bbt_page_info = kzalloc(BOOTBLK_PORTION_SIZE, GFP_KERNEL);
	if (!boot_bbt_page_info) 
		return -ENOMEM;

	boot_bbt_page_info->magic_dw[0] = BOOTBLK_BBT_MDW0;	
	boot_bbt_page_info->magic_dw[1] = BOOTBLK_BBT_MDW1;
	boot_bbt_page_info->magic_dw[2] = BOOTBLK_BBT_MDW2;
	boot_bbt_page_info->magic_dw[3] = BOOTBLK_BBT_MDW3;

	// metapage
	boot_meta_page_info = kzalloc(BOOTBLK_PORTION_SIZE, GFP_KERNEL);
	if (!boot_meta_page_info)
		goto free_bbt_page;

	boot_meta_page_info->magic_dw[0] = BOOTBLK_META_MDW0;	
	boot_meta_page_info->magic_dw[1] = BOOTBLK_META_MDW1;
	boot_meta_page_info->magic_dw[2] = BOOTBLK_META_MDW2;
	boot_meta_page_info->magic_dw[3] = BOOTBLK_META_MDW3;
	boot_meta_page_info->bbt_page_address.ppa = INVALID_PAGE;

	for (i = 0; i < 4; i++) {
		boot_meta_page_info->bbt_ch_iter[i] = i;	
		boot_meta_page_info->meta_ch_iter[i] = i + 4;
	}

	return 0;
	
free_bbt_page:
	kfree(boot_bbt_page_info);
	return -ENOMEM;
}

void bootblk_page_exit(void)
{
	kfree(boot_meta_page_info);
	kfree(boot_bbt_page_info);
}

/* How identify a badblock, pls reference NandFlash datasheet */
static void bootblk_flush_completion(struct request *req, int error)
{
	struct nvme_command *cmd = nvme_req(req)->cmd;
	struct nvme_ppa_iod *ppa_iod = \
		ppacmd_to_pdu((struct nvme_ppa_command *)cmd);

	struct nvm_exdev *dev = ppa_iod->dev;
	u16 lun = ppa_iod->idx;

	TRACE_TAG("lun%d flush complete", lun);
	//if (ch_jmp) {
	//	erase last channel
	//}
	
	dma_pool_page_free(dev, ppa_iod->vaddr_meta, ppa_iod->dma_meta);
	dma_pool_page_free(dev, ppa_iod->vaddr_ppalist, ppa_iod->dma_ppalist);
	
	kfree(cmd);
	blk_mq_free_request(req);
}

u8 bootblk_bbt_channel_jmp(u8 last)
{
	int i;
	u8 curr;
	
	for (i = 0; i < 4; i++) {
		u8 ch = boot_meta_page_info->bbt_ch_iter[i];

		if (last == ch)
			break;
	}

	BUG_ON(i == 4);

	curr = boot_meta_page_info->bbt_ch_iter[(i+1)%4];

	TRACE_TAG("bbtpage channel jump %d->%d", last, curr);

	return curr;
}

geo_ppa bootblk_get_next_bbt_pos(void)
{
	bool carry;
	u8 channel;
	geo_ppa curpos;

	curpos = bootblk_get_bbt_pos();
	if (curpos.ppa == INVALID_PAGE) {
		curpos.ppa = 0;
		curpos.nand.ch = boot_meta_page_info->bbt_ch_iter[0];
		curpos.nand.pg = 0;
	} else {
		INCRE_BOUNDED(curpos.nand.pg, PG_BITS, carry)
		if (carry) {
			carry = 0;
			channel = bootblk_bbt_channel_jmp(curpos.nand.ch);
			curpos.nand.ch = channel;
		} else {
			/* do nothing */
		}
	}

	boot_meta_page_info->bbt_page_address = curpos;

	return curpos;
}

int bootblk_recovery_meta_page(void)
{
	return 0;
}

void bootblk_flush_meta_page(enum power_down_flag flag)
{
	return;
}

/* update boot_bbtpage from bmi->bbt then flush the latest bbt info to bootblk */
void bootblk_flush_bbt(struct nvm_exdev *dev)
{
	int i, sec, pl, blk, lun;
	geo_ppa bbt_pos;
	u64 *ppalist;
	void *metabuf;
	dma_addr_t ppa_dma, meta_dma;
	int nr_ppas = CFG_DRIVE_LINE_NUM;	
	u16 ctrl = NVM_IO_DUAL_ACCESS | NVM_IO_SCRAMBLE_ENABLE;

	/* update bbtpage from BMI */
	for_each_blk(blk) {
		for_each_lun(lun) {
			struct bmi_item *bmi = get_bmi_item(blk);
			boot_bbt_page_info->bbt[blk][lun] = bmi->bbt[lun];
		}
	}
	
	bbt_pos.ppa = 0;
	bbt_pos = bootblk_get_next_bbt_pos();
	
	for_each_lun(lun) {
		struct nvme_ppa_command *ppa_cmd;
		struct nvme_ppa_iod *ppa_iod;
			
		ppa_cmd = alloc_ppa_rqd_ctx();
		ppa_iod = ppacmd_to_pdu(ppa_cmd);

		ppalist = dma_pool_page_zalloc(dev, &ppa_dma);

		i = 0;
		for_each_pl(pl) {
			for_each_sec(sec) {
				geo_ppa ppa;
				set_ppa_nand_addr(&ppa, bbt_pos.nand.ch, sec, 
						pl, lun, bbt_pos.nand.pg, 0);
				ppalist[i++] = ppa.ppa;
			}
		}

		WARN_ON(i != nr_ppas);

		metabuf = dma_pool_page_zalloc(dev, &meta_dma);
		
		ppa_iod->dev = dev;
		ppa_iod->vaddr_meta = metabuf;
		ppa_iod->dma_meta = meta_dma;
		ppa_iod->vaddr_ppalist = ppalist;
		ppa_iod->dma_ppalist = ppa_dma;
		ppa_iod->idx = lun;
		
		ppa_cmd->opcode = NVM_OP_WRPPA;
		ppa_cmd->nsid = dev->bns->ns_id;
		ppa_cmd->nlb = cpu_to_le16(nr_ppas - 1);
		ppa_cmd->control = cpu_to_le16(ctrl);
		ppa_cmd->ppalist = cpu_to_le64(ppa_dma);
		ppa_cmd->metadata = cpu_to_le64(meta_dma);

		nvme_submit_ppa_cmd(dev, ppa_cmd, boot_bbt_page_info, 
				    BOOTBLK_PORTION_SIZE, 
				    bootblk_flush_completion, NULL);
	}

	TRACE_TAG("on ch:%d page%d", bbt_pos.nand.ch, bbt_pos.nand.pg);
}

void bootblk_recovery_bbt(void)
{
	return;
}

