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
 * pblk-gc.c - pblk's garbage collector
 */

#include "pblk.h"

static int pblk_gc_move_valid_secs(struct pblk *pblk, struct pblk_line *line,
				   u64 *lba_list, unsigned int secs_to_move)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	struct nvm_geo *geo = &dev->geo;
	void *data;
	unsigned int alloc_entries, nr_secs, secs_to_gc;
	unsigned int secs_left = secs_to_move;
	int off = 0, max = pblk->max_write_pgs;

	if (!secs_to_move)
		return 0;

	alloc_entries = (secs_to_move > max) ? max : secs_to_move;

	data = kmalloc(alloc_entries * geo->sec_size, GFP_KERNEL);
	if (!data)
		goto out;

	do {
		nr_secs = (secs_left > max) ? max : secs_left;

		/* Read from GC victim block */
		if (pblk_submit_read_gc(pblk, &lba_list[off], data, nr_secs,
							&secs_to_gc, line))
			goto fail_free_data;

		if (!secs_to_gc)
			goto next;

		/* Write to buffer */
		if (pblk_write_gc_to_cache(pblk, data, &lba_list[off],
				nr_secs, secs_to_gc, line, PBLK_IOTYPE_GC))
			goto fail_free_data;

next:
		secs_left -= nr_secs;
		off += nr_secs;
	} while (secs_left > 0);

fail_free_data:
	kfree(data);
out:
	return off;
}

static void pblk_put_line_back(struct pblk *pblk, struct pblk_line *line)
{
	struct pblk_line_mgmt *l_mg = &pblk->l_mg;
	struct list_head *move_list = NULL;

	spin_lock(&line->lock);
	WARN_ON(line->state != PBLK_LINESTATE_GC);
	line->state = PBLK_LINESTATE_CLOSED;
	move_list = pblk_line_gc_list(pblk, line);
	spin_unlock(&line->lock);

	if (move_list) {
		spin_lock(&l_mg->gc_lock);
		list_add_tail(&line->list, move_list);
		spin_unlock(&l_mg->gc_lock);
	}
}

static void pblk_gc_line_ws(struct work_struct *work)
{
	struct pblk_line_ws *line_ws = container_of(work, struct pblk_line_ws,
									ws);
	struct pblk *pblk = line_ws->pblk;
	struct pblk_line *line = line_ws->priv;
	struct pblk_line_mgmt *l_mg = &pblk->l_mg;
	struct pblk_line_meta *lm = &pblk->lm;
	struct line_emeta *emeta;
	int max = pblk->max_write_pgs;
	int sec_moved, sec_left;
	u64 *gc_lba_list, *lba_list;
	int nr_ppas, bit, ret;

	pr_debug("pblk: line '%d' being reclaimed for GC\n", line->id);

	spin_lock(&line->lock);
	sec_left = line->vsc;
	if (!sec_left) {
		/* Lines are erased before being used (l_mg->data_/log_next) */
		spin_unlock(&line->lock);
		kref_put(&line->ref, pblk_line_put);
		goto out;
	}
	spin_unlock(&line->lock);

	/* logic error */
	BUG_ON(sec_left < 0);

	gc_lba_list = kmalloc_array(max, sizeof(u64), GFP_KERNEL);
	if (!gc_lba_list)
		goto out;

	emeta = l_mg->gc_meta.meta;
	if (!emeta) {
		pr_err("pblk: cannot use GC emeta\n");
		goto free_lba_list;
	}

	line->emeta = emeta;
	ret = pblk_line_read_emeta(pblk, line);
	if (ret) {
		pr_err("pblk: line %d read emeta failed (%d)\n", line->id, ret);
		goto free_emeta;
	}

	/* If this read fails, it means that emeta is corrupted. For now, leave
	 * the line untouched. TODO: Implement a recovery routine that scans and
	 * moves all sectors on the line.
	 */
	lba_list = pblk_recov_get_lba_list(pblk, emeta);
	if (!lba_list) {
		pr_err("pblk: could not interpret emeta (line %d)\n", line->id);
		goto put_line;
	}

	bit = -1;
next_rq:
	nr_ppas = 0;
	do {
		bit = find_next_zero_bit(line->invalid_bitmap, lm->sec_per_line,
								bit + 1);
		if (bit > line->emeta_ssec)
			goto prepare_rq;

		gc_lba_list[nr_ppas] = lba_list[bit];
		nr_ppas++;
	} while (nr_ppas < pblk->max_write_pgs);

prepare_rq:
	sec_moved = pblk_gc_move_valid_secs(pblk, line, gc_lba_list, nr_ppas);
	if (sec_moved != nr_ppas) {
		pr_err("pblk: could not GC all sectors: line:%d (%d/%d/%d)\n",
						line->id, line->vsc,
						sec_moved, nr_ppas);
		pblk_put_line_back(pblk, line);
		goto free_emeta;
	}

	sec_left -= sec_moved;
	if (sec_left > 0)
		goto next_rq;

	/* Logic error */
	BUG_ON(sec_left != 0);

put_line:
	/* Lines are erased before being used (l_mg->data_/log_next) */
	kref_put(&line->ref, pblk_line_put);

free_emeta:
	line->emeta = NULL;
free_lba_list:
	kfree(gc_lba_list);
out:
	mempool_free(line_ws, pblk->line_ws_pool);
}

static int pblk_gc_line(struct pblk *pblk, struct pblk_line *line)
{
	struct pblk_line_ws *line_ws;

	line_ws = mempool_alloc(pblk->line_ws_pool, GFP_ATOMIC);
	if (!line_ws)
		return 1;

	line_ws->pblk = pblk;
	line_ws->priv = line;

	INIT_WORK(&line_ws->ws, pblk_gc_line_ws);
	queue_work(pblk->gc_wq, &line_ws->ws);

	return 0;
}

static void pblk_gc_run(struct pblk *pblk)
{
	struct pblk_line_mgmt *l_mg = &pblk->l_mg;
	struct pblk_gc *gc = &pblk->gc;
	struct pblk_line *line, *tline;
	unsigned int nr_blocks_free, nr_blocks_need;
	struct list_head *group_list;
	int run_gc, gc_group = 0;

	spin_lock(&l_mg->gc_lock);
	list_for_each_entry_safe(line, tline, &l_mg->gc_full_list, list) {
		spin_lock(&line->lock);
		BUG_ON(line->state != PBLK_LINESTATE_CLOSED);
		line->state = PBLK_LINESTATE_GC;
		spin_unlock(&line->lock);

		list_del(&line->list);
		kref_put(&line->ref, pblk_line_put);
	}

	nr_blocks_need = pblk_rl_gc_thrs(&pblk->rl);
	nr_blocks_free = pblk_rl_nr_free_blks(&pblk->rl);
	run_gc = (nr_blocks_need > nr_blocks_free || gc->gc_forced);

next_gc_group:
	group_list = l_mg->gc_lists[gc_group++];
	while (run_gc && !list_empty(group_list)) {
		if (!run_gc)
			goto out;

		line = list_first_entry(group_list, struct pblk_line, list);
		nr_blocks_free += line->blk_in_line;

		spin_lock(&line->lock);
		BUG_ON(line->state != PBLK_LINESTATE_CLOSED);
		line->state = PBLK_LINESTATE_GC;
		spin_unlock(&line->lock);

		list_del(&line->list);
		if (pblk_gc_line(pblk, line)) {
			pr_err("pblk: failed to GC line %d\n", line->id);
			goto out;
		}

		run_gc = (nr_blocks_need > nr_blocks_free || gc->gc_forced);
	}

	if (gc_group < PBLK_NR_GC_LISTS)
		goto next_gc_group;

out:
	spin_unlock(&l_mg->gc_lock);
}

static void pblk_gc_kick(struct pblk *pblk)
{
	wake_up_process(pblk->ts_gc);
	mod_timer(&pblk->gc_timer, jiffies + msecs_to_jiffies(GC_TIME_MSECS));
}

/*
 * timed GC every interval.
 */
static void pblk_gc_timer(unsigned long data)
{
	struct pblk *pblk = (struct pblk *)data;

	pblk_gc_kick(pblk);
}

static int pblk_gc_ts(void *data)
{
	struct pblk *pblk = data;

	while (!kthread_should_stop()) {
		pblk_gc_run(pblk);
		set_current_state(TASK_INTERRUPTIBLE);
		io_schedule();
	}

	return 0;
}

static void pblk_gc_start(struct pblk *pblk)
{
	pblk->gc.gc_active = 1;

	pr_debug("pblk: gc running\n");
}

int pblk_gc_status(struct pblk *pblk)
{
	struct pblk_gc *gc = &pblk->gc;
	int ret;

	spin_lock(&gc->lock);
	ret = gc->gc_active;
	spin_unlock(&gc->lock);

	return ret;
}

static void __pblk_gc_should_start(struct pblk *pblk)
{
	struct pblk_gc *gc = &pblk->gc;

#ifdef CONFIG_NVM_DEBUG
	lockdep_assert_held(&gc->lock);
#endif

	if (gc->gc_enabled && !gc->gc_active)
		pblk_gc_start(pblk);
}

void pblk_gc_should_start(struct pblk *pblk)
{
	struct pblk_gc *gc = &pblk->gc;

	spin_lock(&gc->lock);
	__pblk_gc_should_start(pblk);
	spin_unlock(&gc->lock);
}

/*
 * If flush_wq == 1 then no lock should be held by the caller since
 * flush_workqueue can sleep
 */
static void pblk_gc_stop(struct pblk *pblk, int flush_wq)
{
	spin_lock(&pblk->gc.lock);
	pblk->gc.gc_active = 0;
	spin_unlock(&pblk->gc.lock);

	pr_debug("pblk: gc paused\n");
}

void pblk_gc_should_stop(struct pblk *pblk)
{
	struct pblk_gc *gc = &pblk->gc;

	if (gc->gc_active && !gc->gc_forced)
		pblk_gc_stop(pblk, 0);
}

void pblk_gc_sysfs_state_show(struct pblk *pblk, int *gc_enabled,
			      int *gc_active)
{
	struct pblk_gc *gc = &pblk->gc;

	spin_lock(&gc->lock);
	*gc_enabled = gc->gc_enabled;
	*gc_active = gc->gc_active;
	spin_unlock(&gc->lock);
}

void pblk_gc_sysfs_force(struct pblk *pblk, int force)
{
	struct pblk_gc *gc = &pblk->gc;
	int rsv = 0;

	spin_lock(&gc->lock);
	if (force) {
		gc->gc_enabled = 1;
		rsv = 64;
	}
	pblk_rl_set_gc_rsc(&pblk->rl, rsv);
	gc->gc_forced = force;
	__pblk_gc_should_start(pblk);
	spin_unlock(&gc->lock);
}

static void pblk_gc_log_page_sector(struct pblk *pblk,
				    struct nvm_log_page log_page)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	struct nvm_geo *geo = &dev->geo;
	struct bio *bio;
	struct pblk_sec_meta *meta_list;
	struct pblk_line *line;
	struct nvm_rq rqd;
	dma_addr_t dma_meta_list;
	void *data;
	u64 lba;
	int ret;
	DECLARE_COMPLETION_ONSTACK(wait);

	meta_list = nvm_dev_dma_alloc(dev->parent, GFP_KERNEL, &dma_meta_list);
	if (!meta_list)
		return;

	data = kcalloc(pblk->max_write_pgs, geo->sec_size, GFP_KERNEL);
	if (!data)
		goto free_meta_list;

	bio = bio_map_kern(dev->q, data, geo->sec_size, GFP_KERNEL);
	if (IS_ERR(bio))
		goto out;

	memset(&rqd, 0, sizeof(struct nvm_rq));

	bio->bi_iter.bi_sector = 0; /* artificial bio */
	bio_set_op_attrs(bio, REQ_OP_READ, 0);
	bio->bi_private = &wait;
	bio->bi_end_io = pblk_end_bio_sync;

	rqd.bio = bio;
	rqd.opcode = NVM_OP_PREAD;
	rqd.flags = pblk_set_read_mode(pblk);
	rqd.meta_list = meta_list;
	rqd.nr_ppas = 1;
	rqd.ppa_addr = log_page.ppa;
	rqd.dma_meta_list = dma_meta_list;
	rqd.end_io = NULL;

	ret = pblk_submit_io(pblk, &rqd);
	if (ret) {
		pr_err("pblk: recovery I/O submission failed: %d\n", ret);
		bio_put(bio);
		goto out;
	}
	wait_for_completion_io(&wait);
	bio_put(bio);

	if (rqd.error) {
		pr_err("pblk: page log read error: %d\n", rqd.error);
		goto out;
	}

	lba = meta_list[0].lba;
	if (lba > pblk->rl.nr_secs) {
		pr_err("pblk: corrupted P2L map - LBA:%llu", lba);
		print_ppa(&log_page.ppa, "BAD PPA", 0);
		goto out;
	}

	line = &pblk->lines[pblk_ppa_to_line(log_page.ppa)];

	/* L2P is updated as a normal GC write */
	if (pblk_write_gc_to_cache(pblk, data, &lba, 1, 1, line,
							PBLK_IOTYPE_GC)) {
		pr_err("pblk: could not recover log page\n");
		print_ppa(&log_page.ppa, "UNREC. LOGPAGE", log_page.scope);
	}

#ifdef CONFIG_PBLK_AER_DEBUG
	printk(KERN_CRIT "sector (ppa %llx) put in cache\n", log_page.ppa.ppa);
#endif

out:
	kfree(data);
free_meta_list:
	nvm_dev_dma_free(dev->parent, meta_list, dma_meta_list);
}

static void pblk_gc_log_page_block(struct pblk *pblk,
				   struct nvm_log_page log_page)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	struct nvm_geo *geo = &dev->geo;
	struct ppa_addr bppa = log_page.ppa;
	struct bio *bio;
	struct ppa_addr *ppa_list;
	struct pblk_sec_meta *meta_list;
	struct pblk_line *line;
	struct nvm_rq rqd;
	struct ppa_addr ppa;
	void *data;
	dma_addr_t dma_meta_list;
	dma_addr_t dma_ppa_list;
	u64 *lba_list;
	int i, j, k, recov_pgs = 0;
	int rq_ppas, rq_len;
	int ret;
	DECLARE_COMPLETION_ONSTACK(wait);

	ppa_list = nvm_dev_dma_alloc(dev->parent, GFP_KERNEL, &dma_ppa_list);
	if (!ppa_list)
		return;

	meta_list = nvm_dev_dma_alloc(dev->parent, GFP_KERNEL, &dma_meta_list);
	if (!meta_list)
		goto free_ppa_list;

	lba_list = kcalloc(pblk->max_write_pgs, sizeof(u64), GFP_KERNEL);
	if (!lba_list)
		goto free_meta_list;

	data = kcalloc(pblk->max_write_pgs, geo->sec_size, GFP_KERNEL);
	if (!data)
		goto free_lba_list;

next_rq:
	memset(&rqd, 0, sizeof(struct nvm_rq));

	rq_ppas = pblk->max_write_pgs;
	rq_len = rq_ppas * geo->sec_size;

	bio = bio_map_kern(dev->q, data, rq_len, GFP_KERNEL);
	if (IS_ERR(bio))
		goto free_data;

	bio->bi_iter.bi_sector = 0; /* artificial bio */
	bio_set_op_attrs(bio, REQ_OP_READ, 0);
	bio->bi_private = &wait;
	bio->bi_end_io = pblk_end_bio_sync;

	rqd.bio = bio;
	rqd.opcode = NVM_OP_PREAD;
	rqd.flags = pblk_set_progr_mode(pblk, READ);
	rqd.flags |= NVM_IO_SUSPEND | NVM_IO_SCRAMBLE_ENABLE;
	rqd.nr_ppas = rq_ppas;
	rqd.meta_list = meta_list;
	rqd.ppa_list = ppa_list;
	rqd.dma_ppa_list = dma_ppa_list;
	rqd.dma_meta_list = dma_meta_list;
	rqd.end_io = NULL;

	for (i = 0; i < rqd.nr_ppas; ) {
		ppa = bppa;
		ppa.g.pg = recov_pgs++;

		for (j = 0; j < geo->nr_planes; j++) {
			ppa.g.pl = j;

			for (k = 0; k < geo->sec_per_pg; k++) {
				ppa.g.sec = k;
				rqd.ppa_list[i++] = ppa;
			}
		}
	}

	ret = pblk_submit_io(pblk, &rqd);
	if (ret) {
		pr_err("pblk: recovery I/O submission failed: %d\n", ret);
		bio_put(bio);
		goto free_data;
	}
	wait_for_completion_io(&wait);

	if (rqd.error) {
		pr_err("pblk: page log read error: %d\n", rqd.error);
		goto out;
	}

	for (i = 0; i < rqd.nr_ppas; i++) {
		u64 lba = lba_list[i] = meta_list[i].lba;

		if (lba > pblk->rl.nr_secs) {
			pr_err("pblk: corrupted P2L map - LBA:%llu", lba);
			print_ppa(&log_page.ppa, "BAD PPA", 0);
			goto out;
		}
	}

	line = &pblk->lines[pblk_ppa_to_line(log_page.ppa)];

	/* L2P updates since the event are handled by the write buffer */
	if (pblk_write_gc_to_cache(pblk, data, lba_list, rqd.nr_ppas,
					rqd.nr_ppas, line, PBLK_IOTYPE_GC)) {
		pr_err("pblk: could not recover log page\n");
		print_ppa(&log_page.ppa, "UNREC. LOGPAGE", log_page.scope);
	}

out:
	bio_put(bio);
	if (recov_pgs < geo->pgs_per_blk)
		goto next_rq;

#ifdef CONFIG_PBLK_AER_DEBUG
	printk(KERN_CRIT "block (ppa %llx) put in cache\n", log_page.ppa.ppa);
#endif

free_data:
	kfree(data);
free_lba_list:
	kfree(lba_list);
free_meta_list:
	nvm_dev_dma_free(dev->parent, meta_list, dma_meta_list);
free_ppa_list:
	nvm_dev_dma_free(dev->parent, ppa_list, dma_ppa_list);
}

/* If a LUN fails, reads will not succeed. Another form for redundancy is
 * necessary to cover this case.
 */
static void pblk_gc_log_page_lun(struct pblk *pblk,
				 struct nvm_log_page log_page)
{
	pr_err("pblk: unrecoverable LUN failure: ch:%d, lun:%d\n",
					log_page.ppa.g.ch,
					log_page.ppa.g.lun);
}

void pblk_gc_log_page(struct pblk *pblk, struct nvm_log_page log_page)
{
	if (log_page.scope & NVM_LOGPAGE_SCOPE_SECTOR)
		pblk_gc_log_page_sector(pblk, log_page);
	else if (log_page.scope & NVM_LOGPAGE_SCOPE_CHUNK)
		pblk_gc_log_page_block(pblk, log_page);
	else if (log_page.scope & NVM_LOGPAGE_SCOPE_LUN)
		pblk_gc_log_page_lun(pblk, log_page);
	else
		pr_err("pblk: unknown log page error (0x%x\n)", log_page.scope);
}

int pblk_gc_init(struct pblk *pblk)
{
	pblk->ts_gc = kthread_create(pblk_gc_ts, pblk, "pblk-gc");

	setup_timer(&pblk->gc_timer, pblk_gc_timer, (unsigned long)pblk);
	mod_timer(&pblk->gc_timer, jiffies + msecs_to_jiffies(GC_TIME_MSECS));

	pblk->gc_wq = alloc_workqueue("pblk-gc", WQ_MEM_RECLAIM | WQ_UNBOUND,
									1);
	if (!pblk->gc_wq)
		return -ENOMEM;

	pblk->gc.gc_active = 0;
	pblk->gc.gc_forced = 0;
	pblk->gc.gc_enabled = 1;

	spin_lock_init(&pblk->gc.lock);

	return 0;
}

void pblk_gc_exit(struct pblk *pblk)
{
	del_timer(&pblk->gc_timer);
	pblk_gc_stop(pblk, 1);

	if (pblk->ts_gc)
		kthread_stop(pblk->ts_gc);

	if (pblk->gc_wq)
		destroy_workqueue(pblk->gc_wq);
}
