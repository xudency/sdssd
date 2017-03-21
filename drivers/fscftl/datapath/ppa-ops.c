#include "../fscftl.h"
#include "ppa-ops.h"

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

	
	dma_unmap_single(&exdev->pdev->dev, dma_meta, dma_meta, DMA_FROM_DEVICE);

	if (nr_ppas > 1)
		nvm_exdev_dma_pool_free(exdev, ppalist, dma_ppalist);

	return 0;
}

///////////////////////////////////////////////////////////////
void erspps_smoke_test(struct nvm_exdev *dev)
{
	struct ppa_addr nandppa[2];

	nandppa[0].ppa = 0x3;
	nandppa[0].ppa = 0x4;

	printk("%s\n", __FUNCTION__);
	
	nvm_ersppa_sync(dev, &nandppa[0], 1);
	nvm_ersppa_sync(dev, &nandppa[1], 1);
}

void rdpparaw_smoke_test(struct nvm_exdev * exdev)
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

void run_testcase(struct nvm_exdev *exdev)
{
	erspps_smoke_test(exdev);
	rdpparaw_smoke_test(exdev);

	return;
}
