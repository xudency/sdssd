/*Apollo Controller intra, each blocks and CPU communication via a message*/

#ifndef _MESSAGE_FORMAT_H_
#define _MESSAGE_FORMAT_H_


// message node(block/module) ID
enum msg_nid {
	MSG_NID_HDC = 0x1,
	MSG_NID_ATC,
	MSG_NID_PHIF,
	MSG_NID_HXTS,
	MSG_NID_HDMA,
	MSG_NID_ROC,
	MSG_NID_SLOTMGR,
	MSG_NID_STC,
	MSG_NID_FDC,
	MSG_NID_FIF,
	MSG_NID_LDPCD,
	MSG_NID_FTLA,
	MSG_NID_FRWMGR,
	MSG_NID_FDMA,
	MSG_NID_RAIF,
};

// message opcode
enum msg_id {
	// CPU 0x10
	MSGID_HDC_NVME_CMD = 0x10,		
	MSGID_ATC_WRITE_RSP = 0x11,
	MSGID_ATC_FQMGR_QOS_REQ = 0x12,	
	MSGID_ATC_FQMGR_QOS_RSP = 0x13,	
	MSGID_STC_READ_RSP = 0x14,	
	MSGID_FDC_WRITE_REQ = 0x15,
	MSGID_FDC_WRITE_RSP = 0x16,		
	MSGID_FDC_RUS_UPD = 0x17,

	// PHIF 0x20
	MSGID_PHIF_CMD_REQ = 0x20,
	MSGID_PHIF_CMD_RSP = 0x21,	
	MSGID_PHIF_CMD_CPL = 0x22,	
	MSGID_PHIF_MSG_SEND = 0x23,
	MSGID_PHIF_MSG_RCVD = 0x24,	
	MSGID_PHIF_RDMA_REQ = 0x25,	
	MSGID_PHIF_RDMA_RSP = 0x26,
	MSGID_PHIF_WDMA_REQ = 0x27,	
	MSGID_PHIF_WDMA_RSP = 0x28,

	// HXTS 0x30
	MSGID_HXTS_RDMA_REQ = 0x30,
	MSGID_HXTS_LPBK_REQ = 0x31,
	MSGID_HXTS_LPBK_RSP = 0x32,

	// HDMA 0x40
	MSGID_HDMA_REQ = 0x40,
	MSGID_HDMA_RSP = 0x41,
	
	// ROC 0x50
	MSGID_ROC_OVLAP_ADD = 0x50,
	MSGID_ROC_OVLAP_GRT = 0x51,
	MSGID_ROC_OVLAP_DEL = 0x52,

	// SLOTMGR 0x60
	MSGID_SLOTMGR_GET = 0x60,
	MSGID_SLOTMGR_GRT = 0x61,
	MSGID_SLOTMGR_REL = 0x62,
	MSGID_SLOTMGR_UPD_REQ = 0x63,
	MSGID_SLOTMGR_UPD_RSP = 0x64,
	MSGID_SLOTMGR_STATE_REQ = 0x65,
	MSGID_SLOTMGR_STATE_RSP = 0x66,

	// FTLA 0x70
	MSGID_FTLA_FIND_REQ = 0x70,
	MSGID_FTLA_FIND_RSP = 0x71,
	MSGID_FTLA_UPD_REQ = 0x72,
	MSGID_FTLA_UPD_RSP = 0x73,
	MSGID_FTLA_REC_WR_REQ = 0x76,
	MSGID_FTLA_REC_WR_RSP = 0x77,
	MSGID_FTLA_FLASH_RD_KICK = 0x78,
	MSGID_FTLA_FLASH_WR_KICK = 0x79,
	MSGID_FTLA_FLASH_WR_DONE = 0x7b,

	// FRWMGR 0x80
	MSGID_FWMGR_WRITE_REQ = 0x80,
	MSGID_FWMGR_WRITE_RSP = 0x81,
	MSGID_FRMGR_READ_REQ = 0x82,
	MSGID_FRMGR_READ_RSP = 0x83,
	MSGID_FRMGR_REC_REQ = 0x84,
	MSGID_FRMGR_REC_RSP = 0x85,
	MSGID_FWMGR_REC_REQ = 0x86,
	MSGID_FWMGR_REC_RSP = 0x87,
	MSGID_FRMGR_FLASH_RD_DONE = 0x8a,

	// FDMA 0x90
	MSGID_FDMA_REQ = 0x90,
	MSGID_FDMA_RSP = 0x91,

	// FQMGR


	// RAIF 0xb0
	MSGID_RAIF_REQ = 0xb0,
	MSGID_RAIF_RSP = 0xb1,

	// LDPCDEC
	MSGID_LDPCDEC_REQ = 0xc0,
	MSGID_LDPCDEC_RSP = 0xc1,
	MSGID_FLASH_RD_DONE = 0xc2,

	// FIF

};

enum extag_id {
	PHIF_EXTAG = 0,

};



/*
`define REG_SRC_HDC        8'h0

// Source ID for ATC
`define REG_SRC_ATC        8'h1

// Source ID for STC
`define REG_SRC_STC        8'h2

// Source ID for FDC
`define REG_SRC_FDC        8'h3

// Source ID for HOST
`define REG_SRC_HOST        8'h4

// Source ID for JTAG
`define REG_SRC_JTAG        8'h5

// Source ID for HDC
`define REG_DST_HDC        8'h0

// Source ID for ATC
`define REG_DST_ATC        8'h1

// Source ID for STC
`define REG_DST_STC        8'h2

// Source ID for FDC
`define REG_DST_FDC        8'h3

// Source ID for CPU_PLL
`define REG_DST_CPU_PLL        8'h4

// Source ID for CPU_CTL
`define REG_DST_CPU_CTL        8'h5

// Source ID for FUSE
`define REG_DST_FUSE        8'h6

// Source ID for SHA
`define REG_DST_SHA        8'h7

// Source ID for SPIFI
`define REG_DST_SPIFI        8'h8

// Source ID for CRYPTO
`define REG_DST_CRYPTO        8'h9

// Source ID for CPU_TSOR
`define REG_DST_CPU_TSOR        8'ha

// Source ID for NURDI
`define REG_DST_NURDI        8'hb

// Source ID for PHIF
`define REG_DST_PHIF        8'h20

// Source ID for HXTS
`define REG_DST_HXTS        8'h21

// Source ID for HDMA
`define REG_DST_HDMA        8'h22

// Source ID for ROC
`define REG_DST_ROC        8'h23

// Source ID for SLOTMGR
`define REG_DST_SLOTMGR        8'h24

// Source ID for HMFR
`define REG_DST_HMFR        8'h25

// Source ID for OOB_UART
`define REG_DST_OOB_UART        8'h26

// Source ID for OOB_I2C
`define REG_DST_OOB_I2C        8'h27

// Source ID for OOB_SMB
`define REG_DST_OOB_SMB        8'h28

// Source ID for HP_CTL
`define REG_DST_HP_CTL        8'h29

// Source ID for SYS_PLL
`define REG_DST_SYS_PLL        8'h2A

// Source ID for HP_TSOR
`define REG_DST_HP_TSOR        8'h2B

// Source ID for CPMU
`define REG_DST_CPMU        8'h2C

// Source ID for LED
`define REG_DST_LED        8'h2D

// Source ID for HP_GPIO
`define REG_DST_HP_GPIO        8'h2E

// Source ID for DBG_UART
`define REG_DST_DBG_UART        8'h2F

// Source ID for DBG_I2C
`define REG_DST_DBG_I2C        8'h30

// Source ID for DPIO
`define REG_DST_DPIO        8'h31

// Source ID for TST_CTL
`define REG_DST_TST_CTL        8'h32

// Source ID for FIF
`define REG_DST_FIF        8'h40

// Source ID for FRWMGR
`define REG_DST_FRWMGR        8'h41

// Source ID for FTLA
`define REG_DST_FTLA        8'h42

// Source ID for FCM
`define REG_DST_FCM        8'h43

// Source ID for FQMGR
`define REG_DST_FQMGR        8'h44

// Source ID for FMFR
`define REG_DST_FMFR        8'h45

// Source ID for LDPCD
`define REG_DST_LDPCD        8'h46

// Source ID for FLASH_PLL
`define REG_DST_FLASH_PLL        8'h47

// Source ID for LDPCD_PLL
`define REG_DST_LDPCD_PLL        8'h48

// Source ID for FP_CTL
`define REG_DST_FP_CTL        8'h49

// Source ID for FP_TSOR
`define REG_DST_FP_TSOR        8'h4A

// Source ID for DEVDEV_I2C
`define REG_DST_DEV_I2C        8'h4B

// Source ID for DEV_UART
`define REG_DST_DEV_UART        8'h4C

// Source ID for FP_GPIO
`define REG_DST_FP_GPIO        8'h4D

// Source ID for CBUF
`define REG_DST_CBUF        8'h4E

// Source ID for DDR_PLL
`define REG_DST_DDR_PLL        8'h60
*/

//In each die, FQMGR support up to 9 command queues
//The separation of the Queue to make sure command in different queue can be scheduled independently.
//FIF will do the schedule to pick a command to execute
enum die_cmd_queue {
	FQMGR_HOST_READ0,   // smallest possible latency
	FQMGR_HOST_READ1,	// mormal, typical latency
	FQMGR_HOST_READ2,	// long latency acceptable
	FQMGR_GC_READ,
	FQMGR_CTL_READ,
	FQMGR_HOST_WRITE,
	FQMGR_GC_WRITE,
	FQMGR_CTL_WRITE,
	FQMGR_ERASE,
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
	u64 port		 :1;	// port of PHIF
	u64 vf			 :4;

	// cmd specific
	u64 sqid		 :8;
	u64 hxts_mode	 :6;
	u64 rsvd3		 :2;
};


// hdc_nvmd_cmd message from PHIF, when a new host command arrival
typedef struct {
	struct msg_qw0 header;    // QW0
	struct nvme_command sqe; 	// QW1--QW8 is original SQE
} hdc_nvme_cmd;


typedef struct {
	struct msg_qw0 header;
	struct nvme_completion cqe;
} phif_cmd_cpl;


typedef struct {
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
	
	u64 rsvd2		 :4;
	u64 sta_sc	 	 :8;
	u64 sta_sct		 :4;
} phif_cmd_rsp;

typedef struct {
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

struct rwdma_ctrl {
	u64 blen			:15; //
	u64	bit_bucket		:1;  // is current req refer to bit bucket
	u64 cpl				:1;  // CP Last indication
	u64 pld_qwn			:4;	 // Payload QW number. QW number of Address
	u64 pi_type			:3;
	u64 prchk			:3;
	u64 rsvd0			:1;
	u64 dix				:1;	 // 1-DIX  0-DIF
	u64 pil				:1;  // PI location in metadata, (0:first 8B)  (1:last 8B)
	u64 pract			:1;
	u64 pien			:1;  // PI enable
	u64 cp_lb_num		:5;  // multiple LB in one CP   0-base
	u64 cp_lb_off		:5;	 // this LB offset in CP
	u64 cp_lb_full		:1;
	u64 rsvd1			:1;
	u64 lb_hmeta_sz     :4; 
	u64 lb_sz			:2;
	u64 lb_crc_en		:1;
	u64 rsvd2			:1;
	u32 hxts_mode		:6;
	u32 rsvd3			:2;
	u64 mode			:2;	  
	u64 rsvd  			:2;
};

typedef struct {
	struct msg_qw0 header;
	struct rwdma_ctrl control;
	u32 cpa;
	u32 rsvd_2;
	u64 hdata_addr;
	u64 hmeta_addr;
	u64 elba			:35;
	u64 rsvd_5			:29;
} phif_wdma_req_mandatory;

#define PHIF_WDMA_REQ_M_LEN   (sizeof(phif_wdma_req_mandatory))

typedef struct {
	// QW_PI
	u32 eilbrt;
	u16 elbat;
	u16 elbatm;

	// QW_ADDR
	u64 cbuff_addr		:36;
	u64 rsvd_addr		:28;

	// QW_DATA
	u64 wdata[2];		// mode2, data in message
} phif_wdma_req_optional;

enum {
	WDMA_QW_PI		= (1<<0),
	WDMA_QW_ADDR	= (1<<1),
	WDMA_QW_DATA0	= (1<<2),
	WDMA_QW_DATA1	= (1<<3),
};


typedef struct {
	phif_wdma_req_mandatory mandatory;
	phif_wdma_req_optional optional;
} phif_wdma_req;


typedef struct {
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
	
	u64 rsvd2		 :14;
	u64 staus		 :2;
} phif_wdma_rsp;

#endif
