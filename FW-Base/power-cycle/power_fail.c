/*
 * Copyright (C) 2018 Wangxun, Inc. dengcaixu@net-swift.com
 * All Rights Reserved.
 *
 * Function: Power-Fail
 *	 when FW detect Unplanned Power Outage, It will dump the following data to Nand 
 *	 during very short period(50-100ms) of capacitor power supply, in our HW design
 *	 it allow write about 188MB data. i.e. 47K X 4K s(lot)
 *
 *		Host Band:
 *			0. disable new host/internal command, close
 *			1. in cache not commit user/recycle data				(128MB	 23K slot)
 *			2. for TLC close UP XP, stable Cell
 *
 *		System Band:
 *			3. each host band open blk's partial LOG Pages			(64KB	 16  slot)
 *			4. RDC Table \
 *						  ----->									(1MB  256 slot)
 *			5. BMI Table /
 *
 *		Boot Block
 *			6. boot_page_primary_info
 *
 */


// global variable
volatile bool power_fail_detected = false;



// when power fail detect
void powerfail_task()
{
	power_fail_detected = true;

//1 0.Stop e.g. Recycle/CKPT/newIO rightnow


//1 1.Host Band
	persisit_flight_hdata(0);	// no pad due to no time

//1 2.System Band
	persisi_flight_sdata(false, 0)	// no MAP due to no time

//1 3.Boot Block
	//2 3.1 Flush Primary Page
	g_primary_page->page.power_down_state = POWER_DOWN_STATE_PFAIL;
	flush_primary_page();
	

	power_fail_detected = false;

	return;
}


// Pfail detect, this IRQ priority is high
irq_event()
{
	u32 state = REG_READ(offset);

	if (test_bit(state, PFAIL_BITPOS)) {
		powerfail_task();
	}
	
}

	
