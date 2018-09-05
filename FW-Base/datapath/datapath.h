#ifndef _DATAPATH_H_
#define _DATAPATH_H_

// host cmd tag is 8 bit, so the max host command
// for Write Tag [0 63]   for Read Tag[64 255]
//#define HOST_WR_CMD_MAX
//#define HOST_RD_CMD_MAX

#define HOST_NVME_CMD_ENTRY_CNT    256
#define FW_INTERNAL_CMD_ENTRY_CNT  256

#define FW_WDMA_REQ_CNT		64
//#define 

typedef enum {
	WRITE_FLOW_STATE_INITIAL = 0x00,    		// init val
	WRITE_FLOW_STATE_QUEUED,			// enqueue, has no process
	WRITE_FLOW_STATE_PHIF_REQ_READY,
	WRITE_FLOW_STATE_PHIF_REQ_SENDOUT,
	WRITE_FLOW_STATE_WAIT_PHIF_RSP,
	WRITE_FLOW_STATE_HAS_PHIF_RSP,
	WRITE_FLOW_STATE_PHIF_CPL_READY,
	WRITE_FLOW_STATE_PHIF_CPL_SENDOUT,
	WRITE_FLOW_STATE_COMPLETE,
} WRITE_FLOW_STATE;


typedef enum {
	READ_FLOW_STATE_INITIAL = 0x00,    		// init val
	READ_FLOW_STATE_QUEUED,			// enqueue, has no process
	READ_FLOW_STATE_PHIF_REQ_READY,
	READ_FLOW_STATE_PHIF_REQ_SENDOUT,
	READ_FLOW_STATE_WAIT_PHIF_RSP,
	READ_FLOW_STATE_HAS_PHIF_RSP,
	READ_FLOW_STATE_PHIF_CPL_READY,
	READ_FLOW_STATE_PHIF_CPL_SENDOUT,
	READ_FLOW_STATE_COMPLETE,
} READ_FLOW_STATE;


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
	u32 ckc			:8;   // chunk count
	u32 rsvd2		:12;

	//host_cmd_callback  // when response, call this function
} host_nvme_cmd_entry;

typedef int (*fw_cmd_callback)(void *);

// WDMA RDMA ... etc.
typedef struct {
	u8 host_tag;     // this fw cmd is split from which host nvme cmd
	u8 rsvd;	
	u16 itn_tag;     // a host cmd will split to multi fw internal command
	fw_cmd_callback fn;  // DEC(host_cmd_entry[host_tag].ckc), if == 0 send phif_cmd_cpl
} fw_internal_cmd_entry;


host_nvme_cmd_entry *__get_fw_cmd_entry(u8 itnl_tag)
{
	return &gat_fw_internal_cmd_array[itnl_tag];
}

host_nvme_cmd_entry *__get_host_cmd_entry(u8 tag)
{
	return &gat_host_nvme_cmd_array[tag];
}

phif_cmd_cpl *__get_host_cmd_cpl_entry(u8 tag)
{
	return &gat_host_cmd_cpl_array[tag];
}


phif_cmd_cpl *__get_fw_wdma_req_entry(u8 tag)
{
	return &gat_fw_wdma_req_array[tag];
}

#endif
