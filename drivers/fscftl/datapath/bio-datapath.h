#ifndef __BIO_DATAPATH__
#define __BIO_DATAPATH__

#include "../writecache/wcb-mngr.h"

enum {
	FSCFTL_BIO_COMPLETE = 0,
	FSCFTL_BIO_RESUBMIT = 1,
};

bool wcb_available(int nr_ppas);
int alloc_wcb_core(sector_t slba, u32 nr_ppas, struct wcb_bio_ctx *wcb_resource);
void set_l2ptbl_write_path(struct nvm_exdev *exdev, struct wcb_bio_ctx *wcb_resource);
void flush_data_to_wcb(struct nvm_exdev *exdev, struct wcb_bio_ctx *wcb_resource, struct bio *bio);
int process_write_bio(struct request_queue *q, struct bio *bio);
int fscftl_writer_init(struct nvm_exdev *exdev);
void fscftl_writer_exit(struct nvm_exdev *exdev);

#endif

