/*
 * first_page is the R-Block header insert in specified FPA 
 * in case of bmitbl corruption.
 * 
 */


 typedef struct first_page
 {
	 u16 blk;
	 u16 sequence;			  /* band sequence */
	 time64_t timestamp;	   /* Erase Safe, data retention */
	 u8 cri;			 /* code rate index */
	 u8 band;					 /* host/recycle/system */
	 u8 state;				 /* FREE/OPEN/CLOSED/ERASE */
	 u8 bb_grown_flag;		 /* BMI_FLAG_PRG_ERR/BMI_FLAG_UECC GC-P0*/
	 u16 pecycle;			 /* Program Erase Cycle */
	 u16 bb_cnt;			 /* MAX is CH*PL*LUN */
	 u16 bbt[CFG_NAND_LUN_NUM][CFG_NAND_PL_NUM]		 /* when latest bbt lost, merge it to previous bbt */
	 read_retry_para fthr;
 } first_page_t;

 extern first_page_t* g_first_page[];

	
