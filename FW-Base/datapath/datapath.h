#ifndef _DATAPATH_H_
#define _DATAPATH_H_

// host cmd tag is 8 bit, so the max host command
// for Write Tag [0 63]   for Read Tag[64 255]
//#define HOST_WR_CMD_MAX
//#define HOST_RD_CMD_MAX

#define HOST_NVME_CMD_ENTRY_CNT  256


typedef struct {
	s_node_t *next;
	u8 cmd_tag;
	u8 state;
	u16 sqid;
	struct nvme_command sqe;
} host_nvme_cmd_entry;

#endif
