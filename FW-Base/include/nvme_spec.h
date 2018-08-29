/* copy from Linux Kernel, don't change i */

#ifndef _NVME_SPEC_H_
#define _NVME_SPEC_H_

enum nvme_admin_opcode {
	nvme_admin_delete_sq		= 0x00,
	nvme_admin_create_sq		= 0x01,
	nvme_admin_get_log_page		= 0x02,
	nvme_admin_delete_cq		= 0x04,
	nvme_admin_create_cq		= 0x05,
	nvme_admin_identify			= 0x06,
	nvme_admin_abort_cmd		= 0x08,
	nvme_admin_set_features		= 0x09,
	nvme_admin_get_features		= 0x0a,
	nvme_admin_async_event		= 0x0c,
	nvme_admin_ns_mgmt			= 0x0d,
	nvme_admin_activate_fw		= 0x10,
	nvme_admin_download_fw		= 0x11,
	nvme_admin_ns_attach		= 0x15,
	nvme_admin_keep_alive		= 0x18,
	nvme_admin_directive_send	= 0x19,
	nvme_admin_directive_recv	= 0x1a,
	nvme_admin_dbbuf			= 0x7C,
	nvme_admin_format_nvm		= 0x80,
	nvme_admin_security_send	= 0x81,
	nvme_admin_security_recv	= 0x82,
	nvme_admin_sanitize_nvm		= 0x84,
};

enum nvme_io_opcode {
	nvme_io_flush			= 0x00,
	nvme_io_write			= 0x01,
	nvme_io_read			= 0x02,
	nvme_io_write_uncor		= 0x04,
	nvme_io_compare			= 0x05,
	nvme_io_write_zeroes	= 0x08,
	nvme_io_dsm				= 0x09,
	nvme_io_resv_register	= 0x0d,
	nvme_io_resv_report		= 0x0e,
	nvme_io_resv_acquire	= 0x11,
	nvme_io_resv_release	= 0x15,
};

// some opcode is rsvd in NVME spec, it is for vendor self-defined command
// openchannel SSD define opcode as below, detail please see lightnvm.io
enum lightnvm_opcode {
	nvme_nvm_admin_identity		= 0xe2,
	nvme_nvm_admin_get_bb_tbl	= 0xf2,
	nvme_nvm_admin_set_bb_tbl	= 0xf1,
	
	lightnvm_io_read			= 0x90,
	lightnvm_io_write			= 0x91,
	lightnvm_io_erase   		= 0x92,
};

enum {
	NVME_RW_LR					= 1 << 15,
	NVME_RW_FUA					= 1 << 14,
	NVME_RW_DSM_FREQ_UNSPEC		= 0,
	NVME_RW_DSM_FREQ_TYPICAL	= 1,
	NVME_RW_DSM_FREQ_RARE		= 2,
	NVME_RW_DSM_FREQ_READS		= 3,
	NVME_RW_DSM_FREQ_WRITES		= 4,
	NVME_RW_DSM_FREQ_RW			= 5,
	NVME_RW_DSM_FREQ_ONCE		= 6,
	NVME_RW_DSM_FREQ_PREFETCH	= 7,
	NVME_RW_DSM_FREQ_TEMP		= 8,
	NVME_RW_DSM_LATENCY_NONE	= 0 << 4,
	NVME_RW_DSM_LATENCY_IDLE	= 1 << 4,
	NVME_RW_DSM_LATENCY_NORM	= 2 << 4,
	NVME_RW_DSM_LATENCY_LOW		= 3 << 4,
	NVME_RW_DSM_SEQ_REQ			= 1 << 6,
	NVME_RW_DSM_COMPRESSED		= 1 << 7,
	NVME_RW_PRINFO_PRCHK_REF	= 1 << 10,
	NVME_RW_PRINFO_PRCHK_APP	= 1 << 11,
	NVME_RW_PRINFO_PRCHK_GUARD	= 1 << 12,
	NVME_RW_PRINFO_PRACT		= 1 << 13,
	NVME_RW_DTYPE_STREAMS		= 1 << 4,
};

typedef union
{
    u16  ctrl;
	
    struct
    {
        u16 rsvd1 	:4;
		u16 dtype 	:4;
        u16 rsvd2 	:2;
		u16 prinfo	:4;
		u16 fua		:1;		// fua enable, write through, read not search cache
        u16 lr		:1;     // limit retry, controller should apply limited retry efforts.
    } bits;
} ctrl_dw12h_t;


typedef union
{
	u32 dw13;

	struct
	{
		u32 dsm_frequency 		:4;
		u32 dsm_latency			:2;
		u32 dsm_seq_req 		:1;
		u32 dsm_incompressible 	:1;
		u32 rsvd				:8;
		u32 dspec				:16; // Directive Specific
	} bits;
} dsm_dw13_t;


struct nvme_sgl_desc {
	u64	addr;
	u32	length;
	u8	rsvd[3];
	u8	type;
};

struct nvme_keyed_sgl_desc {
	u64	addr;
	u8	length[3];
	u8	key[4];
	u8	type;
};

union nvme_data_ptr {
	struct {
		u64	prp1;
		u64	prp2;
	};
	struct nvme_sgl_desc	sgl;
	struct nvme_keyed_sgl_desc ksgl;
} ;

struct nvme_common_command {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u32			cdw2[2];
	u64			mptr;		   //meta pointer
	union nvme_data_ptr	dptr;  //data pointer
	u32			cdw10[6];
};

struct nvme_rw_command {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2;
	u64			mptr;
	union nvme_data_ptr	dptr;
	u64			slba;
	u16			length;
	u16			control;
	u32			dsmgmt;
	u32			reftag;
	u16			apptag;
	u16			appmask;
};

struct nvme_identify {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[2];
	union nvme_data_ptr	dptr;
	u8			cns;
	u8			rsvd3;
	u16			ctrlid;
	u32			rsvd11[5];
};


struct nvme_features {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[2];
	union nvme_data_ptr	dptr;
	u32			fid;
	u32			dword11;
	u32         dword12;
	u32         dword13;
	u32         dword14;
	u32         dword15;
};

struct nvme_command {
	union {
		struct nvme_common_command common;
		struct nvme_rw_command rw;
		struct nvme_identify identify;
		struct nvme_features features;
		//struct nvme_create_cq create_cq;
		//struct nvme_create_sq create_sq;
		//struct nvme_delete_queue delete_queue;
		//struct nvme_download_firmware dlfw;
		//struct nvme_format_cmd format;
		//struct nvme_dsm_cmd dsm;
		//struct nvme_write_zeroes_cmd write_zeroes;
		//struct nvme_abort_cmd abort;
		//struct nvme_get_log_page_command get_log_page;
		//struct nvmf_common_command fabrics;
		//struct nvmf_connect_command connect;
		//struct nvmf_property_set_command prop_set;
		//struct nvmf_property_get_command prop_get;
		//struct nvme_dbbuf dbbuf;
		//struct nvme_directive_cmd directive;
	};
};


struct nvme_completion {
	/*
	 * Used by Admin and Fabrics commands to return data:
	 */
	union nvme_result {
		u16 bit16;
		u32	bit32;
		u64 bit64;
	} result;

	u16	sq_head;	/* how much of this queue may be reclaimed */
	u16	sq_id;		/* submission queue that generated this entry */
	u16	command_id;	/* of the command which completed */
	u16	status;		/* did the command fail, and if so, why? */
};


#endif
