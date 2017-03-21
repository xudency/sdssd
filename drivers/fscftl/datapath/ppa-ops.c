#include "../fscftl.h"
#include "ppa-ops.h"
#include "../writecache/wcb-mngr.h"

/*********************************************************************
 * How to use submit_bio to submit PPA command
 * form a rqd
 * 		1.fullfill field of rqd.xx
 *		2.callback fn(ctx)
 * 		3.ppalist
 * 		4.metadata
 * 		5.form a bio, the R/W data should be in bio->bvecpage
 * 		6.call core io handle fn
 *
 *********************************************************************/

static void print_ppa_cqe(struct nvme_ppa_command *cmd, u64 result, int status)
{
	printk("New PPA command CQE complete\n");
	
	switch (cmd->opcode) {
	case NVM_OP_ESPPA:
		printk("  ersppa");
		break;
	case NVM_OP_WRPPA:
		printk("  wrppa");
		break;
	case NVM_OP_RDPPA:
		printk("  rdppa");
		break;
	case NVM_OP_WRRAW:
		printk("  wrpparaw");
		break;
	case NVM_OP_RDRAW:
		printk("  rdpparaw");
		break;
	default:
		printk("  opcode invalid!!!");
	}

	printk("  nr_ppas:%d  control:%x\n", (cmd->nlb+1), cmd->control);
	printk("  status:0x%x result.u64:0x%llx\n", status, result);
	printk("\n");
}

/*
 * Returns 0 on success.  If the result is negative, it's a Linux error code;
 * if the result is positive, it's an NVM Express status code
 */
static void nvme_ppa_completion(struct request *req, int error)
{
	//void *ctx = req->end_io_data;
	int status = error;						/* No phase tag */
	u64 result = nvme_req(req)->result.u64; /* 64bit completion btmap */
	struct nvme_command *cmd = nvme_req(req)->cmd;	/* original sqe */

	// goto free/release some source
	print_ppa_cqe((struct nvme_ppa_command *)cmd, result, status);

	kfree(nvme_req(req)->cmd);
	blk_mq_free_request(req);
}

static void nvm_end_io_sync(struct nvm_rq *rqd)
{
	struct completion *waiting = rqd->private;

	complete(waiting);
}

int nvm_ersppa_sync(struct nvm_exdev *dev, struct ppa_addr *ppas, int nr_ppas)
{
	int ret = 0;
	struct nvm_rq rqd;

	DECLARE_COMPLETION_ONSTACK(wait);

	memset(&rqd, 0, sizeof(struct nvm_rq));

    // 1.fullfill field of rqd.xx
    rqd.opcode = NVM_OP_ERASE;
	rqd.flags = NVM_IO_SNGL_ACCESS;

    // 2.callback fn(ctx)
	rqd.end_io = nvm_end_io_sync;
	rqd.private = &wait;

    // 3.ppalist
	ret = set_rqd_nand_ppalist(dev, &rqd, ppas, nr_ppas);
	if (ret)
		return ret;

    // 4.metadata

    // 5.form a bio

    // 6.call core io handle fn
    ret = dev->ops->submit_io(dev, &rqd);  //nvm_submit_ppa
	if (ret) {
		pr_err("erase I/O submission falied: %d\n", ret);
		goto free_ppa_list;
	}

	wait_for_completion_io(&wait);

free_ppa_list:
	free_rqd_nand_ppalist(dev, &rqd);
    return ret;
}

int nvm_rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
					  int nr_ppas, u16 ctrl, void *databuf, void *metabuf)
{
	int i;
	u64 *ppalist = NULL;
	dma_addr_t dma_ppalist, dma_meta;
	struct nvme_ppa_command ppa_cmd;
	struct request_queue *q = exdev->bns->queue;

	memset(&ppa_cmd, 0x00, sizeof(ppa_cmd));

	// set ppalist
	if (nr_ppas == 1) {
		ppa_cmd.ppalist = cpu_to_le64(ppa[0].ppa); 
	} else {
		ppalist = nvm_exdev_dma_pool_alloc(exdev, &dma_ppalist);
		if (!ppalist) {
			pr_err("nvm: failed to allocate dma memory\n");
			return -ENOMEM;
		}

		for (i = 0; i < nr_ppas; i++)
			ppalist[i] = (u64)ppa[i].ppa;

		ppa_cmd.ppalist = cpu_to_le64(dma_ppalist); 
	}

	// set metadata
	dma_meta = dma_map_single(&exdev->pdev->dev, metabuf, 
							  nr_ppas * NAND_RAW_SIZE, DMA_FROM_DEVICE);
	
	ppa_cmd.opcode = NVM_OP_RDRAW;
	ppa_cmd.nsid = exdev->bns->ns_id;
	ppa_cmd.metadata = cpu_to_le64(dma_meta);
	ppa_cmd.nlb = cpu_to_le16(nr_ppas - 1);
	ppa_cmd.control = cpu_to_le16(ctrl);

	nvme_submit_sync_cmd(q, (struct nvme_command *)&ppa_cmd,
						 databuf, nr_ppas * CFG_NAND_EP_SIZE);

	dma_unmap_single(&exdev->pdev->dev, dma_meta, 
					nr_ppas * NAND_RAW_SIZE, DMA_FROM_DEVICE);

	if (nr_ppas > 1)
		nvm_exdev_dma_pool_free(exdev, ppalist, dma_ppalist);

	return 0;
}

/////////////////////////////////////////////////////////////////
// async Mode use dma_coherent
int nvm_rdpparaw(struct nvm_exdev *exdev, dmappa ppa, int nr_ppas, 
				u16 ctrl, void *databuf, dma_addr_t meta_dma)
{	
	struct nvme_ppa_command *ppa_cmd;
	
	ppa_cmd = kzalloc(sizeof(struct nvme_ppa_command), GFP_KERNEL);
	if (!ppa_cmd)
		return -ENOMEM;

	ppa_cmd->opcode = NVM_OP_RDRAW;
	ppa_cmd->nsid = exdev->bns->ns_id;
	ppa_cmd->metadata = cpu_to_le64(meta_dma);
	ppa_cmd->ppalist = ppa.dma_ppa_list;
	ppa_cmd->nlb = cpu_to_le16(nr_ppas - 1);
	ppa_cmd->control = cpu_to_le16(ctrl);

	nvme_submit_ppa_cmd(exdev, ppa_cmd, databuf, nr_ppas * CFG_NAND_EP_SIZE, 
						nvme_ppa_completion, NULL);

	// in IRQ callkack completion to release

	return 0;
}

void erspps_smoke_test(struct nvm_exdev *dev)
{
	struct ppa_addr nandppa[2];

	nandppa[0].ppa = 0x3;
	nandppa[0].ppa = 0x4;

	printk("%s\n", __FUNCTION__);
	
	nvm_ersppa_sync(dev, &nandppa[0], 1);
	nvm_ersppa_sync(dev, &nandppa[1], 1);
}

void rdpparawsync_smoke_test(struct nvm_exdev * exdev)
{
	struct physical_address ppa[2];
	void *databuf, *metabuf;
	int *tmp;

	ppa[0].ppa = 0x100;	
	ppa[1].ppa = 0x103;

	printk("%s\n", __FUNCTION__);

	databuf = kzalloc((CFG_NAND_EP_SIZE + NAND_RAW_SIZE) * 2, GFP_KERNEL);
	metabuf = databuf + CFG_NAND_EP_SIZE * 2;

	nvm_rdpparaw_sync(exdev, ppa, 2, NVM_IO_SNGL_ACCESS, databuf, metabuf);

	tmp = (int *)databuf;
	printk("0x%x 0x%x 0x%x 0x%x\n", tmp[0], tmp[1], tmp[2], tmp[3]);

	kfree(databuf);
}

void rdpparraw_smoke_test(struct nvm_exdev * exdev)
{
	//g_ppalist_buf[0] = 0x13;
	int nr_ppas=1;
	dma_addr_t dma_meta;
	dmappa ppa;
	void *databuf = wcb_entity_base_data(0);
	void *metabuf = wcb_entity_base_meta(0);
	ppa.nandppa = 0x12;

	dma_meta = dma_map_single(&exdev->pdev->dev, metabuf, 
							  nr_ppas * NAND_RAW_SIZE, DMA_FROM_DEVICE);

	nvm_rdpparaw(exdev, ppa, nr_ppas, 0, databuf, dma_meta);
}

void run_testcase(struct nvm_exdev *exdev)
{
	rdpparraw_smoke_test(exdev);

	//erspps_smoke_test(exdev);
	//rdpparawsync_smoke_test(exdev);

	return;
}

