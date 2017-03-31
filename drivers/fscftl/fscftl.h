#ifndef FSCFTL_H_
#define FSCFTL_H_

#include "../../include/linux/lightnvm.h"
#include "../nvme/host/nvme.h"
#include "hwcfg/cfg/flash_cfg.h"
#include "bootblk/bootblk_mngr.h"
#include "systbl/sys-meta.h"
#include "utils/utils.h"

#define SEC_PER_PPA 8 // 4K/512
#define EXP_PPA_SIZE (4096)

//don't define fix value, should read from HW register
#define MAX_PPA_PER_CMD		64   // due to cqe 64 bit

#define NAND_RAW_SIZE 		304
#define NAND_META_SIZE 		16

// io opcode
enum {
	NVM_OP_WRPPA		= 0x91,
	NVM_OP_RDPPA		= 0x92,
	NVM_OP_ESPPA		= 0x90,
	NVM_OP_WRRAW		= 0x95,
	NVM_OP_RDRAW		= 0x96,
};

// ppa sqe
struct nvme_ppa_command {
	__u8			opcode;
	__u8			flags;
	__u16			command_id;
	__le32			nsid;
	__u64			rsvd2;
	__le64			metadata;
	__le64			prp1;
	__le64			prp2;
	__le64			ppalist;
	__le16			nlb;
	__le16			control;
	__le32			dsmgmt;
	__le64			resv;
};

struct nvme_ppa_iod {
	void *ctx;	// lun entity
	struct nvm_exdev *dev;
	
	void *vaddr_data;
	dma_addr_t dma_data;

	void *vaddr_meta;
	dma_addr_t dma_meta;

	u64 *vaddr_ppalist;
	dma_addr_t dma_ppalist;
	int idx;   // which line or which ch
};

static inline sector_t get_bio_slba(struct bio *bio)
{
	return bio->bi_iter.bi_sector / SEC_PER_PPA;
}

static inline unsigned int get_bio_nppa(struct bio *bio)
{
	return  bio->bi_iter.bi_size / EXP_PPA_SIZE;
}

/** |nvme_ppa_command | nvme_ppa_iod| **/
static inline struct nvme_ppa_command *alloc_ppa_rqd_ctx(void)
{
	return kzalloc(sizeof(struct nvme_ppa_command) + \
			  sizeof(struct nvme_ppa_iod), GFP_KERNEL);
}

static inline void *ppacmd_to_pdu(struct nvme_ppa_command *ppa_cmd)
{
	return ppa_cmd + 1;
}

/* extern fn */
blk_qc_t fscftl_make_rq(struct request_queue *q, struct bio *bio);

int nvm_create_exns(struct nvm_exdev *exdev);
void nvm_delete_exns(struct nvm_exdev *exdev);

void free_rqd_nand_ppalist(struct nvm_exdev * dev, struct nvm_rq *rqd);
int set_rqd_nand_ppalist(struct nvm_exdev *dev, struct nvm_rq *rqd, 
			 struct ppa_addr *ppas, int nr_ppas);

int nvm_exdev_setup_pool(struct nvm_exdev *dev, char *name);
void nvm_exdev_release_pool(struct nvm_exdev *dev);
void *dma_pool_page_zalloc(struct nvm_exdev *dev, dma_addr_t *dma_handle);
void dma_pool_page_free(struct nvm_exdev *dev, void *vaddr, 
			dma_addr_t dma_handle);

int nvme_submit_ppa_cmd(struct nvm_exdev *dev, 
                        struct nvme_ppa_command *cmd,
			void *buffer, unsigned bufflen, 
			rq_end_io_fn *done, void *ctx);

int nvme_submit_ppa_cmd_sync(struct nvm_exdev *dev, 
                             struct nvme_ppa_command *cmd,
			     void *buffer, unsigned bufflen);

#endif
