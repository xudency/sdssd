/*  
 * CNEX Labs, Inc.
 * CONFIDENTIAL
 * Copyright 2013 – 2016 CNEX Labs, Inc.
 * All Rights Reserved.
 * 
 * NOTICE: 
 * 	All information contained in this file, is and shall remain the
 * 	property of CNEX Labs, Inc. and its suppliers, if any. 
 * 	The intellectual and technical concepts contained herein are 
 * 	confidential and proprietary to CNEX Labs, Inc. and are protected
 * 	by trade secret and copyright law.  In addition, elements of the 
 * 	information may be patent pending.
 * 	This file is part of CNEX Lab’s Westlake product. No part of the 
 * 	Westlake product, including this file, may be use, copied, modified, 
 *	or distributed except in accordance with the terms contained in 
 * 	CNEX Labs license agreement under which you obtained this file.
 *
 * SVN Info:
 *
 * $Id: cfg_1TB_TH58TFG9DFK_sample.h 5807 2016-05-06 10:48:18Z wenzheng $
 * $Rev: 5807 $:     Revision of last commit
 * $Author: wenzheng $:  Author of last commit
 * $HeadURL: http://172.29.0.208/svn/nexus_sw/branches/nexus_ftl_staging_a/cfg/cfg_1TB_TH58TFG9DFK_sample.h $
 * $Date: 2016-05-06 03:48:18 -0700 (Fri, 06 May 2016) $:    Date of last commit
 *
 */
 

#ifndef CFG_1TB_TH58TFG9DFK_H_
#define CFG_1TB_TH58TFG9DFK_H_

#include "cfg_common.h"

#define FLASH_TYPE  "1TB_TH58TFG9DFK"

//#define CFG_NAND_TIMING_FILE          “..\..\nand_timing\timing_table_toshiba_type_a.h”
#define FLASH_TOSHIBA
//#define USE_SLC_FOR_BOOT_BLK     //use slc for boot blk by default
#define ENABLE_TLC              0
#define NEXUS_CMD_CONTROL_M_PL  0x02
#define NEXUS_CMD_CONTROL_S_PL  0x0

#define CAPACITY ((u64)1<<40)

#define CH_BITS   4
#define EP_BITS   2
#define PL_BITS   2
#define LN_BITS   2
#define PG_BITS   8
#define BL_BITS   11

#define CFG_NAND_EP_NUM              (1<<EP_BITS)
#define CFG_NAND_CHANNEL_NUM         16
#define CFG_NAND_LUN_NUM             4
#define CFG_NAND_BLOCK_NUM           1069
#define CFG_NAND_PAGE_NUM            256
#define CFG_NAND_PLANE_NUM           4
#define CFG_NAND_PAGE_SIZE           16384
#define CFG_NAND_PAGE_SPARE          1260
#define CFG_NAND_EP_SIZE             4096

#define CFG_NAND_BAD_MARKER_PAGE_CNT 2
#define CFG_NAND_BAD_MARKER_PAGE_0   0
#define CFG_NAND_BAD_MARKER_PAGE_1   255
#define CFG_NAND_BAD_MARKER_PAGE_2   0xFFFF
#define CFG_NAND_BAD_MARKER_PAGE_3   0xFFFF

#define CFG_NAND_PADDING_PAGES       8

// Column: retention  month0 month1 month2 month3
// Line:   pecycle    step 500 every line
#define PE_DIV_500       36      // Line number
#define RETENTION_MONTH  4	 // Column number

#define  THR_MAPPING_TABLE     {{23, 20, 20, 20}, \
				{23, 21, 21, 22}, \
				{23, 20, 20, 20}, \
				{23, 22, 22, 22}, \
				{23, 25, 24, 23}, \
				{23, 22, 23, 23}, \
				{23, 23, 23, 23}, \
				{23, 23, 23, 23}, \
				{23, 23, 23, 22}, \
				{23, 23, 23, 22}, \
				{23, 30, 28, 28}, \
				{23, 30, 28, 28}, \
				{23, 23, 23, 23}, \
				{23, 23, 23, 23}, \
				{23, 22, 22, 21}, \
				{23, 22, 22, 21}, \
				{23, 23, 23, 23}, \
				{23, 23, 23, 23}, \
				{23, 23, 23, 23}, \
				{23, 23, 23, 23}, \
				{23, 24, 24, 24}, \
				{23, 24, 24, 24}, \
				{23, 24, 24, 24}, \
				{23, 24, 24, 24}, \
				{23, 25, 25, 25}, \
				{23, 25, 25, 25}, \
				{23, 25, 25, 25}, \
				{23, 25, 25, 25}, \
				{23, 25, 25, 25}, \
				{23, 25, 25, 25}, \
				{23, 24, 24, 24}, \
				{23, 24, 24, 24}, \
				{23, 25, 25, 25}, \
				{23, 25, 25, 25}, \
				{23, 26, 26, 26}, \
				{23, 26, 26, 26}}

#ifdef USE_SLC_FOR_BOOT_BLK
	#define CFG_NAND_SLC_PAGE_NUM	128
    #define CFG_NAND_LAST_SLC_VALID_PG  (127)
    #define CFG_NAND_SLC_PAGE_TABLE    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, \
                                    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, \
                                    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, \
                                    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, \
                                    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, \
                                    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, \
                                    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, \
                                    71, 72, 73, 74, 75, 76, 77, 78, 79, 80, \
                                    81, 82, 83, 84, 85, 86, 87, 88, 89, 90, \
                                    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, \
                                    101, 102, 103, 104, 105, 106, 107, 108, 109, 110, \
                                    111, 112, 113, 114, 115, 116, 117, 118, 119, 120, \
                                    121, 122, 123, 124, 125, 126, 127}
#endif 
	
	#define CFG_NAND_LOWER_PAGE_NUM      128
    #define CFG_NAND_LAST_VALID_PG  (253)
    #define CFG_NAND_LOWER_PAGE_TABLE    {0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, \
                                 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, \
                                 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, \
                                 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, \
                                 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, \
                                 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, \
                                 121, 123, 125, 127, 129, 131, 133, 135, 137, 139, \
                                 141, 143, 145, 147, 149, 151, 153, 155, 157, 159, \
                                 161, 163, 165, 167, 169, 171, 173, 175, 177, 179, \
                                 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, \
                                 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, \
                                 221, 223, 225, 227, 229, 231, 233, 235, 237, 239, \
                                 241, 243, 245, 247, 249, 251, 253}  

 
#ifdef USE_SLC_FOR_BOOT_BLK
	#define BOOT_PAGE_NUM	CFG_NAND_SLC_PAGE_NUM
#else
	#define BOOT_PAGE_NUM	CFG_NAND_LOWER_PAGE_NUM
#endif

#define CFG_NAND_MAX_PE              3000
#define CFG_NAND_READ_DISTURB_CNT    100000

#define CFG_DRIVE_LUN_NUM            CFG_NAND_LUN_NUM
#define CFG_DRIVE_CH_NUM             CFG_NAND_CHANNEL_NUM
#define CFG_DRIVE_EP_SIZE            CFG_NAND_EP_SIZE
#define CFG_DRIVE_EP_NUM             (CFG_NAND_PAGE_SIZE / CFG_DRIVE_EP_SIZE)
#define CFG_DRIVE_LINE_NUM           (CFG_NAND_PLANE_NUM * CFG_DRIVE_EP_NUM)
#define CFG_DRIVE_TOTAL_SEC_NUM      (CFG_NAND_PAGE_NUM * CFG_DRIVE_EP_NUM)
#define CFG_DRIVE_FTLLOG_SEC_CNT     (CFG_NAND_PAGE_NUM * CFG_DRIVE_EP_NUM)  /* 256*4 = 1024*/
#define CFG_DRIVE_FTLLOG_PPA_CNT     (DIV_ROUND_UP(CFG_DRIVE_FTLLOG_SEC_CNT * sizeof(unsigned int), FTL_PPA_SIZE)) /* 1 */ 
#define CFG_DRIVE_FTLLOG_START_SEC   (CFG_DRIVE_EP_NUM - 1)
#define CFG_DRIVE_FTLLOG_PG_INTERVAL (256)
#define PAGE_IS_FTLLOG_PAGE(_page)	(((0 == (((_page) + 1) % CFG_DRIVE_FTLLOG_PG_INTERVAL)) || ((CFG_NAND_PAGE_NUM - 1) == (_page))) ? (1) : (0))
#define PAGE_NOT_FTLLOG_PAGE(_page)	(((0 == (((_page) + 1) % CFG_DRIVE_FTLLOG_PG_INTERVAL)) || ((CFG_NAND_PAGE_NUM - 1) == (_page))) ? (0) : (1))

#define CFG_OOB_RAW_PHY_ADDR_OFFSET  0xc70

// ppa address structure for current flash type

enum {
	NVME_QUART_PLANE    = PL_BITS << 0,
	NVME_SINGLE_PLANE	= 0 << 0,	
	NVME_DUAL_PLANE		= 1 << 0,
	NVME_AES_DISABLE 	= 0 << 6,
	NVME_AES_ENABLE 	= 1 << 6,
	NVME_AES_KEY0 		= 0 << 2,
	NVME_AES_KEY1 		= 1 << 2,
	NVME_AES_KEY2 		= 2 << 2,
	NVME_AES_KEY3 		= 3 << 2,
	NVME_AES_KEY4 		= 4 << 2,
	NVME_AES_KEY5 		= 5 << 2,
	NVME_AES_KEY6 		= 6 << 2,
	NVME_AES_KEY7 		= 7 << 2,
	NVME_AES_KEY8 		= 8 << 2,
	NVME_AES_KEY9 		= 9 << 2,
	NVME_AES_KEYA 		= 10 << 2,
	NVME_AES_KEYB 		= 11 << 2,
	NVME_AES_KEYC 		= 12 << 2,
	NVME_AES_KEYD 		= 13 << 2,
	NVME_AES_KEYE 		= 14 << 2,
	NVME_AES_KEYF 		= 15 << 2,
};

struct physical_address
{
  union
  {
    // ppa format in nand
    struct
    {
      unsigned int ch        : 4;  // 16 channels
      unsigned int sec       : 2;  // 4 sectors per page
      unsigned int pl        : 2;  // 4 planes per LUN
      unsigned int lun       : 2;  // 4 LUNs per channel
      unsigned int pg        : 8;  // 256 pages per block	//SLUO@SJ rename page to pg
      unsigned int blk       : 13; // 1024 blocks per plane  //SLUO@SJ rename bl to blk
      unsigned int resved    : 1;  // 4 bits reversed        //SLUO@SJ rename resreved to resved
    } nand;

    // ppa format in write cache
    struct
    {
      unsigned int ch         : 4;  // 16 channels
      unsigned int line       : 4;  // 16 sector lines per LUN
      unsigned int lun        : 2;  // 4 LUNs per channel * ver
      unsigned int ver        : 8; // max version is defined in software based on application
      unsigned int resved     : 13;  // 9 bits reversed  //SLUO@SJ rename resreved to resved
      unsigned int in_cache   : 1;  // 1 bit flag
    } cache;

    unsigned int ppa;
  };
};


/********************************************************************************************/
/* NAND threshold tuning defines */
#define SLC_T1L 1
#define SLC_T1R 0

#define MLC_T1L_STATE 		3
#define MLC_T1R_T2L_STATE 	1
#define MLC_T2R_T3L_STATE 	0
#define MLC_T3R_STATE 		2

#define RD_THR_RANG_LOW    (-64)//(0xc0)/* negative value */
#define RD_THR_RANG_HIGH   (0x3f)

#define NUM_THR_RAM_ENTRIES 512  //512 for MLC, 256 for TLC
#define THR_RAM_ENTRY_SIZE  4    // 4 for MLC, 8 for TLC

#define PAIR_PG_NUM_MAX (128)
#define PAIR_MEM_NUM    (2)
#define CFG_THR_REF_PAIR_PG_TBL \
{ \
	{0,2},{1,4},{3,6},{5,8},{7,10},{9,12},{11,14},{13,16},{15,18},{17,20},\
	{19,22},{21,24},{23,26},{25,28},{27,30},{29,32},{31,34},{33,36},{35,38},{37,40},\
	{39,42},{41,44},{43,46},{45,48},{47,50},{49,52},{51,54},{53,56},{55,58},{57,60},\
	{59,62},{61,64},{63,66},{65,68},{67,70},{69,72},{71,74},{73,76},{75,78},{77,80},\
	{79,82},{81,84},{83,86},{85,88},{87,90},{89,92},{91,94},{93,96},{95,98},{97,100},\
	{99,102},{101,104},{103,106},{105,108},{107,110},{109,112},{111,114},{113,116},{115,118},{117,120},\
	{119,122},{121,124},{123,126},{125,128},{127,130},{129,132},{131,134},{133,136},{135,138},{137,140},\
	{139,142},{141,144},{143,146},{145,148},{147,150},{149,152},{151,154},{153,156},{155,158},{157,160},\
	{159,162},{161,164},{163,166},{165,168},{167,170},{169,172},{171,174},{173,176},{175,178},{177,180},\
	{179,182},{181,184},{183,186},{185,188},{187,190},{189,192},{191,194},{193,196},{195,198},{197,200},\
	{199,202},{201,204},{203,206},{205,208},{207,210},{209,212},{211,214},{213,216},{215,218},{217,220},\
	{219,222},{221,224},{223,226},{225,228},{227,230},{229,232},{231,234},{233,236},{235,238},{237,240},\
	{239,242},{241,244},{243,246},{245,248},{247,250},{249,252},{251,254},{253,255}, \
}

typedef enum {
    LPO_IDX = 0,
	MLC_IDX_START = 1,
    T1_IDX = 1,
    T2_IDX,
    T3_IDX,
} thr_seq;

#define THR_MLC_IDX_ARRAY {T1_IDX, T2_IDX, T3_IDX}
#define THR_LPO_IDX_ARRAY {LPO_IDX}

#define RD_RETRY_FEATURE_ADDR  (0x11)
/********************************************************************************************/


/* firmware define */
#define FLASH_TYPE_VALUE (1)

#define PPAF_CH_LSB      0
#define PPAF_EP_LSB      4
#define PPAF_PL_LSB      8
#define PPAF_LN_LSB      12
#define PPAF_PG_LSB      16
#define PPAF_BL_LSB      20

#define CFG_LUN_PER_CE   0
#define CFG_PPA_FORMAT   ((CH_BITS<<PPAF_CH_LSB) | \
                          (EP_BITS<<PPAF_EP_LSB) | \
                          (PL_BITS<<PPAF_PL_LSB) | \
                          (LN_BITS<<PPAF_LN_LSB) | \
                          (PG_BITS<<PPAF_PG_LSB) | \
                          (11<<PPAF_BL_LSB))
                          
#define CFG_PLANE_NUM    (CFG_NAND_PLANE_NUM - 1)
#define CFG_LUN_NUM      (CFG_NAND_LUN_NUM - 1)
#define CFG_PAGE_NUM     (CFG_NAND_PAGE_NUM - 1)
#define CFG_BLOCK_NUM    0x7ff
#define CFG_CHANNEL_NUM  (CFG_NAND_CHANNEL_NUM -1)
#define CFG_EP_NUM       ((CFG_NAND_PAGE_SIZE / CFG_NAND_EP_SIZE) - 1)
#define CFG_ROW2_LOW     0x0d0c0b0a
#define CFG_ROW2_HIGH    0x11100f0e
#define CFG_ROW3_LOW     0x13120706
#define CFG_ROW3_HIGH    0x17161514
#define CFG_ROW4_LOW     0x1b1a1918
#define CFG_ROW4_HIGH    0x1f1f1f1c
#define CFG_ROW5_LOW     0x1f1f1f1f
#define CFG_ROW5_HIGH    0x1f1f1f1f

#define CFG_TM_RD_THR          0x18     /* TM_RD_THR */
#define CFG_TM_PR_THR          0x2BC    /* TM_PR_THR */  
#define CFG_TM_ERS_THR         0x5DC    /* TM_ERS_THR */ 
#define CFG_PRG_THR            0x10     /* PRG_THR */ 
#define CFG_SPPA_PL_LOC        0x8

#define LUN_OFFSET        (CH_BITS+EP_BITS+PL_BITS) /* offset=8 */

#define SEQ_ENTRY_NUM_MAX  (512)
static unsigned int seq_cfg_array[SEQ_ENTRY_NUM_MAX] = 
{
    0x58dd0003,
    0x590f0003,
    0x59400003,
    0x58960003,
    0x58c88013,
    0x58d28013,
    0x58700003,
    0x588c0003,
    0x59680003,
    0x597c0003,
    0x58400003,
    0x59a40003,
    0x59900003,
    0x59b80003,
    0x59c80003,
    0x582f0003,
    0x59e00003,
    0xf0000003,
    0xf0000003,
    0x58e30003,
    0x58e70003,
    0x582f0003,
    0x58700003,
    0x58940003,
    0xf0000003,
    0xf0000003,
    0x58a40003,
    0xf0000003,
    0xf0000003,
    0x58f00003,
    0x58fa0003,
    0x59c90003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x59d60003,
    0x59db0003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01fa0003,
    0x00000393,
    0x30020b96,
    0x30010b97,
    0x00000a13,
    0x30020a1a,
    0x30010a1b,
    0x00000a3b,
    0x30020a3a,
    0x30010a3b,
    0x00000a5b,
    0x30020a5a,
    0x30010a5b,
    0x00000a5b,
    0x30010a4b,
    0xf0000003,
    0xf0000003,
    0x01ef0003,
    0x00000b93,
    0x30020b96,
    0x30010b97,
    0x00000bd3,
    0x30020bda,
    0x30010bdb,
    0x30068a1b,
    0x300b831b,
    0x09028013,
    0x301f8313,
    0x09008313,
    0x09028b13,
    0x09008333,
    0x09028b33,
    0x09008353,
    0x09028b53,
    0x09008373,
    0x09028b73,
    0x300f8013,
    0x30038003,
    0x30960003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01600003,
    0x503d0013,
    0x58300013,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01610003,
    0x503c0a13,
    0x58300a13,
    0x30010a4b,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01d00003,
    0x51260003,
    0x59230003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01810013,
    0x58970013,
    0x01800013,
    0x30020b96,
    0x30010b97,
    0x51210a13,
    0x59140a73,
    0x300c821b,
    0x09028013,
    0x301e8013,
    0x10088813,
    0x33e68813,
    0x20008813,
    0x33208813,
    0xf8008013,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x30018017,
    0x09068017,
    0x01118a17,
    0x00008b97,
    0x30028b96,
    0x30018b97,
    0x300a8b97,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x30038017,
    0x09068017,
    0x01108a17,
    0x50cf8b97,
    0x58cb8b97,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x51220003,
    0x590f0003,
    0x01320a53,
    0x51260a53,
    0x59230a53,
    0xf0000003,
    0x01330003,
    0x51260003,
    0x59100003,
    0xf0000003,
    0x01360003,
    0x00008383,
    0x30028b96,
    0x30018b97,
    0x30018b87,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x015c0003,
    0x30080b96,
    0x30050b97,
    0x01c50017,
    0x30080b96,
    0x30050b97,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01550017,
    0x30080b96,
    0x30050b97,
    0x30050bda,
    0x30050bdb,
    0x301e0b13,
    0x30050b12,
    0x30050b13,
    0x30960003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01000003,
    0x00000393,
    0x30020b96,
    0x30010b97,
    0x00000a73,
    0x30020a7a,
    0x30010a7b,
    0x00000a9b,
    0x30020a9a,
    0x30010a9b,
    0x00000a1b,
    0x30020a1a,
    0x30010a1b,
    0x00000a3b,
    0x30020a3a,
    0x30010a3b,
    0x00000a5b,
    0x30020a5a,
    0x30010a5b,
    0x01300a53,
    0x00000b93,
    0x30020b96,
    0x30010b97,
    0x30010007,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x51220003,
    0x590f0003,
    0x01050a53,
    0x51180a53,
    0x59100a53,
    0x01e00a53,
    0x00000b93,
    0x30020b96,
    0x30010b97,
    0x301e0013,
    0x03970013,
    0x05080013,
    0x09010413,
    0x300a0413,
    0x300a0413,
    0x300a0431,
    0x388e4431,
    0x300a4433,
    0x40004413,
    0x300a4403,
    0x09000003,
    0x00000003,
    0x03000003,
    0x05000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x000003d3,
    0x30020bd6,
    0x30010bd7,
    0xf0000003,
    0xf0000003,
    0x01090003,
    0x30020b96,
    0x30010b97,
    0x00000bd3,
    0x30020bd6,
    0x30010bd7,
    0x01800a13,
    0x58970a13,
    0x000003c3,
    0x30020bd6,
    0x30010bd7,
    0x300a0013,
    0x03000013,
    0x09010013,
    0x30030013,
    0x00000631,
    0x30050633,
    0x30030603,
    0x00000e03,
    0x40000603,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01780003,
    0x503c0003,
    0x58300003,
    0x51720013,
    0x596b0013,
    0x40000603,
    0xf0000003,
    0xf0000003,
    0x010d0003,
    0x59610b93,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01900003,
    0x50350003,
    0x58300003,
    0x300a0013,
    0x03050013,
    0x09010013,
    0x30080613,
    0x30060613,
    0x30060631,
    0x30080633,
    0x30010603,
    0x40000603,
    0xf0000003,
    0xf0000003,
    0x30018017,
    0x09068017,
    0x011a8a17,
    0x58cc8b97,
    0xf0000003,
    0xf0000003,
    0x01ee0003,
    0x50470003,
    0x58410003,
    0x30960013,
    0x03030013,
    0x09010013,
    0x30080613,
    0x30060613,
    0x30010631,
    0x30010633,
    0x30010631,
    0x30010633,
    0x30010631,
    0x30010633,
    0x30010631,
    0x30080633,
    0x30010603,
    0x40000603,
    0xf0000003,
    0xf0000003,
    0x01ec0003,
    0x50350003,
    0x58300003,
    0x10200013,
    0x31f40013,
    0x20000013,
    0x03ff0013,
    0x09010013,
    0x30080613,
    0x31002631,
    0x30082633,
    0x30032603,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x07210003,
    0x01ff0013,
    0x30080b96,
    0x30050b97,
    0x30010b87,
    0x07000003,
    0x10c80003,
    0x33e80003,
    0x20000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x51220003,
    0x590f0003,
    0x012e0a53,
    0x58e00a53,
    0xf0000003,
    0x51220003,
    0x590f0003,
    0x012d0a53,
    0x58e00a53,
    0xf0000003,
    0x01ef0003,
    0x30070b96,
    0x30050b97,
    0x30050bda,
    0x30050bdb,
    0x301e0b13,
    0x30050b12,
    0x30050b13,
    0x30050b32,
    0x30050b33,
    0x30050b52,
    0x30050b53,
    0x30050b72,
    0x30050b73,
    0x30960003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
};

// use remove the warning message when make ftl module
inline static void unused_func(void)
{
    (void)seq_cfg_array;
    return;
}

#endif /* CFG_1TB_TH58TFG9DFK_H_ */
