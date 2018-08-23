/*Apollo Controller intra, each blocks and CPU communication via a message*/

#ifndef _MESSAGE_FORMAT_H_
#define _MESSAGE_FORMAT_H_

// qw0 is various depend on message type
struct msg_qw0_1 {
	u64 cnt			 :4;
	u64 dstfifo		 :4;
	u64 dst			 :5;
	u64 prio		 :2;
	u64 last		 :1;
	u64 msgid		 :8;
	u64 tag			 :8;
	u64 src			 :5;
	u64 rsvd0		 :3;
	u64 vf			 :4;
	u64 rsvd1		 :2;
	u64 vfa			 :1;
	u64 port		 :1;
	u64 sqid		 :8;
	u64 rsvd2		 :8;
};

struct msg_qw0_2 {
	u64 cnt			 :4;
	u64 dstfifo		 :4;
	u64 dst			 :5;
	u64 prio		 :2;
	u64 last		 :1;
	u64 msgid		 :8;
	u64 tag			 :8;   //cmdid
	u64 src			 :5;
	u64 rsvd0		 :3;
	u64 vf			 :4;
	u64 vfa			 :1;
	u64 rsvd1		 :2;
	u64 port		 :1;
	u64 sqid		 :8;
	u64 hxts_mode	 :6;
	u64 range_bypass :1;
	u64 rsvd2		 :1;
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
	struct msg_qw0_1 header;    // QW0
	struct nvme_command sqe; 	// QW1--QW8 is original SQE
} hdc_nvme_cmd;


typedef struct phif_cmd_complete {
	struct msg_qw0_1 header;
	struct nvme_completion cqe;
} phif_cmd_cpl;


typedef struct phif_cmd_request {
	struct msg_qw0_2 header;
	u32 cpa;
	u32 hmeta_size		:4;  // 8B * (this value)
	u32 cph_size		:4;  // 8B * (this value)
	u32 lb_size		    :2;  // 00:512B  01:4KB  10:16KB
	u32 crc_en			:1;
	u32 rsvd0			:1;
	u32 dps				:4;  // PI enable/disable type1/2/3
	u32 flbas			:5;  // Format LBA dize
	u32 rsvd1			:3;
	u32 ovlap			:2;  // ovlap check type
	u32 rsvd2			:1;
	u32 cache_en		:1;
	u32 stream			:4;
} phif_cmd_req;

#endif
