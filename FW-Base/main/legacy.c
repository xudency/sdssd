/*
 * in this file all functions are no used now, but may helpful afterward
 * before production, please keep it here, if you write some code, 
 * but not useful crrently, and you wonder it is helpful, can move it here
 *
 * by Dengcai Xu
 *
 */

// test if the 2 ppa belong the same Page
// blk pg lun ch pl cp, only cp is different
bool is_ppa_in_the_same_page(ppa_t p1, ppa_t p2)
{
	// ignore cp, other is all the same
	return ((p1 >> CP_BITS) == (p2 >> CP_BITS))
}

bool is_ppa_in_this_die(ppa_t ppa, u8 die)
{
	return (PPA_TO_DIE(ppa) == die);
}

// test if the 2 ppa belong the Die
// if yes, this 2 ppa type is the same but some insert ppa
bool is_ppa_in_the_same_die(ppa_t p1, ppa_t p2)
{
	// blk ch lun, ignore Page
	return ((PPA_TO_DIE(p1) == PPA_TO_DIE(p2)) && (p1.nand.blk == p2.nand.blk));
}

// it will skip bad Die
int fwd_die_to_prevn(u16 blk, u8 start_lun, u8 start_ch, 
						u8 end_lun, u8 end_ch, u16 n, u8 *dies)
{
	bool r;
	r = get_lastn_good_die_within_range(blk, start_lun, start_ch, end_lun, end_ch, n, dies);
	if (r)
		return dies[n-1];
	else
		return -1;
}

// this ppa is for host data oe insert sys data or badblock
// delete, this is too slow, cpu-consume, 
// because page type not change frequently, very time to re-cal is not worth
/*pg_type calc_page_type_slow(ppa_t ppa)
{
	int type;

	if (is_bad_block(ppa)) {
		return BADBLK_PAGE;
	}

	if (is_first_ppa(ppa)) {
		return FIRST_PAGE;
	}

	if (is_ftl_log_page(ppa)) {
		return FTL_LOG_PAGE;
	}

	type = is_raif_page(ppa);
	if (type == 0) {
		return NORMAL_PAGE;
	} else if (type == 1) {
		return RAIF1_PAGE;
	} else if (type == 2) {
		return RAIF2_PAGE;
	}
}
*/

