#include "power.h"
#include "../fscftl.h"
#include "../datapath/ppa-ops.h"

static struct completion bbt_completion;
static atomic_t bbt_issu_cnt = ATOMIC_INIT(0);
static atomic_t bbt_cmpl_cnt = ATOMIC_INIT(0);

// How identify a badblock, pls reference NandFlash datasheet
static void rdpparaw_completion(struct request *req, int error)
{
	int i;
	u32 offt = EXP_PPA_SIZE - 3 * NAND_RAW_SIZE; 
	struct nvme_ppa_iod *ppa_iod = req->end_io_data;
	struct nvm_exdev *dev = ppa_iod->dev;
	/*struct nvme_ppa_command *cmd = \
		(struct nvme_ppa_command *)nvme_req(req)->cmd;*/

	/* if any one plane is bb, we regard all these planes all bb */
	for (i = 0; i < CFG_NAND_PLANE_NUM; i++) {
		u8 *databuf = (u8 *)(ppa_iod->vaddr_data + i * EXP_PPA_SIZE);
		if (databuf[offt] != 0xff) {
			geo_ppa ppa;
			// add sanity check
			// all ppa in vaddr_ppalist is the same(ch lun blk)
			ppa.ppa = (u32)ppa_iod->vaddr_ppalist[0];
			mark_bbt_tbl(ppa.nand.blk, ppa.nand.lun, 
				     ppa.nand.ch, BADB);
			break;
		}
	}
	
	nvm_exdev_dma_pool_free(dev, ppa_iod->vaddr_ppalist, 
				ppa_iod->dma_ppalist);
	
	kfree(nvme_req(req)->cmd);
	blk_mq_free_request(req);

	if (atomic_inc_return(&bbt_cmpl_cnt) == atomic_read(&bbt_issu_cnt))
		complete(&bbt_completion);
}

// read @ppa correspond PL ppas address
// check if the @ppa is badblock or good block
int discovery_bbt_rdpparaw(struct nvm_exdev *exdev, geo_ppa ppa, 
			    void *databuf, dma_addr_t metadma)
{
	int i;
	u64 *ppalist;
	dma_addr_t ppa_dma;
	u16 nr_ppas = CFG_NAND_PLANE_NUM;
	struct nvme_ppa_command *ppa_cmd;
	struct nvme_ppa_iod *ppa_iod;

	ppa_cmd = kzalloc(sizeof(struct nvme_ppa_command) + \
			  sizeof(struct nvme_ppa_iod), GFP_KERNEL);
	if (!ppa_cmd)
		return -ENOMEM;
	
	ppa_iod = (struct nvme_ppa_iod *)((uintptr_t)ppa_cmd + \
				sizeof(struct nvme_ppa_command));

	ppalist = nvm_exdev_dma_pool_alloc(exdev, &ppa_dma);
	if (!ppalist)
		goto free_cmd;
	
	for (i = 0; i < nr_ppas; i++) {
		ppa.nand.pl = i;
		ppalist[i++] = ppa.ppa;
	}

	ppa_iod->dev = exdev;
	ppa_iod->vaddr_ppalist = ppalist;
	ppa_iod->dma_ppalist = ppa_dma;
	ppa_iod->vaddr_data = databuf;

	ppa_cmd->opcode = NVM_OP_RDRAW;
	ppa_cmd->nsid = exdev->bns->ns_id;
	ppa_cmd->metadata = cpu_to_le64(metadma);
	ppa_cmd->nlb = cpu_to_le16(nr_ppas - 1);
	ppa_cmd->ppalist = cpu_to_le64(ppa_dma);
	ppa_cmd->control = cpu_to_le16(NVM_IO_SNGL_ACCESS);

	nvme_submit_ppa_cmd(exdev, ppa_cmd, databuf, EXP_PPA_SIZE*nr_ppas, 
			    rdpparaw_completion, ppa_iod);

	return 0;

free_cmd:
	kfree(ppa_cmd);
	return -ENOMEM;
}

// Micron page=0 ep=3
void bbt_rblk_discovery(struct nvm_exdev * exdev, u16 blk, 
			void *gdatabuf, dma_addr_t gmetadma)
{
	u16 ch, lun;
	void *databuf;
	dma_addr_t metadma;

	for (lun = 0; lun < CFG_NAND_LUN_NUM; lun++) {
		for (ch = 0; ch < CFG_NAND_CHANNEL_NUM; ch++) {
			geo_ppa ppa;
			set_ppa_nand_addr(&ppa, ch, 3, 0, lun, 0, blk);

			// travsal pl inside
			databuf = (void *)((unsigned long)gdatabuf + (ch + lun*CFG_NAND_CHANNEL_NUM)*(CFG_NAND_PLANE_NUM*EXP_PPA_SIZE));

			metadma = gmetadma + (ch + lun*CFG_NAND_CHANNEL_NUM)*(CFG_NAND_PLANE_NUM*NAND_RAW_SIZE);

			discovery_bbt_rdpparaw(exdev, ppa, databuf, metadma);
			
			atomic_inc(&bbt_issu_cnt);
		}
	}	
}

// erase one raidblk
int erase_rblk_wait(struct nvm_exdev *exdev, u16 blk)
{
        int ret = 0;
        u16 nlb = 0, ch, lun;
        u16 plmode;
        u16 nr_ppas = CFG_NAND_CHANNEL_NUM*CFG_NAND_LUN_NUM;
        dma_addr_t dma_ppalist;
        u64 *ppalist;
        geo_ppa ppa;
	struct nvme_ppa_command *ppa_cmd;
	
	ppa_cmd = kzalloc(sizeof(struct nvme_ppa_command), GFP_KERNEL);
	if (!ppa_cmd)
		return -ENOMEM;

        if (CFG_NAND_PLANE_NUM == 4) {
                plmode = NVM_IO_QUAD_ACCESS;
        } else if (CFG_NAND_PLANE_NUM == 2) {
                plmode = NVM_IO_DUAL_ACCESS;
        }

        ppalist = nvm_exdev_dma_pool_alloc(exdev, &dma_ppalist);
        if (!ppalist) {
                pr_err("nvm: failed to allocate dma memory\n");
                ret = -ENOMEM;
                goto free_cmd;
        }

        for (lun = 0; lun < CFG_NAND_LUN_NUM; lun++) {
                for (ch = 0; ch < CFG_NAND_CHANNEL_NUM; ch++) {
                        set_ppa_nand_addr(&ppa, ch, 0, 0, lun, 0, blk);
                        ppalist[nlb++] = (u64)ppa.ppa;
                }
        }

	WARN_ON(nlb != nr_ppas);

	ppa_cmd->opcode = NVM_OP_ERASE;
	ppa_cmd->nsid = exdev->bns->ns_id;
	ppa_cmd->nlb = cpu_to_le16(nr_ppas - 1);
        ppa_cmd->ppalist = cpu_to_le64(dma_ppalist);
	ppa_cmd->control = cpu_to_le16(plmode);

        nvme_submit_ppa_cmd_sync(exdev, ppa_cmd, NULL, 0);
	//nvme_submit_sync_cmd(q, (struct nvme_command *)&ppa_cmd,
                             //NULL, nr_ppas * 0);

        nvm_exdev_dma_pool_free(exdev, ppalist, dma_ppalist);
free_cmd:
        kfree(ppa_cmd);
	return ret;
}

static void sweepup_disk(struct nvm_exdev *exdev)
{
        u16 blk;

	printk("sweepup the whole disk\n");

        for (blk = 0; blk < CFG_NAND_BLOCK_NUM; blk++) {
                printk("erase raidblk:%d\n", blk);
                erase_rblk_wait(exdev, blk);
        }
}

#define DMA_ALLOC_MAX  (1024*1024*4)   // 4MB

static void fscftl_bbt_discovery(struct nvm_exdev *exdev)
{
	struct device *dmadev = &exdev->pdev->dev;
	u32 blk;
	void *databuf, *metabuf;
	dma_addr_t datadma, metadma;
	// to store a raidblk bb check data
	size_t datalen = \
	    CFG_NAND_CHANNEL_NUM*CFG_NAND_LUN_NUM*CFG_PLANE_NUM*EXP_PPA_SIZE;
	size_t metalen = \
	    CFG_NAND_CHANNEL_NUM*CFG_NAND_LUN_NUM*CFG_PLANE_NUM*NAND_RAW_SIZE;

	BUILD_BUG_ON(datalen > DMA_ALLOC_MAX);	
	BUILD_BUG_ON(metalen > DMA_ALLOC_MAX);
	
	databuf = dma_alloc_coherent(dmadev, datalen, &datadma, GFP_KERNEL);
	if (!databuf) {
		printk("%s-%d\n", __FUNCTION__, __LINE__);
		return;
	}		

	metabuf = dma_alloc_coherent(dmadev, metalen, &metadma, GFP_KERNEL);
	if (!metabuf) {
		printk("%s-%d\n", __FUNCTION__, __LINE__);
		goto free_data_dma;
	}

	for (blk = 0; blk < 1; blk++) {  //CFG_NAND_BLOCK_NUM
		init_completion(&bbt_completion);

		printk("bbt discovery raidblk:%d\n", blk);
		bbt_rblk_discovery(exdev, blk, databuf, metadma);

		// as request may not enough
		// so wait a bit prevent nvme_alloc_request failed
		wait_for_completion(&bbt_completion);
	}

	dma_free_coherent(dmadev, metalen, metabuf, metadma);
free_data_dma:
	dma_free_coherent(dmadev, datalen, databuf, datadma);
	return;
}


int do_manufactory_init(struct nvm_exdev * exdev)
{
	printk("start %s\n", __FUNCTION__);

	sweepup_disk(exdev);

	//fscftl_bbt_discovery(exdev);  //set bootblk bbt && bmi->bb

	// Now all systbl is clean, don't need Flush down

	bootblk_flush_bbt();
	bootblk_flush_primary_page(POWER_DOWN_UNSAFE);

	printk("complete %s\n", __FUNCTION__);

	return 0;
}
