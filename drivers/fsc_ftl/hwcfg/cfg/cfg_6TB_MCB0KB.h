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
 */

#ifndef CFG_6TB_MC_B0KB_H_
#define CFG_6TB_MC_B0KB_H_

#include "cfg_common.h"

#define FLASH_TYPE  "6TB_MC_B0KB"
#define ENABLE_TWO_PAGE_PROGRAM     1

#define FLASH_MC_B0KB_S_PL_MODE  /*if defined, single plane for read and erase*/

#define FLASH_MCBOKB
#define BOKB_2_8 1
#define BOKB_4_8 0

#if (BOKB_4_8 == BOKB_2_8)
#error "Choose BOKB type 2-8 or 4-8!"
#endif

#if ENABLE_TWO_PAGE_PROGRAM
#define ENABLE_BOKB_SKIP_PG	    0
#else
#define ENABLE_BOKB_SKIP_PG	    1
#endif

//#define USE_SLC_FOR_BOOT_BLK     //use slc for boot blk by default
#define ENABLE_TLC              0
#define NEXUS_CMD_CONTROL_M_PL  0x02
#define NEXUS_CMD_CONTROL_S_PL  0x00


#define CAPACITY_TOTAL (((u64)1<<41)*3/(4))        /* = 6T  LUN  BLK*/

#define CAPACITY CAPACITY_TOTAL

#define CH_BITS   4
#define EP_BITS   2
#define PL_BITS   2
#define LN_BITS   2
#define PG_BITS   11
#define BL_BITS   9

#define CFG_NAND_EP_NUM              (1<<EP_BITS)
#define CFG_NAND_CHANNEL_NUM         16
#define CFG_NAND_LUN_NUM             2 //(1 << LN_BITS)//1//8
#define CFG_NAND_BLOCK_NUM           511 // will change to 548 in next step

#define CFG_NAND_PAGE_NUM            1536
#define CFG_NAND_PLANE_NUM           4
#define CFG_NAND_PAGE_SIZE           16384
#define CFG_NAND_PAGE_SPARE          1260
#define CFG_NAND_EP_SIZE             4096

#define CFG_NAND_BAD_MARKER_PAGE_CNT 2
#define CFG_NAND_BAD_MARKER_PAGE_0   0
#define CFG_NAND_BAD_MARKER_PAGE_1   255
#define CFG_NAND_BAD_MARKER_PAGE_2   0xFFFF
#define CFG_NAND_BAD_MARKER_PAGE_3   0xFFFF

#define CFG_NAND_PADDING_PAGES       300

// Column: retention  month0 month1 month2 month3
// Line:   pecycle    step 500 every line
#define PE_DIV_500       36      // Line number
#define RETENTION_MONTH  4	 // Column number

#define BOKB_SKIP_PG_NUM	32	
#define BOKB_SKIP_PG_TABLE	{16, 18, 20, 22, 24, 26, 28, 30, 32, 34, \
				36, 38, 40, 42, 44, 46, 1457, 1460, 1463, 1466,\
				1469, 1472, 1475, 1478, 1481, 1484, 1487, 1490, 1493, 1496,\
				1499, 1502}


#if (BOKB_2_8 == 1)
#define BOKB_SKIP_PG_TABLE_INDEX	{16, 18, 20, 22, 24, 26, 28, 30, 32, 34, \
					36, 38, 40, 42, 44, 46, 1426, 1430, 1434, 1438,\
					1442, 1446, 1450, 1454, 1458, 1462,1466, 1470, 1474, 1478,\
					1482, 1486}
#elif (BOKB_4_8 == 1)
#define BOKB_SKIP_PG_TABLE_INDEX	{16, 18, 20, 22, 24, 26, 28, 30, 32, 34, \
					36, 38, 40, 42, 44, 46, 1457, 1460, 1463, 1466,\
					1469, 1472, 1475, 1478, 1481, 1484, 1487, 1490, 1493, 1496,\
					1499, 1502}
#endif

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
#define CFG_NAND_LAST_SLC_VALID_PG  (511)
#define CFG_NAND_SLC_PAGE_NUM	   512
#define CFG_NAND_SLC_PAGE_TABLE    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, \
 				10, 11, 12, 13, 14, 15, 16, 17, 18, 19, \
 				20, 21, 22, 23, 24, 25, 26, 27, 28, 29, \
 				30, 31, 32, 33, 34, 35, 36, 37, 38, 39, \
 				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, \
 				50, 51, 52, 53, 54, 55, 56, 57, 58, 59, \
 				60, 61, 62, 63, 64, 65, 66, 67, 68, 69, \
 				70, 71, 72, 73, 74, 75, 76, 77, 78, 79, \
 				80, 81, 82, 83, 84, 85, 86, 87, 88, 89, \
 				90, 91, 92, 93, 94, 95, 96, 97, 98, 99, \
 				100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
 				110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
 				120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
 				130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
 				140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
 				150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
 				160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
 				170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
 				180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
 				190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
 				200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
 				210, 211, 212, 213, 214, 215, 216, 217, 218, 219, \
 				220, 221, 222, 223, 224, 225, 226, 227, 228, 229, \
				230, 231, 232, 233, 234, 235, 236, 237, 238, 239, \
				240, 241, 242, 243, 244, 245, 246, 247, 248, 249, \
				250, 251, 252, 253, 254, 255, 256, 257, 258, 259, \
				260, 261, 262, 263, 264, 265, 266, 267, 268, 269, \
				270, 271, 272, 273, 274, 275, 276, 277, 278, 279, \
				280, 281, 282, 283, 284, 285, 286, 287, 288, 289, \
				290, 291, 292, 293, 294, 295, 296, 297, 298, 299, \
				300, 301, 302, 303, 304, 305, 306, 307, 308, 309, \
				310, 311, 312, 313, 314, 315, 316, 317, 318, 319, \
				320, 321, 322, 323, 324, 325, 326, 327, 328, 329, \
				330, 331, 332, 333, 334, 335, 336, 337, 338, 339, \
				340, 341, 342, 343, 344, 345, 346, 347, 348, 349, \
				350, 351, 352, 353, 354, 355, 356, 357, 358, 359, \
				360, 361, 362, 363, 364, 365, 366, 367, 368, 369, \
				370, 371, 372, 373, 374, 375, 376, 377, 378, 379, \
				380, 381, 382, 383, 384, 385, 386, 387, 388, 389, \
				390, 391, 392, 393, 394, 395, 396, 397, 398, 399, \
				400, 401, 402, 403, 404, 405, 406, 407, 408, 409, \
				410, 411, 412, 413, 414, 415, 416, 417, 418, 419, \
				420, 421, 422, 423, 424, 425, 426, 427, 428, 429, \
				430, 431, 432, 433, 434, 435, 436, 437, 438, 439, \
				440, 441, 442, 443, 444, 445, 446, 447, 448, 449, \
				450, 451, 452, 453, 454, 455, 456, 457, 458, 459, \
				460, 461, 462, 463, 464, 465, 466, 467, 468, 469, \
				470, 471, 472, 473, 474, 475, 476, 477, 478, 479, \
				480, 481, 482, 483, 484, 485, 486, 487, 488, 489, \
				490, 491, 492, 493, 494, 495, 496, 497, 498, 499, \
				500, 501, 502, 503, 504, 505, 506, 507, 508, 509, \
				510, 511}
#endif

#define CFG_NAND_LAST_VALID_PG  (1535)
#define CFG_NAND_LOWER_PAGE_NUM      512
#define CFG_NAND_LOWER_PAGE_TABLE   \
{  0,    1,    2,    3,    4,    5,    6,    7,    8,    9,  \
   10,   11,   12,   13,   14,   15,   48,   50,   52,   54, \
   56,   58,   60,   62,   64,   66,   68,   70,   72,   74, \
   76,   78,   80,   82,   84,   86,   88,   90,   92,   94, \
   96,   98,  100,  102,  104,  106,  108,  110,  113,  116, \
  119,  122,  125,  128,  131,  134,  137,  140,  143,  146, \
  149,  152,  155,  158,  161,  164,  167,  170,  173,  176, \
  179,  182,  185,  188,  191,  194,  197,  200,  203,  206, \
  209,  212,  215,  218,  221,  224,  227,  230,  233,  236, \
  239,  242,  245,  248,  251,  254,  257,  260,  263,  266, \
  269,  272,  275,  278,  281,  284,  287,  290,  293,  296, \
  299,  302,  305,  308,  311,  314,  317,  320,  323,  326, \
  329,  332,  335,  338,  341,  344,  347,  350,  353,  356, \
  359,  362,  365,  368,  371,  374,  377,  380,  383,  386, \
  389,  392,  395,  398,  401,  404,  407,  410,  413,  416, \
  419,  422,  425,  428,  431,  434,  437,  440,  443,  446, \
  449,  452,  455,  458,  461,  464,  467,  470,  473,  476, \
  479,  482,  485,  488,  491,  494,  497,  500,  503,  506, \
  509,  512,  515,  518,  521,  524,  527,  530,  533,  536, \
  539,  542,  545,  548,  551,  554,  557,  560,  563,  566, \
  569,  572,  575,  578,  581,  584,  587,  590,  593,  596, \
  599,  602,  605,  608,  611,  614,  617,  620,  623,  626, \
  629,  632,  635,  638,  641,  644,  647,  650,  653,  656, \
  659,  662,  665,  668,  671,  674,  677,  680,  683,  686, \
  689,  692,  695,  698,  701,  704,  707,  710,  713,  716, \
  719,  722,  725,  728,  731,  734,  737,  740,  743,  746, \
  749,  752,  755,  758,  761,  764,  767,  770,  773,  776, \
  779,  782,  785,  788,  791,  794,  797,  800,  803,  806, \
  809,  812,  815,  818,  821,  824,  827,  830,  833,  836, \
  839,  842,  845,  848,  851,  854,  857,  860,  863,  866, \
  869,  872,  875,  878,  881,  884,  887,  890,  893,  896, \
  899,  902,  905,  908,  911,  914,  917,  920,  923,  926, \
  929,  932,  935,  938,  941,  944,  947,  950,  953,  956, \
  959,  962,  965,  968,  971,  974,  977,  980,  983,  986, \
  989,  992,  995,  998, 1001, 1004, 1007, 1010, 1013, 1016, \
 1019, 1022, 1025, 1028, 1031, 1034, 1037, 1040, 1043, 1046, \
 1049, 1052, 1055, 1058, 1061, 1064, 1067, 1070, 1073, 1076, \
 1079, 1082, 1085, 1088, 1091, 1094, 1097, 1100, 1103, 1106, \
 1109, 1112, 1115, 1118, 1121, 1124, 1127, 1130, 1133, 1136, \
 1139, 1142, 1145, 1148, 1151, 1154, 1157, 1160, 1163, 1166, \
 1169, 1172, 1175, 1178, 1181, 1184, 1187, 1190, 1193, 1196, \
 1199, 1202, 1205, 1208, 1211, 1214, 1217, 1220, 1223, 1226, \
 1229, 1232, 1235, 1238, 1241, 1244, 1247, 1250, 1253, 1256, \
 1259, 1262, 1265, 1268, 1271, 1274, 1277, 1280, 1283, 1286, \
 1289, 1292, 1295, 1298, 1301, 1304, 1307, 1310, 1313, 1316, \
 1319, 1322, 1325, 1328, 1331, 1334, 1337, 1340, 1343, 1346, \
 1349, 1352, 1355, 1358, 1361, 1364, 1367, 1370, 1373, 1376, \
 1379, 1382, 1385, 1388, 1391, 1394, 1397, 1400, 1403, 1406, \
 1409, 1412, 1415, 1418, 1421, 1424, 1427, 1430, 1433, 1436, \
 1439, 1442, 1445, 1448, 1451, 1454, 1505, 1507, 1509, 1511, \
 1513, 1515, 1517, 1519, 1521, 1523, 1525, 1527, 1529, 1531, \
 1533, 1535}

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
#define CFG_DRIVE_FTLLOG_SEC_CNT     (CFG_NAND_PAGE_NUM * CFG_DRIVE_EP_NUM)  /* 1536 * 4 = 6144 */
#define CFG_DRIVE_FTLLOG_PPA_CNT     (DIV_ROUND_UP(CFG_DRIVE_FTLLOG_SEC_CNT * sizeof(unsigned int), FTL_PPA_SIZE)) /* 6 */
#define CFG_DRIVE_FTLLOG_START_SEC   (CFG_DRIVE_EP_NUM - 1)
#define CFG_DRIVE_FTLLOG_PG_INTERVAL (256)
#define PAGE_IS_FTLLOG_PAGE(_page)		(((0 == (((_page) + 1) % CFG_DRIVE_FTLLOG_PG_INTERVAL)) || ((CFG_NAND_PAGE_NUM - 1) == (_page))) ? (1) : (0))
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
      unsigned int pl        : 2;  // 2 planes per LUN
      unsigned int lun       : LN_BITS;  // 8 LUNs per channel
      unsigned int pg        : 11; // 1536 pages per block	
      unsigned int blk       : 9;  // 512 blocks per plane 
      unsigned int resved    : 1;  // 4 bits reversed        
    } nand;

    // ppa format in write cache
    struct
    {
      unsigned int ch         : 4;  // 16 channels
      unsigned int line       : 4;  // 16 sector lines per LUN
      unsigned int lun        : LN_BITS;  // 8 LUNs per channel
      unsigned int ver        : 11; // max version is defined in software based on application
      unsigned int resved     : 9;  // 9 bits reversed  //SLUO@SJ rename resreved to resved
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

#define ECC_HIGH_THR  66

#define HC_DEMO       1

/********************************************************************************************/

#endif /* CFG_6TB_MC_B0KB_H_ */

