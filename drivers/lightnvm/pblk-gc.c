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
int pblk_gc_move_valid_secs(struct pblk *pblk, struct pblk_line *line,
			    u64 *lba_list, unsigned int secs_to_move)
{
	struct nvm_tgt_dev *dev = pblk->dev;
	struct nvm_geo *geo = &dev->geo;
	void *data;
	unsigned int alloc_entries, nr_secs, secs_to_gc;
	unsigned int secs_left;
	int max = pblk->max_write_pgs;
	int off = 0;

	if (secs_to_move == 0)
		return 0;

	alloc_entries = (secs_to_move > max) ? max : secs_to_move;
	data = kmalloc(alloc_entries * geo->sec_size, GFP_KERNEL);
	if (!data)
		goto out;

	secs_left = secs_to_move;
	do {
		nr_secs = (secs_left > max) ? max : secs_left;

		/* Read from GC victim block */
		if (pblk_submit_read_gc(pblk, &lba_list[off], data, nr_secs,
							&secs_to_gc, line))
			goto fail_free_data;

		if (secs_to_gc == 0)
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

/* linearly find the line with highest number of invalid pages */
static struct pblk_line *line_find_max_isc(struct pblk_line_mgmt *l_mg)
{
	struct pblk_line *line, *max;

#ifdef CONFIG_NVM_DEBUG
	lockdep_assert_held(&l_mg->lock);
#endif

	/* logic error */
	BUG_ON(list_empty(&l_mg->closed_list));

	max = list_first_entry(&l_mg->closed_list, struct pblk_line, list);
	list_for_each_entry(line, &l_mg->closed_list, list)
		max = (line->isc > max->isc) ? line : max;

	return max;
}

static void pblk_put_line_back(struct pblk *pblk, struct pblk_line *line)
{
	struct pblk_line_mgmt *l_mg = &pblk->l_mg;

	spin_lock(&l_mg->lock);
	spin_lock(&line->lock);
	BUG_ON(line->state != PBLK_LINESTATE_GC);
	line->state = PBLK_LINESTATE_CLOSED;
	spin_unlock(&line->lock);

	list_add_tail(&line->list, &l_mg->closed_list);
	spin_unlock(&l_mg->lock);
}

static void pblk_gc_line(struct pblk *pblk, struct pblk_line *line)
{
	struct pblk_line_mgmt *l_mg = &pblk->l_mg;
	struct pblk_line_meta *lm = &pblk->lm;
	struct line_emeta *emeta;
	int sec_moved, sec_left;
	u64 *gc_lba_list, *lba_list;
	int nr_ppas, bit, ret;

	pr_debug("pblk: line '%d' being reclaimed for GC\n", line->id);

	sec_left = line->sec_in_line - line->isc;
	if (!sec_left) {
		/* Lines are erased before being used (l_mg->data_/log_next) */
		kref_put(&line->ref, pblk_line_put);
		return;
	}

	/* logic error */
	BUG_ON(sec_left < 0);

	gc_lba_list = kmalloc(pblk->max_write_pgs * sizeof(u64), GFP_KERNEL);
	if (!gc_lba_list)
		return;

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

	/*
	 * If this read fails, it means that emeta is corrupted. For now, leave
	 * the line untouched.
	 * TODO: Implement a recovery routine that scans and moves all sectors
	 * on the line.
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
		pr_err("pblk: could not GC all sectors: line:%d, GC:%d/%d/%d\n",
						line->id,
						sec_moved, nr_ppas, line->isc);
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
}

void pblk_gc_run(struct work_struct *work)
{
	struct pblk *pblk = container_of(work, struct pblk, ws_gc);
	struct pblk_line_mgmt *l_mg = &pblk->l_mg;
	struct pblk_gc *gc = &pblk->gc;
	struct pblk_line *line, *tline;
	unsigned int nr_blocks_free, nr_blocks_need;
	int run_gc;
	LIST_HEAD(gc_list);

	nr_blocks_need = pblk_rl_gc_thrs(&pblk->rl);
	nr_blocks_free = pblk_rl_nr_free_blks(&pblk->rl);

	run_gc = (nr_blocks_need > nr_blocks_free || gc->gc_forced);
	spin_lock(&l_mg->lock);
	while (run_gc && !list_empty(&l_mg->closed_list)) {
		line = line_find_max_isc(l_mg);
		if (!line->isc)
			goto start_gc;

		nr_blocks_free += line->blk_in_line;

		spin_lock(&line->lock);
		BUG_ON(line->state != PBLK_LINESTATE_CLOSED);
		line->state = PBLK_LINESTATE_GC;
		spin_unlock(&line->lock);

		list_move_tail(&line->list, &gc_list);

		run_gc = (nr_blocks_need > nr_blocks_free || gc->gc_forced);
	}

start_gc:
	spin_unlock(&l_mg->lock);

	list_for_each_entry_safe(line, tline, &gc_list, list) {
		list_del(&line->list);
		pblk_gc_line(pblk, line);
	}

	//JAVIER
	/* if (unlikely(!list_empty(&rlun->g_bb_list))) */
		/* pblk_recov_clean_g_bb_list(pblk, rlun); */
}

void pblk_gc_kick(struct pblk *pblk)
{
	queue_work(pblk->krqd_wq, &pblk->ws_gc);
}

/*
 * timed GC every interval.
 */
static void pblk_gc_timer(unsigned long data)
{
	struct pblk *pblk = (struct pblk *)data;

	pblk_gc_kick(pblk);
	mod_timer(&pblk->gc_timer, jiffies + msecs_to_jiffies(GC_TIME_MSECS));
}

static void pblk_gc_start(struct pblk *pblk)
{
	setup_timer(&pblk->gc_timer, pblk_gc_timer, (unsigned long)pblk);
	mod_timer(&pblk->gc_timer, jiffies + msecs_to_jiffies(5000));

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
	del_timer(&pblk->gc_timer);

	if (flush_wq)
		flush_workqueue(pblk->kgc_wq);

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

int pblk_gc_sysfs_force(struct pblk *pblk, int value)
{
	struct pblk_gc *gc = &pblk->gc;
	int rsv = 0;

	if (value != 0 && value != 1)
		return -EINVAL;

	spin_lock(&gc->lock);
	if (value == 1) {
		gc->gc_enabled = 1;
		rsv = 64;
	}
	pblk_rl_set_gc_rsc(&pblk->rl, rsv);
	gc->gc_forced = value;
	__pblk_gc_should_start(pblk);
	spin_unlock(&gc->lock);

	return 0;
}

int pblk_gc_sysfs_enable(struct pblk *pblk, int value)
{
	struct pblk_gc *gc = &pblk->gc;
	int ret = 0;

	if (value == 0) {
		spin_lock(&gc->lock);
		gc->gc_enabled = value;
		spin_unlock(&gc->lock);
		if (gc->gc_active)
			pblk_gc_stop(pblk, 0);
	} else if (value == 1) {
		spin_lock(&gc->lock);
		gc->gc_enabled = value;
		if (!gc->gc_active)
			pblk_gc_start(pblk);
		spin_unlock(&gc->lock);
	} else {
		ret = -EINVAL;
	}

	return ret;
}

int pblk_gc_init(struct pblk *pblk)
{
	pblk->krqd_wq = alloc_workqueue("pblk-gc", WQ_MEM_RECLAIM | WQ_UNBOUND,
									1);
	if (!pblk->krqd_wq)
		return -ENOMEM;

	pblk->kgc_wq = alloc_workqueue("pblk-bg", WQ_MEM_RECLAIM, 1);
	if (!pblk->kgc_wq)
		goto fail_destrow_krqd_qw;

	pblk->gc.gc_active = 0;
	pblk->gc.gc_forced = 0;
	pblk->gc.gc_enabled = 1;

	spin_lock_init(&pblk->gc.lock);

	return 0;

fail_destrow_krqd_qw:
	destroy_workqueue(pblk->krqd_wq);
	return -ENOMEM;
}

void pblk_gc_exit(struct pblk *pblk)
{
	pblk_gc_stop(pblk, 1);

	if (pblk->krqd_wq)
		destroy_workqueue(pblk->krqd_wq);

	if (pblk->kgc_wq)
		destroy_workqueue(pblk->kgc_wq);
}
