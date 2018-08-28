/*Apollo Controller intra, each blocks and CPU communication via a message*/

#ifndef _MESSAGE_FORMAT_H_
#define _MESSAGE_FORMAT_H_

enum module_src_id {
	SRC_ID_HDC = 0,
	SRC_ID_ATC,
	SRC_ID_STC,
	SRC_ID_FDC,
	SRC_ID_HOST,
	SRC_ID_JTAG,
};

enum module_dst_id {
	DST_ID_HDC = 0,
	DST_ID_ATC,
	DST_ID_STC,
	DST_ID_FDC,
	DST_ID_CPU_PLL,
	DST_ID_CPU_CTL,
	DST_ID_FUSE,
	DST_ID_SHA,
	DST_ID_SPIFI,
	DST_ID_CRYPTO,
	DST_ID_CPU_TSOR,
	DST_ID_NURDI,
	DST_ID_PHIF = 32,
	DST_ID_HXTS,
	DST_ID_HDMA,
	DST_ID_ROC,
	DST_ID_RSCPOOL,
	DST_ID_HMFR,
	DST_ID_OOB_UART,
	DST_ID_OOB_I2C,
	DST_ID_OOB_SMB,
	DST_ID_HP_CTL,
	DST_ID_SYS_CTL,
	DST_ID_HR_TSOR,
	DST_ID_CPMU,
	DST_ID_LED,
	DST_ID_HP_GPIO,
	DST_ID_DBG_UART,
	DST_ID_DBGI2C,
	DST_ID_DPIO,
	DST_ID_TST_CTRL,
	DST_ID_FIF = 64,
	DST_ID_FWMGR
};

// qw0 is various depend on message type
struct msg_qw0 {
	u64 cnt			 :4;
	u64 dstfifo		 :4;
	u64 dst			 :5;
	u64 prio		 :1;
	u64 rsvd0		 :1;
	u64 ckl		 	 :1;   //chunk last
	u64 msgid		 :8;
	u64 tag			 :8;
	u64 ext_tag		 :4;
	u64 src			 :5;
	u64 rsvd1		 :1;
	u64 vfa			 :1;
	u64 port		 :1;
	u64 vf			 :4;
	u64 sqid		 :8;
	u64 hxts_mode	 :6;
	u64 rsvd3		 :2;
};


/*
typedef union
{
    u64 	value;
	struct hnc_header hnc;	  //hdc nvme cmd
	struct pcc_header pcc;    //phif cmd cpl
} msg_qw0;*/


// hdc_nvmd_cmd message from PHIF, when a new host command arrival
typedef struct host_nvme_cmd {
	struct msg_qw0 header;    // QW0
	struct nvme_command sqe; 	// QW1--QW8 is original SQE
} hdc_nvme_cmd;


typedef struct phif_cmd_complete {
	struct msg_qw0 header;
	struct nvme_completion cqe;
} phif_cmd_cpl;


typedef struct phif_cmd_request {
	struct msg_qw0 header;

	// QW1
	u32 cpa;				 // convert from LBA
	u32 hmeta_size		:3;  // 8B * (this value)
	u32 rsvd0			:1;  // zero
	u32 cph_size		:2;  // 16B * (this value)
	u32 rsvd1			:2;	 // zero
	u32 lb_size		    :2;  // 00:512B  01:4KB  10:16KB
	u32 crc_en			:1;
	u32 rsvd2			:1;
	u32 dps				:4;  // PI enable/disable type1/2/3
	u32 flbas			:5;  // Bit4: 1-DIF / 0-DIX,  bit[3:0] LBA formats type
	u32 rsvd3			:2;
	u32 cache_en		:1;
	u32 band_rdtype		:8;  // which band, gc_read/ctl_read/host_read

	//QW2
	u64 elba			:35; // for LB < CP
	u64 rsvd4			:29;
} phif_cmd_req;

#endif
