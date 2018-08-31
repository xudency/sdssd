#ifndef _DATAPATH_H_
#define _DATAPATH_H_

// host cmd tag is 8 bit, so the max host command
// for Write Tag [0 63]   for Read Tag[64 255]
//#define HOST_WR_CMD_MAX
//#define HOST_RD_CMD_MAX

#define HOST_NVME_CMD_ENTRY_CNT  256

//#define 

typedef enum {
	WRITE_FLOW_STATE_INITIAL,    		// init val
	WRITE_FLOW_STATE_ENQUEUE,			// enqueue, has no process
	WRITE_FLOW_STATE_PHIF_REQ_READY,
	WRITE_FLOW_STATE_PHIF_REQ_SENDOUT,
	WRITE_FLOW_STATE_WAIT_PHIF_RSP,
	WRITE_FLOW_STATE_HAS_PHIF_RSP,
	WRITE_FLOW_STATE_PHIF_CPL_SENDOUT,
	WRITE_FLOW_STATE_COMPLETE,
} WRITE_FLOW_STATE;



typedef struct {
	struct qnode next;
	u8 cmd_tag;				// 0-255
	u8 state;
	u8 sqid;
	u8 vfa		:1;
	u8 port		:1;
	u8 vf		:4;
	u8 rsvd		:2;
	struct nvme_command sqe;
	//struct nvme_completion cqe;
	u32 sta_sc		:8;
	u32 sta_sct		:4;
	u32 rsvd2		:20;
} host_nvme_cmd_entry;




#endif
