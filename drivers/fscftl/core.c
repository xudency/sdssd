#include "fscftl.h"

static void print_cqe_result(struct nvm_rq *rqd)
{
	printk("New CQE complete\n");

	switch (rqd->opcode) {
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
	}

	printk("  nr_ppas:%d  control:%x\n", rqd->nr_ppas, rqd->flags);
	printk("  status:0x%x result.u64:0x%llx\n", rqd->error, rqd->ppa_status);
	printk("\n");
}

int nvm_exdev_setup_pool(struct nvm_exdev *dev, char *name)
{
	struct device *dmadev = &dev->pdev->dev;

	dev->dmapoll = dma_pool_create(name, dmadev, PAGE_SIZE, PAGE_SIZE, 0);
	if(!dev->dmapoll)
		return -ENOMEM;
	else
		return 0;
}

void nvm_exdev_release_pool(struct nvm_exdev *dev)
{
	dma_pool_destroy(dev->dmapoll);
}

void *nvm_exdev_dma_pool_alloc(struct nvm_exdev *dev, dma_addr_t *dma_handle)
{
	return dma_pool_alloc(dev->dmapoll, GFP_KERNEL, dma_handle);
}

void nvm_exdev_dma_pool_free(struct nvm_exdev *dev, void *vaddr, dma_addr_t dma_handle)
{
	dma_pool_free(dev->dmapoll, vaddr, dma_handle);
}

int set_rqd_nand_ppalist(struct nvm_exdev *dev, struct nvm_rq *rqd, 
						 struct ppa_addr *ppas, int nr_ppas)
{
	int i;

	rqd->nr_ppas = nr_ppas;

	if (nr_ppas == 1) {
		rqd->ppa_addr = ppas[0];
		return 0;
	}
	
	rqd->ppa_list = nvm_exdev_dma_pool_alloc(dev, &rqd->dma_ppa_list);
	if (!rqd->ppa_list) {
		pr_err("nvm: failed to allocate dma memory\n");
		return -ENOMEM;
	} else {
		for (i = 0; i < nr_ppas; i++)
			rqd->ppa_list[i] = ppas[i];
	}

	return 0;
}

void free_rqd_nand_ppalist(struct nvm_exdev * dev, struct nvm_rq *rqd)
{
	if (!rqd->ppa_list)   //nr_ppa
		return;

	nvm_exdev_dma_pool_free(dev, rqd->ppa_list, rqd->dma_ppa_list);
}

static inline void nvm_rqd_to_ppacmd(struct nvm_rq *rqd, int instance, 
									 struct nvme_ppa_command *c)
{
	c->opcode = rqd->opcode;
	c->nsid = cpu_to_le32(instance);
	c->ppalist = cpu_to_le64(rqd->ppa_addr.ppa);
	c->metadata = cpu_to_le64(rqd->dma_meta_list);
	c->control = cpu_to_le16(rqd->flags);
	c->nlb = cpu_to_le16(rqd->nr_ppas - 1);

	/* prp1 prp2 is in bio */
}

static void nvm_ppa_end_io(struct request *rq, int error)
{
	struct nvm_rq *rqd = rq->end_io_data;

	rqd->ppa_status = nvme_req(rq)->result.u64;
	rqd->error = error;

	print_cqe_result(rqd);

	//callback fn(ctx)
	if (rqd->end_io)
		rqd->end_io(rqd);

	kfree(nvme_req(rq)->cmd);
	blk_mq_free_request(rq);
}

/*
 * We submit NVMe command to NVMe Device Drive hw_queue,
 * this queue is Under blk-mq, It has bind with cpu_num blk-mq-tagset
 */
static int nvm_submit_ppa(struct nvm_exdev *exdev, struct nvm_rq *rqd)
{
	struct nvme_ns *bns = exdev->bns;
	struct request_queue *bq = bns->queue;
	struct request *rq;
	struct bio *bio = rqd->bio;
	struct nvme_ppa_command *cmd;

	cmd = kzalloc(sizeof(struct nvme_ppa_command), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	rq = nvme_alloc_request(bq, (struct nvme_command *)cmd, 0, NVME_QID_ANY);
	if (IS_ERR(rq)) {
		kfree(cmd);
		return -ENOMEM;
	}
	rq->cmd_flags &= ~REQ_FAILFAST_DRIVER;

	if (bio) {
        // write/read need form a bio
		rq->ioprio = bio_prio(bio);
		rq->__data_len = bio->bi_iter.bi_size;
		rq->bio = rq->biotail = bio;
		if (bio_has_data(bio))
			rq->nr_phys_segments = bio_phys_segments(bq, bio);
	} else {
		// erase
		rq->ioprio = IOPRIO_PRIO_VALUE(IOPRIO_CLASS_BE, IOPRIO_NORM);
		rq->__data_len = 0;
	}

	nvm_rqd_to_ppacmd(rqd, bns->instance, cmd);

	rq->end_io_data = rqd;

	blk_execute_rq_nowait(bq, NULL, rq, 0, nvm_ppa_end_io);

	return 0;
}

/* inherit interface from Lightnvm Subsystem and pblk */
struct nvme_ppa_ops exdev_ppa_ops = {
	.name			 = "expssd",
	.module			 = THIS_MODULE,
	.submit_io       = nvm_submit_ppa,
};


/***************************************************************************
 *						    Divide										   *
 ***************************************************************************/

/*
 * Returns 0 on success.  If the result is negative, it's a Linux error code;
 * if the result is positive, it's an NVM Express status code
 */
static void nvme_ppa_completion(struct request *req, int error)
{
	void *ctx = req->end_io_data;
	int status = error;						/* No phase tag */
	u64 result = nvme_req(req)->result.u64; /* 64bit completion btmap */
	struct nvme_command *cmd = nvme_req(req)->cmd;	/* original sqe */

	// goto free/release some source

	blk_mq_free_request(req);
}

static int __nvme_submit_ppa_cmd(struct request_queue *q, 
		struct nvme_ppa_command *cmd, union nvme_result *result, 
		void *buffer, unsigned bufflen, unsigned timeout, 
		int qid, int at_head, int flags, 
		rq_end_io_fn *done, void *ctx)
{
	struct request *req;
	int ret;

	req = nvme_alloc_request(q, (struct nvme_command *)cmd, flags, qid);
	if (IS_ERR(req))
		return PTR_ERR(req);

	req->timeout = timeout ? timeout : NVME_IO_TIMEOUT;

	if (buffer && bufflen) {
		ret = blk_rq_map_kern(q, req, buffer, bufflen, GFP_KERNEL);
		if (ret)
			goto out;
	}

	req->end_io_data = ctx;
	
	blk_execute_rq_nowait(q, NULL, req, 0, done);

	return 0;
 out:
	blk_mq_free_request(req);
	return ret;
}

/*
 * this method will bypass dev->ops.submit_io
 * so we don't need prepare a nvm_rq rqd
 * before call this function, Caller should Guarantee:
 *		1. nvme_ppa_command is ready
 *      2. ppalist metadata DMA buffer is allocated and set in cmd
 *      3. databuff is dma_map inside this fn, so we don't need dma_map it
 */
int nvme_submit_ppa_cmd(struct nvm_exdev *dev, struct nvme_ppa_command *cmd,
						void *buffer, unsigned bufflen, 
						rq_end_io_fn *done, void *ctx)
{
	struct request_queue *q = dev->bns->queue;

	return __nvme_submit_ppa_cmd(q, cmd, NULL, buffer, bufflen, 0,
								 NVME_QID_ANY, 0, 0, done, ctx);
}

