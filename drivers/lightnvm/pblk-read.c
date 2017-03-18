/*
 * Copyright (C) 2016 CNEX Labs
 * Initial release: Javier Gonzalez <javier@cnexlabs.com>
 *                  Matias Bjorling <matias@cnexlabs.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 *
 * pblk-read.c - pblk's read path
 */

#include "pblk.h"

static void pblk_lookup_l2p_seq(struct pblk *pblk, struct ppa_addr *ppas,
				sector_t bladdr, int nr_secs)
{
	int i;

	spin_lock(&pblk->trans_lock);
	for (i = 0; i < nr_secs; i++)
		ppas[i] = pblk->trans_map[bladdr + i];
	spin_unlock(&pblk->trans_lock);
}

static void pblk_lookup_l2p_rand(struct pblk *pblk, struct ppa_addr *ppas,
				 u64 *lba_list, int nr_secs)
{
	sector_t lba;
	int i;

	spin_lock(&pblk->trans_lock);
	for (i = 0; i < nr_secs; i++) {
		lba = lba_list[i];
		if (lba == ADDR_EMPTY) {
			ppas[i].ppa = ADDR_EMPTY;
		} else {
			/* logic error: lba out-of-bounds */
			BUG_ON(!(lba >= 0 && lba < pblk->rl.nr_secs));
			ppas[i] = pblk->trans_map[lba];
		}
	}
	spin_unlock(&pblk->trans_lock);
}

/*
 * There is no guarantee that the value read from cache has not been updated and
 * resides at another location in the cache. We guarantee though that if the
 * value is read from the cache, it belongs to the mapped lba. In order to
 * guarantee and order between writes and reads are ordered, a flush must be
 * issued.
 */
static int pblk_read_from_cache(struct pblk *pblk, struct bio *bio,
				sector_t laddr, struct ppa_addr ppa,
				int bio_iter)
{
	return pblk_rb_copy_to_bio(&pblk->rwb, bio, laddr,
				nvm_addr_to_cacheline(ppa), bio_iter);
}

static int pblk_try_read_from_cache(struct pblk *pblk, struct bio *bio,
				sector_t laddr, struct ppa_addr ppa,
				int bio_iter)
{
	if (!nvm_addr_in_cache(ppa))
		return 0;

	return pblk_read_from_cache(pblk, bio, laddr, ppa, bio_iter);
}

static int pblk_read_ppalist_rq(struct pblk *pblk, struct nvm_rq *rqd,
				unsigned long *read_bitmap)
{
	struct bio *bio = rqd->bio;
	struct ppa_addr ppas[PBLK_MAX_REQ_ADDRS];
	sector_t bladdr = pblk_get_laddr(bio);
	int nr_secs = rqd->nr_ppas;
	int advanced_bio = 0;
	int i, j = 0;

	/* logic error: lba out-of-bounds */
	BUG_ON(!(bladdr >= 0 && bladdr + nr_secs < pblk->rl.nr_secs));

	pblk_lookup_l2p_seq(pblk, ppas, bladdr, nr_secs);

	for (i = 0; i < nr_secs; i++) {
		struct ppa_addr *p = &ppas[i];
		sector_t laddr = bladdr + i;

		if (ppa_empty(*p)) {
			WARN_ON(test_and_set_bit(i, read_bitmap));
			continue;
		}

retry:
		/* Try to read from write buffer. The address is later checked
		 * on the write buffer to prevent retrieving overwritten data.
		 */
		if (nvm_addr_in_cache(*p)) {
			WARN_ON(test_and_set_bit(i, read_bitmap));
			if (!pblk_read_from_cache(pblk, bio, laddr, *p, i)) {
				WARN_ON(!test_and_clear_bit(i, read_bitmap));
				pblk_lookup_l2p_seq(pblk, p, laddr, 1);
				goto retry;
			}
			advanced_bio = 1;
		} else {
			/* Fill ppa_list with the sectors that cannot be
			 * read from cache
			 */
			rqd->ppa_list[j] = *p;
			j++;
		}

		if (advanced_bio)
			bio_advance(bio, PBLK_EXPOSED_PAGE_SIZE);
	}

#ifdef CONFIG_NVM_DEBUG
		atomic_add(nr_secs, &pblk->inflight_reads);
#endif

	return NVM_IO_OK;
}

static int pblk_submit_read_io(struct pblk *pblk, struct nvm_rq *rqd)
{
	int err;

	rqd->flags = pblk_set_read_mode(pblk);

	err = pblk_submit_io(pblk, rqd);
	if (err) {
		pr_err("pblk: I/O submission failed: %d\n", err);
		return NVM_IO_ERR;
	}

	return NVM_IO_OK;
}

static void pblk_end_io_read(struct nvm_rq *rqd)
{
	struct pblk *pblk = rqd->private;
	struct nvm_tgt_dev *dev = pblk->dev;
	struct pblk_r_ctx *r_ctx = nvm_rq_to_pdu(rqd);
	struct bio *bio = rqd->bio;

	if (rqd->error)
		pblk_log_read_err(pblk, rqd);
#ifdef CONFIG_NVM_DEBUG
	else
		BUG_ON(bio->bi_error);
#endif

	if (rqd->nr_ppas > 1)
		nvm_dev_dma_free(dev->parent, rqd->ppa_list, rqd->dma_ppa_list);

	if (rqd->meta_list)
		nvm_dev_dma_free(dev->parent, rqd->meta_list,
							rqd->dma_meta_list);

	bio_put(bio);
	if (r_ctx->orig_bio) {
#ifdef CONFIG_NVM_DEBUG
		BUG_ON(r_ctx->orig_bio->bi_error);
#endif
		bio_endio(r_ctx->orig_bio);
		bio_put(r_ctx->orig_bio);
	}

#ifdef CONFIG_NVM_DEBUG
	atomic_add(rqd->nr_ppas, &pblk->sync_reads);
	atomic_sub(rqd->nr_ppas, &pblk->inflight_reads);
#endif

	pblk_free_rqd(pblk, rqd, READ);
}

static int pblk_fill_partial_read_bio(struct pblk *pblk, struct nvm_rq *rqd,
				      unsigned int bio_init_idx,
				      unsigned long *read_bitmap)
{
	struct bio *new_bio, *bio = rqd->bio;
	struct bio_vec src_bv, dst_bv;
	void *ppa_ptr = NULL;
	void *src_p, *dst_p;
	dma_addr_t dma_ppa_list = 0;
	int nr_secs = rqd->nr_ppas;
	int nr_holes = nr_secs - bitmap_weight(read_bitmap, nr_secs);
	int hole;
	int i;
	int ret;
	DECLARE_COMPLETION_ONSTACK(wait);

	new_bio = bio_alloc(GFP_KERNEL, nr_holes);
	if (!new_bio) {
		pr_err("pblk: could not alloc read bio\n");
		return NVM_IO_ERR;
	}

	if (pblk_bio_add_pages(pblk, new_bio, GFP_KERNEL, nr_holes))
		goto err;

	if (nr_holes != new_bio->bi_vcnt) {
		pr_err("pblk: malformed bio\n");
		goto err;
	}

	new_bio->bi_iter.bi_sector = 0; /* artificial bio */
	bio_set_op_attrs(new_bio, REQ_OP_READ, 0);
	new_bio->bi_private = &wait;
	new_bio->bi_end_io = pblk_end_bio_sync;

	rqd->bio = new_bio;
	rqd->nr_ppas = nr_holes;
	rqd->end_io = NULL;

	if (unlikely(nr_secs > 1 && nr_holes == 1)) {
		ppa_ptr = rqd->ppa_list;
		dma_ppa_list = rqd->dma_ppa_list;
		rqd->ppa_addr = rqd->ppa_list[0];
	}

	ret = pblk_submit_read_io(pblk, rqd);
	if (ret) {
		bio_put(rqd->bio);
		pr_err("pblk: read IO submission failed\n");
		goto err;
	}
	wait_for_completion_io(&wait);

	if (rqd->error) {
		inc_stat(pblk, &pblk->read_failed, 0);
#ifdef CONFIG_NVM_DEBUG
		pblk_print_failed_rqd(pblk, rqd, rqd->error);
#endif
	}

	if (unlikely(nr_secs > 1 && nr_holes == 1)) {
		rqd->ppa_list = ppa_ptr;
		rqd->dma_ppa_list = dma_ppa_list;
	}

	/* Fill the holes in the original bio */
	i = 0;
	hole = find_first_zero_bit(read_bitmap, nr_secs);
	do {
		src_bv = new_bio->bi_io_vec[i];
		dst_bv = bio->bi_io_vec[bio_init_idx + hole];

		src_p = kmap_atomic(src_bv.bv_page);
		dst_p = kmap_atomic(dst_bv.bv_page);

		memcpy(dst_p + dst_bv.bv_offset,
			src_p + src_bv.bv_offset,
			PBLK_EXPOSED_PAGE_SIZE);

		kunmap_atomic(src_p);
		kunmap_atomic(dst_p);

		mempool_free(src_bv.bv_page, pblk->page_pool);

		i++;
		hole = find_next_zero_bit(read_bitmap, nr_secs, hole + 1);
	} while (hole < nr_secs);

	bio_put(new_bio);

	/* Complete the original bio and associated request */
	rqd->bio = bio;
	rqd->nr_ppas = nr_secs;
	rqd->private = pblk;

	bio_endio(bio);
	pblk_end_io_read(rqd);
	return NVM_IO_OK;

err:
	/* Free allocated pages in new bio */
	pblk_bio_free_pages(pblk, bio, 0, new_bio->bi_vcnt);
	rqd->private = pblk;
	pblk_end_io_read(rqd);
	return NVM_IO_ERR;
}

static int pblk_read_rq(struct pblk *pblk, struct nvm_rq *rqd,
			unsigned long *read_bitmap)
{
	struct bio *bio = rqd->bio;
	struct ppa_addr ppa;
	sector_t laddr = pblk_get_laddr(bio);

	/* logic error: lba out-of-bounds */
	BUG_ON(!(laddr >= 0 && laddr < pblk->rl.nr_secs));

	spin_lock(&pblk->trans_lock);
	ppa = pblk->trans_map[laddr];
	spin_unlock(&pblk->trans_lock);

	if (ppa_empty(ppa)) {
		WARN_ON(test_and_set_bit(0, read_bitmap));
		return NVM_IO_DONE;
	}

	/* Try to read from write buffer. The address is later checked on the
	 * write buffer to prevent retrieving overwritten data.
	 */
	if (pblk_try_read_from_cache(pblk, bio, laddr, ppa, 0)) {
		WARN_ON(test_and_set_bit(0, read_bitmap));
		return NVM_IO_DONE;
	}

	rqd->ppa_addr = ppa;

#ifdef CONFIG_NVM_DEBUG
	atomic_inc(&pblk->inflight_reads);
#endif
	return NVM_IO_OK;
}

int pblk_submit_read(struct pblk *pblk, struct bio *bio)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	int nr_secs = pblk_get_secs(bio);
	struct nvm_rq *rqd;
	unsigned long read_bitmap; /* Max 64 ppas per request */
	unsigned int bio_init_idx;
	int ret = NVM_IO_ERR;

	if (nr_secs > PBLK_MAX_REQ_ADDRS)
		return NVM_IO_ERR;

	bitmap_zero(&read_bitmap, nr_secs);

	rqd = pblk_alloc_rqd(pblk, READ);
	if (IS_ERR(rqd)) {
		pr_err_ratelimited("pblk: not able to alloc rqd");
		bio_io_error(bio);
		return NVM_IO_ERR;
	}
	rqd->opcode = NVM_OP_PREAD;
	rqd->bio = bio;
	rqd->nr_ppas = nr_secs;
	rqd->private = pblk;
	rqd->end_io = pblk_end_io_read;

	/* Save the index for this bio's start. This is needed in case
	 * we need to fill a partial read.
	 */
	bio_init_idx = pblk_get_bi_idx(bio);

	if (nr_secs > 1) {
		rqd->ppa_list = nvm_dev_dma_alloc(dev->parent, GFP_KERNEL,
						&rqd->dma_ppa_list);
		if (!rqd->ppa_list) {
			pr_err("pblk: not able to allocate ppa list\n");
			goto fail_rqd_free;
		}

		pblk_read_ppalist_rq(pblk, rqd, &read_bitmap);
	} else {
		ret = pblk_read_rq(pblk, rqd, &read_bitmap);
		if (ret)
			goto fail_rqd_free;
	}

	bio_get(bio);
	if (bitmap_full(&read_bitmap, nr_secs)) {
		bio_endio(bio);
		pblk_end_io_read(rqd);
		return NVM_IO_OK;
	}

	/* All sectors are to be read from the device */
	if (bitmap_empty(&read_bitmap, rqd->nr_ppas)) {
		struct bio *int_bio = NULL;
		struct pblk_r_ctx *r_ctx = nvm_rq_to_pdu(rqd);

		/* Clone read bio to deal with read errors internally */
		int_bio = bio_clone_bioset(bio, GFP_KERNEL, fs_bio_set);
		if (!int_bio) {
			pr_err("pblk: could not clone read bio\n");
			return NVM_IO_ERR;
		}

		rqd->bio = int_bio;
		r_ctx->orig_bio = bio;

		ret = pblk_submit_read_io(pblk, rqd);
		if (ret) {
			pr_err("pblk: read IO submission failed\n");
			if (int_bio) {
				bio_endio(bio);
				bio_put(int_bio);
			}
			return ret;
		}

		return NVM_IO_OK;
	}

	/* The read bio request could be partially filled by the write buffer,
	 * but there are some holes that need to be read from the drive.
	 */
	ret = pblk_fill_partial_read_bio(pblk, rqd, bio_init_idx, &read_bitmap);
	if (ret) {
		pr_err("pblk: failed to perform partial read\n");
		return ret;
	}

	return NVM_IO_OK;

fail_rqd_free:
	pblk_free_rqd(pblk, rqd, READ);
	return ret;
}

static int read_ppalist_rq_gc(struct pblk *pblk, struct nvm_rq *rqd,
			      struct pblk_line *line, u64 *lba_list,
			      unsigned int nr_secs)
{
	struct ppa_addr ppas[PBLK_MAX_REQ_ADDRS];
	int valid_secs = 0;
	int i;

	pblk_lookup_l2p_rand(pblk, ppas, lba_list, nr_secs);

	for (i = 0; i < nr_secs; i++) {
		/* Ignore updated values until the moment */
		if (nvm_addr_in_cache(ppas[i]) || ppas[i].g.blk != line->id ||
							ppa_empty(ppas[i])) {
			lba_list[i] = ADDR_EMPTY;
			continue;
		}

		rqd->ppa_list[valid_secs++] = ppas[i];
	}

#ifdef CONFIG_NVM_DEBUG
		atomic_add(valid_secs, &pblk->inflight_reads);
#endif
	return valid_secs;
}

static int read_rq_gc(struct pblk *pblk, struct nvm_rq *rqd,
		      struct pblk_line *line, sector_t laddr)
{
	struct ppa_addr ppa;
	int valid_secs = 0;

	if (laddr == ADDR_EMPTY)
		goto out;

	/* logic error: lba out-of-bounds */
	BUG_ON(!(laddr >= 0 && laddr < pblk->rl.nr_secs));

	spin_lock(&pblk->trans_lock);
	ppa = pblk->trans_map[laddr];
	spin_unlock(&pblk->trans_lock);

	/* Ignore updated values until the moment */
	if (nvm_addr_in_cache(ppa) || ppa.g.blk != line->id || ppa_empty(ppa))
		goto out;

	rqd->ppa_addr = ppa;
	valid_secs = 1;

#ifdef CONFIG_NVM_DEBUG
	atomic_inc(&pblk->inflight_reads);
#endif
	return NVM_IO_OK;
out:
	return valid_secs;
}

static void pblk_free_gc_rqd(struct pblk *pblk, struct nvm_rq *rqd)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	uint8_t nr_secs = rqd->nr_ppas;

	if (nr_secs > 1)
		nvm_dev_dma_free(dev->parent, rqd->ppa_list, rqd->dma_ppa_list);

	if (rqd->meta_list)
		nvm_dev_dma_free(dev->parent, rqd->meta_list,
							rqd->dma_meta_list);

	pblk_free_rqd(pblk, rqd, READ);
}

int pblk_submit_read_gc(struct pblk *pblk, u64 *lba_list, void *data,
			unsigned int nr_secs, unsigned int *secs_to_gc,
			struct pblk_line *line)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	struct nvm_geo *geo = &dev->geo;
	struct request_queue *q = dev->q;
	struct bio *bio;
	struct nvm_rq *rqd;
	int data_len;
	int ret;
	DECLARE_COMPLETION_ONSTACK(wait);

	rqd = pblk_alloc_rqd(pblk, READ);
	if (IS_ERR(rqd)) {
		pr_err("pblk: could not allocate GC request\n");
		goto fail;
	}

	if (nr_secs > 1) {
		rqd->ppa_list = nvm_dev_dma_alloc(dev->parent, GFP_KERNEL,
						  &rqd->dma_ppa_list);
		if (!rqd->ppa_list) {
			pr_err("pblk: not able to allocate ppa list\n");
			return NVM_IO_ERR;
		}

		*secs_to_gc = read_ppalist_rq_gc(pblk, rqd, line, lba_list,
								nr_secs);
		if (*secs_to_gc == 1) {
			struct ppa_addr ppa;

			ppa = rqd->ppa_list[0];
			nvm_dev_dma_free(dev->parent, rqd->ppa_list,
							rqd->dma_ppa_list);
			rqd->ppa_addr = ppa;
		}
	} else {
		*secs_to_gc = read_rq_gc(pblk, rqd, line, lba_list[0]);
	}

	if (!(*secs_to_gc))
		goto out;

	data_len = (*secs_to_gc) * geo->sec_size;
	bio = bio_map_kern(q, data, data_len, GFP_KERNEL);
	if (!bio) {
		pr_err("pblk: could not allocate GC bio\n");
		goto fail_free_rqd;
	}

	bio->bi_iter.bi_sector = 0; /* artificial bio */
	bio_set_op_attrs(bio, REQ_OP_READ, 0);
	bio->bi_private = &wait;
	bio->bi_end_io = pblk_end_bio_sync;

	rqd->opcode = NVM_OP_PREAD;
	rqd->nr_ppas = *secs_to_gc;
	rqd->end_io = NULL;
	rqd->bio = bio;

	ret = pblk_submit_read_io(pblk, rqd);
	if (ret) {
		bio_endio(bio);
		pr_err("pblk: GC read request failed\n");
		goto fail_free_bio;
	}
	wait_for_completion_io(&wait);

	if (rqd->error) {
		inc_stat(pblk, &pblk->read_failed_gc, 0);
#ifdef CONFIG_NVM_DEBUG
		pblk_print_failed_rqd(pblk, rqd, rqd->error);
#endif
	}

#ifdef CONFIG_NVM_DEBUG
	atomic_add(*secs_to_gc, &pblk->sync_reads);
	atomic_sub(*secs_to_gc, &pblk->inflight_reads);
#endif

	bio_put(bio);
out:
	pblk_free_gc_rqd(pblk, rqd);
	return NVM_IO_OK;

fail_free_bio:
	bio_put(bio);
fail_free_rqd:
	pblk_free_gc_rqd(pblk, rqd);
fail:
	return NVM_IO_ERR;
}
