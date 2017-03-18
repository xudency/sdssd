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
 * $Id: cfg_2TB_MCL95B_sample.h 5807 2016-05-06 10:48:18Z wenzheng $
 * $Rev: 5807 $:     Revision of last commit
 * $Author: wenzheng $:  Author of last commit
 * $HeadURL: http://172.29.0.208/svn/nexus_sw/branches/nexus_ftl_staging_a/cfg/cfg_2TB_MCL95B_sample.h $
 * $Date: 2016-05-06 03:48:18 -0700 (Fri, 06 May 2016) $:    Date of last commit
 *
 */
 

#ifndef CFG_2TB_MC_L95B_H_
#define CFG_2TB_MC_L95B_H_

#include "cfg_common.h"

#define FLASH_TYPE  "2TB_MC_L95B"
#define FLASH_MICRON
#define USE_SLC_FOR_BOOT_BLK     //use slc for boot blk by default
#define ENABLE_TLC              0
#define NEXUS_CMD_CONTROL_M_PL  0x01
#define NEXUS_CMD_CONTROL_S_PL  0x00

#define CAPACITY ((u64)1<<41)       //41 /* = 2T*/


#define CH_BITS   4
#define EP_BITS   2
#define PL_BITS   1
#define LN_BITS   3
#define PG_BITS   9
#define BL_BITS   10

#define CFG_NAND_EP_NUM              (1<<EP_BITS)
#define CFG_NAND_CHANNEL_NUM         16
#define CFG_NAND_LUN_NUM             8
#define CFG_NAND_BLOCK_NUM           1048
#define CFG_NAND_PAGE_NUM            512
#define CFG_NAND_PLANE_NUM           2
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
    #define CFG_NAND_LAST_SLC_VALID_PG  (255)
    #define CFG_NAND_SLC_PAGE_NUM      256
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
                                 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, \
                                 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, \
                                 141 ,142, 143, 144, 145, 146, 147, 148, 149, 150, \
                                 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, \
                                 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, \
                                 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, \
                                 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, \
                                 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, \
                                 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, \
                                 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, \
                                 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, \
                                 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, \
                                 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, \
                                 251, 252, 253, 254, 255}
#endif

#define CFG_NAND_LAST_VALID_PG  (510)
#define CFG_NAND_LOWER_PAGE_NUM      260
#define CFG_NAND_LOWER_PAGE_TABLE    {0, 1, 2, 3, 4, 5, 7, 8, 10, 11, \
					14, 15, 18, 19, 22, 23, 26, 27, 30, 31, \
					34, 35, 38, 39, 42, 43, 46, 47, 50, 51, \
					54, 55, 58, 59, 62, 63, 66, 67, 70, 71, \
					74, 75, 78, 79, 82, 83, 86, 87, 90, 91, \
					94, 95, 98, 99, 102, 103, 106, 107, 110, 111, \
					114, 115, 118, 119, 122, 123, 126, 127, 130, 131, \
					134, 135, 138, 139, 142, 143, 146, 147, 150, 151, \
					154, 155, 158, 159, 162, 163, 166, 167, 170, 171, \
					174, 175, 178, 179, 182, 183, 186, 187, 190, 191, \
					194, 195, 198, 199, 202, 203, 206, 207, 210, 211, \
					214, 215, 218, 219, 222, 223, 226, 227, 230, 231, \
					234, 235, 238, 239, 242, 243, 246, 247, 250, 251, \
					254, 255, 258, 259, 262, 263, 266, 267, 270, 271, \
					274, 275, 278, 279, 282, 283, 286, 287, 290, 291, \
					294, 295, 298, 299, 302, 303, 306, 307, 310, 311, \
					314, 315, 318, 319, 322, 323, 326, 327, 330, 331, \
					334, 335, 338, 339, 342, 343, 346, 347, 350, 351, \
					354, 355, 358, 359, 362, 363, 366, 367, 370, 371, \
					374, 375, 378, 379, 382, 383, 386, 387, 390, 391, \
					394, 395, 398, 399, 402, 403, 406, 407, 410, 411, \
					414, 415, 418, 419, 422, 423, 426, 427, 430, 431, \
					434, 435, 438, 439, 442, 443, 446, 447, 450, 451, \
					454, 455, 458, 459, 462, 463, 466, 467, 470, 471, \
					474, 475, 478, 479, 482, 483, 486, 487, 490, 491, \
					494, 495, 498, 499, 502, 503, 506, 507, 509, 510}


#ifdef USE_SLC_FOR_BOOT_BLK
	#define BOOT_PAGE_NUM	CFG_NAND_SLC_PAGE_NUM
#else
	#define BOOT_PAGE_NUM	CFG_NAND_LOWER_PAGE_NUM
#endif

#define CFG_NAND_MAX_PE              14000
#define CFG_NAND_READ_DISTURB_CNT    100000

#define CFG_DRIVE_LUN_NUM            CFG_NAND_LUN_NUM
#define CFG_DRIVE_CH_NUM             CFG_NAND_CHANNEL_NUM
#define CFG_DRIVE_EP_SIZE            CFG_NAND_EP_SIZE
#define CFG_DRIVE_EP_NUM             (CFG_NAND_PAGE_SIZE / CFG_DRIVE_EP_SIZE)
#define CFG_DRIVE_LINE_NUM           (CFG_NAND_PLANE_NUM * CFG_DRIVE_EP_NUM)
#define CFG_DRIVE_TOTAL_SEC_NUM      (CFG_NAND_PAGE_NUM * CFG_DRIVE_EP_NUM)
#define CFG_DRIVE_FTLLOG_SEC_CNT     (CFG_NAND_PAGE_NUM * CFG_DRIVE_EP_NUM)  /* 512*4 = 2048*/
#define CFG_DRIVE_FTLLOG_PPA_CNT     (DIV_ROUND_UP(CFG_DRIVE_FTLLOG_SEC_CNT * sizeof(unsigned int), FTL_PPA_SIZE)) /* 2 */
#define CFG_DRIVE_FTLLOG_START_SEC   (CFG_DRIVE_EP_NUM - 1)
#define CFG_DRIVE_FTLLOG_PG_INTERVAL (256)
#define PAGE_IS_FTLLOG_PAGE(_page)	(((0 == (((_page) + 1) % CFG_DRIVE_FTLLOG_PG_INTERVAL)) || ((CFG_NAND_PAGE_NUM - 1) == (_page))) ? (1) : (0))
#define PAGE_NOT_FTLLOG_PAGE(_page)	(((0 == (((_page) + 1) % CFG_DRIVE_FTLLOG_PG_INTERVAL)) || ((CFG_NAND_PAGE_NUM - 1) == (_page))) ? (0) : (1))

#define CFG_OOB_RAW_PHY_ADDR_OFFSET  0xc70


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


// ppa address structure for current flash type
struct physical_address
{
  union
  {
    // ppa format in nand
    struct
    {
      unsigned int ch        : 4;  // 16 channels
      unsigned int sec       : 2;  // 4 sectors per page
      unsigned int pl        : 1;  // 2 planes per LUN
      unsigned int lun       : 3;  // 8 LUNs per channel
      unsigned int pg        : 9;  // 512 pages per block	//SLUO@SJ rename page to pg
      unsigned int blk       : 12; // 1024 blocks per plane  //SLUO@SJ rename bl to blk
      unsigned int resved    : 1;  // 4 bits reversed        //SLUO@SJ rename resreved to resved
    } nand;

    // ppa format in write cache
    struct
    {
      unsigned int ch         : 4;  // 16 channels
      unsigned int line       : 3;  // 8 sector lines per LUN
      unsigned int lun        : 3;  // 8 LUNs per channel
      unsigned int ver        : 9; // max version is defined in software based on application
      unsigned int resved     : 12;  // 9 bits reversed  //SLUO@SJ rename resreved to resved
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

#define PAIR_PG_NUM_MAX (252)
#define PAIR_MEM_NUM    (2)
#define CFG_THR_REF_PAIR_PG_TBL \
{ \
	{1,6},{5,9},{7,12},{8,13},{10,16},{11,17},{14,20},{15,21},{18,24},{19,25},\
	{22,28},{23,29},{26,32},{27,33},{30,36},{31,37},{34,40},{35,41},{38,44},{39,45},\
	{42,48},{43,49},{46,52},{47,53},{50,56},{51,57},{54,60},{55,61},{58,64},{59,65},\
	{62,68},{63,69},{66,72},{67,73},{70,76},{71,77},{74,80},{75,81},{78,84},{79,85},\
	{82,88},{83,89},{86,92},{87,93},{90,96},{91,97},{94,100},{95,101},{98,104},{99,105},\
	{102,108},{103,109},{106,112},{107,113},{110,116},{111,117},{114,120},{115,121},{118,124},{119,125},\
	{122,128},{123,129},{126,132},{127,133},{130,136},{131,137},{134,140},{135,141},{138,144},{139,145},\
	{142,148},{143,149},{146,152},{147,153},{150,156},{151,157},{154,160},{155,161},{158,164},{159,165},\
	{162,168},{163,169},{166,172},{167,173},{170,176},{171,177},{174,180},{175,181},{178,184},{179,185},\
	{182,188},{183,189},{186,192},{187,193},{190,196},{191,197},{194,200},{195,201},{198,204},{199,205},\
	{202,208},{203,209},{206,212},{207,213},{210,216},{211,217},{214,220},{215,221},{218,224},{219,225},\
	{222,228},{223,229},{226,232},{227,233},{230,236},{231,237},{234,240},{235,241},{238,244},{239,245},\
	{242,248},{243,249},{246,252},{247,253},{250,256},{251,257},{254,260},{255,261},{258,264},{259,265},\
	{262,268},{263,269},{266,272},{267,273},{270,276},{271,277},{274,280},{275,281},{278,284},{279,285},\
	{282,288},{283,289},{286,292},{287,293},{290,296},{291,297},{294,300},{295,301},{298,304},{299,305},\
	{302,308},{303,309},{306,312},{307,313},{310,316},{311,317},{314,320},{315,321},{318,324},{319,325},\
	{322,328},{323,329},{326,332},{327,333},{330,336},{331,337},{334,340},{335,341},{338,344},{339,345},\
	{342,348},{343,349},{346,352},{347,353},{350,356},{351,357},{354,360},{355,361},{358,364},{359,365},\
	{362,368},{363,369},{366,372},{367,373},{370,376},{371,377},{374,380},{375,381},{378,384},{379,385},\
	{382,388},{383,389},{386,392},{387,393},{390,396},{391,397},{394,400},{395,401},{398,404},{399,405},\
	{402,408},{403,409},{406,412},{407,413},{410,416},{411,417},{414,420},{415,421},{418,424},{419,425},\
	{422,428},{423,429},{426,432},{427,433},{430,436},{431,437},{434,440},{435,441},{438,444},{439,445},\
	{442,448},{443,449},{446,452},{447,453},{450,456},{451,457},{454,460},{455,461},{458,464},{459,465},\
	{462,468},{463,469},{466,472},{467,473},{470,476},{471,477},{474,480},{475,481},{478,484},{479,485},\
	{482,488},{483,489},{486,492},{487,493},{490,496},{491,497},{494,500},{495,501},{498,504},{499,505},\
	{503,508},{507,511},\
}

typedef enum {
	MLC_IDX_START = 0,
    T1_IDX = 0,
    T2_IDX,
    T3_IDX,
    LPO_IDX,
} thr_seq;

#define THR_MLC_IDX_ARRAY {T1_IDX, T2_IDX, T3_IDX}
#define THR_LPO_IDX_ARRAY {LPO_IDX}

#define RD_RETRY_FEATURE_ADDR  (0xa0)
/********************************************************************************************/


/* firmware define */
#define FLASH_TYPE_VALUE (2)

#define PPAF_CH_LSB      0
#define PPAF_EP_LSB      4
#define PPAF_PL_LSB      8
#define PPAF_LN_LSB      12
#define PPAF_PG_LSB      16
#define PPAF_BL_LSB      20

#define CFG_LUN_PER_CE   0x1
#define CFG_PPA_FORMAT   ((CH_BITS<<PPAF_CH_LSB) | \
                          (EP_BITS<<PPAF_EP_LSB) | \
                          (PL_BITS<<PPAF_PL_LSB) | \
                          (LN_BITS<<PPAF_LN_LSB) | \
                          (PG_BITS<<PPAF_PG_LSB) | \
                          (11<<PPAF_BL_LSB))
#define CFG_PLANE_NUM    (CFG_NAND_PLANE_NUM - 1)
#define CFG_LUN_NUM      (CFG_NAND_LUN_NUM - 1)
#define CFG_PAGE_NUM     (CFG_NAND_PAGE_NUM - 1)
#define CFG_BLOCK_NUM    (CFG_NAND_BLOCK_NUM - 1)
#define CFG_CHANNEL_NUM  (CFG_NAND_CHANNEL_NUM - 1)
#define CFG_EP_NUM       ((CFG_NAND_PAGE_SIZE / CFG_DRIVE_EP_SIZE) - 1)
#define CFG_ROW2_LOW     0x0d0c0b0a
#define CFG_ROW2_HIGH    0x11100f0e
#define CFG_ROW3_LOW     0x14130612
#define CFG_ROW3_HIGH    0x18171615
#define CFG_ROW4_LOW     0x1c1b1a19
#define CFG_ROW4_HIGH    0x1f1f071d
#define CFG_ROW5_LOW     0x1f1f1f1f
#define CFG_ROW5_HIGH    0x1f1f1f1f

#define CFG_TM_RD_THR          0x18     /* TM_RD_THR */
#define CFG_TM_PR_THR          0x2BC    /* TM_PR_THR */  
#define CFG_TM_ERS_THR         0x5DC    /* TM_ERS_THR */ 
#define CFG_PRG_THR            0x10     /* PRG_THR */   
#define CFG_SPPA_PL_LOC        0x9

#define LUN_OFFSET        (CH_BITS+EP_BITS+PL_BITS) /* offset=7 */

#define SEQ_ENTRY_NUM_MAX  (512)

static unsigned int seq_cfg_array[SEQ_ENTRY_NUM_MAX] = 
{
    0x58dd0003,
    0x590f0003,
    0x59400003,
    0x587e0003,
    0x58c88013,
    0x58d28013,
    0x58700003,
    0x58a00003,
    0x59680003,
    0x597c0003,
    0x58400003,
    0x59a40003,
    0x59900003,
    0x59b80003,
    0x59c80003,
    0x582f0003,
    0x59e00003,
    0x58610003,
    0x58580003,
    0x58e30003,
    0x58e70003,
    0x58760003,
    0x58700003,
    0x587e0003,
    0x59280003,
    0x59300003,
    0x58a40003,
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
    0x588d0003,
    0x59d60003,
    0x59db0003,
    0x588e0003,
    0xf0000003,
    0x59f10003,
    0x586a0003,
    0xf0000003,
    0x59f40003,
    0x01fa0003,
    0x00008b93,
    0x30028b96,
    0x30018b97,
    0x00008a13,
    0x30028a1a,
    0x30018a1b,
    0x00008a3b,
    0x30028a3a,
    0x30018a3b,
    0x00008a5b,
    0x30028a5a,
    0x30018a5b,
    0x00008a5b,
    0x30018a4b,
    0xf0000003,
    0xf0000003,
    0x01ef0003,
    0x00008b93,
    0x30028b96,
    0x30018b97,
    0x00008bd3,
    0x30028bda,
    0x30018bdb,
    0x30068a1b,
    0x300b831b,
    0x09028013,
    0x300b8313,
    0x00008313,
    0x30018b13,
    0x30018b33,
    0x30018b53,
    0x30018b73,
    0x300f8013,
    0x30018003,
    0x30960003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01e10003,
    0x00008b93,
    0x30028b96,
    0x30018b97,
    0x00008b13,
    0x30028b1a,
    0x30018b1b,
    0x30058b13,
    0xf0000003,
    0x01e10003,
    0x30070b96,
    0x30050b97,
    0x30050b1a,
    0x30050b1b,
    0x30050b13,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01d40003,
    0x50960003,
    0x58910003,
    0x30028bda,
    0x30018bdb,
    0x59a78013,
    0x01600003,
    0x503d8a13,
    0x58308a13,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01610003,
    0x503c8a13,
    0x58308a13,
    0x30018a4b,
    0xf0000003,
    0xf0000003,
    0x01810013,
    0x587f0013,
    0x01800013,
    0x09080013,
    0x30028b96,
    0x30018b97,
    0x51228a13,
    0x59148a73,
    0x09028013,
    0x30108013,
    0x10088813,
    0x33e68813,
    0x20008813,
    0x33208813,
    0xf8008013,
    0xf0000003,
    0xf0000003,
    0x588f0003,
    0x03020003,
    0x0b210003,
    0x01d50003,
    0x30028b96,
    0x30018b97,
    0x00008bf3,
    0x30028bfa,
    0x30018bfb,
    0x01918b93,
    0x30028b9a,
    0x30018b9b,
    0x504b8a1b,
    0x58478a1b,
    0x01018393,
    0x30018ad3,
    0x30018b93,
    0x58d88af3,
    0xf0000003,
    0x01d00003,
    0x51260003,
    0x59230003,
    0xf0000003,
    0x50440003,
    0x58400003,
    0x01a08b93,
    0x30028b9a,
    0x30018b9b,
    0x504b8a1b,
    0x58478a1b,
    0x30018b13,
    0x30038af3,
    0x300f8013,
    0x30018003,
    0x30960003,
    0x09000003,
    0x50440003,
    0x58400003,
    0x01a18b93,
    0x30028b9a,
    0x30018b9b,
    0x504b8a1b,
    0x58478a1b,
    0x30018b33,
    0x30038af3,
    0x300f8013,
    0x30018003,
    0x30960003,
    0x09000003,
    0x50440003,
    0x58400003,
    0x01a28b93,
    0x30028b9a,
    0x30018b9b,
    0x504b8a1b,
    0x58478a1b,
    0x30018b53,
    0x30038af3,
    0x58508013,
    0x30028017,
    0x09008017,
    0x01118a17,
    0x00008b97,
    0x30028b96,
    0x30018b97,
    0x30018007,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x30028017,
    0x09008017,
    0x01108a17,
    0x50ce8b97,
    0x58cb8b97,
    0xf0000003,
    0x30018af3,
    0x300f8013,
    0x30018003,
    0x30960003,
    0xf0000003,
    0x51220003,
    0x590f0003,
    0x01328a53,
    0x51268a53,
    0x59238a53,
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
    0x01000003,
    0x00008393,
    0x30028b96,
    0x30018b97,
    0x00008a73,
    0x30028a7a,
    0x30018a7b,
    0x00008a9b,
    0x30028a9a,
    0x30018a9b,
    0x00008a1b,
    0x30028a1a,
    0x30018a1b,
    0x00008a3b,
    0x30028a3a,
    0x30018a3b,
    0x00008a5b,
    0x30028a5a,
    0x30018a5b,
    0x01308a53,
    0x00008b93,
    0x30028b96,
    0x30018b97,
    0x30018007,
    0xf0000003,
    0x01840013,
    0x09080013,
    0x30028b96,
    0x30018b97,
    0x51228a13,
    0x59148a73,
    0xf0008013,
    0xf0000003,
    0x01130013,
    0x59290013,
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
    0x01058a53,
    0x51198a53,
    0x59108a53,
    0x01e08a53,
    0x00008b93,
    0x30028b96,
    0x30018b97,
    0x301e8013,
    0x03978013,
    0x05088013,
    0x09018413,
    0x30018413,
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
    0x587f0a13,
    0x000003c3,
    0x30028bd6,
    0x30018bd7,
    0x300a8613,
    0x03008013,
    0x09018013,
    0x30018613,
    0x30050613,
    0x30050613,
    0x30010631,
    0x300a0633,
    0x30010e13,
    0x30080613,
    0x30010603,
    0x40000603,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01780003,
    0x503c0003,
    0x58300003,
    0x51758613,
    0x596b8613,
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
    0x50470003,
    0x58410003,
    0x300a8013,
    0x03058013,
    0x09018013,
    0x30018613,
    0x30070613,
    0x30060613,
    0x30060631,
    0x30080633,
    0x30010603,
    0x40000603,
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
    0x30968013,
    0x03038013,
    0x09018013,
    0x30018613,
    0x30070613,
    0x30060613,
    0x30040631,
    0x30080633,
    0x30010603,
    0x40000603,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0xf0000003,
    0x01ec0003,
    0x50470003,
    0x58420003,
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
    0x50960003,
    0x58900003,
    0x58448bd3,
    0x0dc00003,
    0x0dc00003,
    0x0dc00003,
    0x0dc00003,
    0x0dc00003,
    0x0dc00003,
    0x70010003,
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

#endif /* CFG_2TB_MC_L85A_H_ */

