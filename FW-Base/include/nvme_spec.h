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


/* NQN names in commands fields specified one size */
#define NVMF_NQN_FIELD_LEN	256

/* However the max length of a qualified name is another size */
#define NVMF_NQN_SIZE		223

#define NVMF_TRSVCID_SIZE	32
#define NVMF_TRADDR_SIZE	256
#define NVMF_TSAS_SIZE		256

#define NVME_DISC_SUBSYS_NAME	"nqn.2014-08.org.nvmexpress.discovery"

#define NVME_RDMA_IP_PORT	4420

#define NVME_NSID_ALL		0xffffffff

enum nvme_subsys_type {
	NVME_NQN_DISC	= 1,		/* Discovery type target subsystem */
	NVME_NQN_NVME	= 2,		/* NVME type target subsystem */
};

/* Address Family codes for Discovery Log Page entry ADRFAM field */
enum {
	NVMF_ADDR_FAMILY_PCI	= 0,	/* PCIe */
	NVMF_ADDR_FAMILY_IP4	= 1,	/* IP4 */
	NVMF_ADDR_FAMILY_IP6	= 2,	/* IP6 */
	NVMF_ADDR_FAMILY_IB	= 3,	/* InfiniBand */
	NVMF_ADDR_FAMILY_FC	= 4,	/* Fibre Channel */
};

/* Transport Type codes for Discovery Log Page entry TRTYPE field */
enum {
	NVMF_TRTYPE_RDMA	= 1,	/* RDMA */
	NVMF_TRTYPE_FC		= 2,	/* Fibre Channel */
	NVMF_TRTYPE_LOOP	= 254,	/* Reserved for host usage */
	NVMF_TRTYPE_MAX,
};

/* Transport Requirements codes for Discovery Log Page entry TREQ field */
enum {
	NVMF_TREQ_NOT_SPECIFIED	= 0,	/* Not specified */
	NVMF_TREQ_REQUIRED	= 1,	/* Required */
	NVMF_TREQ_NOT_REQUIRED	= 2,	/* Not Required */
};

/* RDMA QP Service Type codes for Discovery Log Page entry TSAS
 * RDMA_QPTYPE field
 */
enum {
	NVMF_RDMA_QPTYPE_CONNECTED	= 1, /* Reliable Connected */
	NVMF_RDMA_QPTYPE_DATAGRAM	= 2, /* Reliable Datagram */
};

/* RDMA QP Service Type codes for Discovery Log Page entry TSAS
 * RDMA_QPTYPE field
 */
enum {
	NVMF_RDMA_PRTYPE_NOT_SPECIFIED	= 1, /* No Provider Specified */
	NVMF_RDMA_PRTYPE_IB		= 2, /* InfiniBand */
	NVMF_RDMA_PRTYPE_ROCE		= 3, /* InfiniBand RoCE */
	NVMF_RDMA_PRTYPE_ROCEV2		= 4, /* InfiniBand RoCEV2 */
	NVMF_RDMA_PRTYPE_IWARP		= 5, /* IWARP */
};

/* RDMA Connection Management Service Type codes for Discovery Log Page
 * entry TSAS RDMA_CMS field
 */
enum {
	NVMF_RDMA_CMS_RDMA_CM	= 1, /* Sockets based endpoint addressing */
};

#define NVME_AQ_DEPTH		32
#define NVME_NR_AEN_COMMANDS	1
#define NVME_AQ_BLK_MQ_DEPTH	(NVME_AQ_DEPTH - NVME_NR_AEN_COMMANDS)

/*
 * Subtract one to leave an empty queue entry for 'Full Queue' condition. See
 * NVM-Express 1.2 specification, section 4.1.2.
 */
#define NVME_AQ_MQ_TAG_DEPTH	(NVME_AQ_BLK_MQ_DEPTH - 1)

enum {
	NVME_REG_CAP	= 0x0000,	/* Controller Capabilities */
	NVME_REG_VS		= 0x0008,	/* Version */
	NVME_REG_INTMS	= 0x000c,	/* Interrupt Mask Set */
	NVME_REG_INTMC	= 0x0010,	/* Interrupt Mask Clear */
	NVME_REG_CC		= 0x0014,	/* Controller Configuration */
	NVME_REG_CSTS	= 0x001c,	/* Controller Status */
	NVME_REG_NSSR	= 0x0020,	/* NVM Subsystem Reset */
	NVME_REG_AQA	= 0x0024,	/* Admin Queue Attributes */
	NVME_REG_ASQ	= 0x0028,	/* Admin SQ Base Address */
	NVME_REG_ACQ	= 0x0030,	/* Admin CQ Base Address */
	NVME_REG_CMBLOC = 0x0038,	/* Controller Memory Buffer Location */
	NVME_REG_CMBSZ	= 0x003c,	/* Controller Memory Buffer Size */
	NVME_REG_DBS	= 0x1000,	/* SQ 0 Tail Doorbell */
};

#define NVME_CAP_MQES(cap)		((cap) & 0xffff)
#define NVME_CAP_TIMEOUT(cap)	(((cap) >> 24) & 0xff)
#define NVME_CAP_STRIDE(cap)	(((cap) >> 32) & 0xf)
#define NVME_CAP_NSSRC(cap)		(((cap) >> 36) & 0x1)
#define NVME_CAP_MPSMIN(cap)	(((cap) >> 48) & 0xf)
#define NVME_CAP_MPSMAX(cap)	(((cap) >> 52) & 0xf)

#define NVME_CMB_BIR(cmbloc)	((cmbloc) & 0x7)
#define NVME_CMB_OFST(cmbloc)	(((cmbloc) >> 12) & 0xfffff)

enum {
	NVME_CMBSZ_SQS		= 1 << 0,
	NVME_CMBSZ_CQS		= 1 << 1,
	NVME_CMBSZ_LISTS	= 1 << 2,
	NVME_CMBSZ_RDS		= 1 << 3,
	NVME_CMBSZ_WDS		= 1 << 4,

	NVME_CMBSZ_SZ_SHIFT	= 12,
	NVME_CMBSZ_SZ_MASK	= 0xfffff,

	NVME_CMBSZ_SZU_SHIFT	= 8,
	NVME_CMBSZ_SZU_MASK	= 0xf,
};

/*
 * Submission and Completion Queue Entry Sizes for the NVM command set.
 * (In bytes and specified as a power of two (2^n)).
 */
#define NVME_NVM_IOSQES		6
#define NVME_NVM_IOCQES		4

enum {
	NVME_CC_ENABLE		= 1 << 0,
	NVME_CC_CSS_NVM		= 0 << 4,
	NVME_CC_EN_SHIFT	= 0,
	NVME_CC_CSS_SHIFT	= 4,
	NVME_CC_MPS_SHIFT	= 7,
	NVME_CC_AMS_SHIFT	= 11,
	NVME_CC_SHN_SHIFT	= 14,
	NVME_CC_IOSQES_SHIFT	= 16,
	NVME_CC_IOCQES_SHIFT	= 20,
	NVME_CC_AMS_RR		= 0 << NVME_CC_AMS_SHIFT,
	NVME_CC_AMS_WRRU	= 1 << NVME_CC_AMS_SHIFT,
	NVME_CC_AMS_VS		= 7 << NVME_CC_AMS_SHIFT,
	NVME_CC_SHN_NONE	= 0 << NVME_CC_SHN_SHIFT,
	NVME_CC_SHN_NORMAL	= 1 << NVME_CC_SHN_SHIFT,
	NVME_CC_SHN_ABRUPT	= 2 << NVME_CC_SHN_SHIFT,
	NVME_CC_SHN_MASK	= 3 << NVME_CC_SHN_SHIFT,
	NVME_CC_IOSQES		= NVME_NVM_IOSQES << NVME_CC_IOSQES_SHIFT,
	NVME_CC_IOCQES		= NVME_NVM_IOCQES << NVME_CC_IOCQES_SHIFT,
	NVME_CSTS_RDY		= 1 << 0,
	NVME_CSTS_CFS		= 1 << 1,
	NVME_CSTS_NSSRO		= 1 << 4,
	NVME_CSTS_PP		= 1 << 5,
	NVME_CSTS_SHST_NORMAL	= 0 << 2,
	NVME_CSTS_SHST_OCCUR	= 1 << 2,
	NVME_CSTS_SHST_CMPLT	= 2 << 2,
	NVME_CSTS_SHST_MASK	= 3 << 2,
};

struct nvme_id_power_state {
	u16			max_power;	/* centiwatts */
	u8			rsvd2;
	u8			flags;
	u32			entry_lat;	/* microseconds */
	u32			exit_lat;	/* microseconds */
	u8			read_tput;
	u8			read_lat;
	u8			write_tput;
	u8			write_lat;
	u16			idle_power;
	u8			idle_scale;
	u8			rsvd19;
	u16			active_power;
	u8			active_work_scale;
	u8			rsvd23[9];
};

enum {
	NVME_PS_FLAGS_MAX_POWER_SCALE	= 1 << 0,
	NVME_PS_FLAGS_NON_OP_STATE	= 1 << 1,
};


struct nvme_id_ctrl {
	u16			vid;
	u16			ssvid;
	char		sn[20];
	char		mn[40];
	char		fr[8];
	u8			rab;
	u8			ieee[3];
	u8			cmic;
	u8			mdts;
	u16			cntlid;
	u32			ver;
	u32			rtd3r;
	u32			rtd3e;
	u32			oaes;
	u32			ctratt;
	u8			rsvd100[156];
	u16			oacs;
	u8			acl;
	u8			aerl;
	u8			frmw;
	u8			lpa;
	u8			elpe;
	u8			npss;
	u8			avscc;
	u8			apsta;
	u16			wctemp;
	u16			cctemp;
	u16			mtfa;
	u32			hmpre;
	u32			hmmin;
	u8			tnvmcap[16];
	u8			unvmcap[16];
	u32			rpmbs;
	u16			edstt;
	u8			dsto;
	u8			fwug;
	u16			kas;
	u16			hctma;
	u16			mntmt;
	u16			mxtmt;
	u32			sanicap;
	u32			hmminds;
	u16			hmmaxd;
	u8			rsvd338[174];
	u8			sqes;
	u8			cqes;
	u16			maxcmd;
	u32			nn;
	u16			oncs;
	u16			fuses;
	u8			fna;
	u8			vwc;
	u16			awun;
	u16			awupf;
	u8			nvscc;
	u8			rsvd531;
	u16			acwu;
	u8			rsvd534[2];
	u32			sgls;
	u8			rsvd540[228];
	char		subnqn[256];
	u8			rsvd1024[768];
	u32			ioccsz;
	u32			iorcsz;
	u16			icdoff;
	u8			ctrattr;
	u8			msdbd;
	u8			rsvd1804[244];
	struct nvme_id_power_state	psd[32];
	u8			vs[1024];
};

enum {
	NVME_CTRL_ONCS_COMPARE			= 1 << 0,
	NVME_CTRL_ONCS_WRITE_UNCORRECTABLE	= 1 << 1,
	NVME_CTRL_ONCS_DSM				= 1 << 2,
	NVME_CTRL_ONCS_WRITE_ZEROES		= 1 << 3,
	NVME_CTRL_ONCS_TIMESTAMP		= 1 << 6,
	NVME_CTRL_VWC_PRESENT			= 1 << 0,
	NVME_CTRL_OACS_SEC_SUPP         = 1 << 0,
	NVME_CTRL_OACS_DIRECTIVES		= 1 << 5,
	NVME_CTRL_OACS_DBBUF_SUPP		= 1 << 8,
	NVME_CTRL_LPA_CMD_EFFECTS_LOG	= 1 << 1,
};

// rp only need 2 bit, pther 6bit is rsvd in standard NVMe spec
// here we use 2bit for CPH_SZ
struct nvme_lbaf {
	u16			ms;     	// metadata size in byte
	u8			ds;     	// LBA size 2^n
	u8			rp   :2;    // relative performance
	u8			cphs :2;    //CPH size  1:16B / 2:32B / 3:48B
	u8			rsvd :4;
};

// 4KB
struct nvme_id_ns {
	u64			nsze;
	u64			ncap;
	u64			nuse;
	u8			nsfeat;
	u8			nlbaf;
	u8			flbas;
	u8			mc;
	u8			dpc;
	u8			dps;
	u8			nmic;
	u8			rescap;
	u8			fpi;
	u8			rsvd33;
	u16			nawun;
	u16			nawupf;
	u16			nacwu;
	u16			nabsn;
	u16			nabo;
	u16			nabspf;
	u16			noiob;
	u8			nvmcap[16];
	u8			rsvd64[40];
	u8			nguid[16];
	u8			eui64[8];
	struct nvme_lbaf	lbaf[16];
	u8			rsvd192[192];
	u8			vs[3712];
};

enum {
	NVME_ID_CNS_NS			= 0x00,
	NVME_ID_CNS_CTRL		= 0x01,
	NVME_ID_CNS_NS_ACTIVE_LIST	= 0x02,
	NVME_ID_CNS_NS_DESC_LIST	= 0x03,
	NVME_ID_CNS_NS_PRESENT_LIST	= 0x10,
	NVME_ID_CNS_NS_PRESENT		= 0x11,
	NVME_ID_CNS_CTRL_NS_LIST	= 0x12,
	NVME_ID_CNS_CTRL_LIST		= 0x13,
};

enum {
	NVME_DIR_IDENTIFY		= 0x00,
	NVME_DIR_STREAMS		= 0x01,
	NVME_DIR_SND_ID_OP_ENABLE	= 0x01,
	NVME_DIR_SND_ST_OP_REL_ID	= 0x01,
	NVME_DIR_SND_ST_OP_REL_RSC	= 0x02,
	NVME_DIR_RCV_ID_OP_PARAM	= 0x01,
	NVME_DIR_RCV_ST_OP_PARAM	= 0x01,
	NVME_DIR_RCV_ST_OP_STATUS	= 0x02,
	NVME_DIR_RCV_ST_OP_RESOURCE	= 0x03,
	NVME_DIR_ENDIR			= 0x01,
};

enum {
	NVME_NS_FEAT_THIN	= 1 << 0,
	NVME_NS_FLBAS_LBA_MASK	= 0xf,
	NVME_NS_FLBAS_META_EXT	= 0x10,
	NVME_LBAF_RP_BEST	= 0,
	NVME_LBAF_RP_BETTER	= 1,
	NVME_LBAF_RP_GOOD	= 2,
	NVME_LBAF_RP_DEGRADED	= 3,
	NVME_NS_DPC_PI_LAST	= 1 << 4,
	NVME_NS_DPC_PI_FIRST	= 1 << 3,
	NVME_NS_DPC_PI_TYPE3	= 1 << 2,
	NVME_NS_DPC_PI_TYPE2	= 1 << 1,
	NVME_NS_DPC_PI_TYPE1	= 1 << 0,
	NVME_NS_DPS_PI_FIRST	= 1 << 3,
	NVME_NS_DPS_PI_MASK	= 0x7,
	NVME_NS_DPS_PI_TYPE1	= 1,
	NVME_NS_DPS_PI_TYPE2	= 2,
	NVME_NS_DPS_PI_TYPE3	= 3,
};

struct nvme_ns_id_desc {
	u8 nidt;
	u8 nidl;
	u16 reserved;
};

#define NVME_NIDT_EUI64_LEN	8
#define NVME_NIDT_NGUID_LEN	16
#define NVME_NIDT_UUID_LEN	16

enum {
	NVME_NIDT_EUI64		= 0x01,
	NVME_NIDT_NGUID		= 0x02,
	NVME_NIDT_UUID		= 0x03,
};

struct nvme_smart_log {
	u8			critical_warning;
	u8			temperature[2];
	u8			avail_spare;
	u8			spare_thresh;
	u8			percent_used;
	u8			rsvd6[26];
	u8			data_units_read[16];
	u8			data_units_written[16];
	u8			host_reads[16];
	u8			host_writes[16];
	u8			ctrl_busy_time[16];
	u8			power_cycles[16];
	u8			power_on_hours[16];
	u8			unsafe_shutdowns[16];
	u8			media_errors[16];
	u8			num_err_log_entries[16];
	u32			warning_temp_time;
	u32			critical_comp_time;
	u16			temp_sensor[8];
	u8			rsvd216[296];
};

struct nvme_fw_slot_info_log {
	u8			afi;
	u8			rsvd1[7];
	u64			frs[7];
	u8			rsvd64[448];
};

enum {
	NVME_CMD_EFFECTS_CSUPP		= 1 << 0,
	NVME_CMD_EFFECTS_LBCC		= 1 << 1,
	NVME_CMD_EFFECTS_NCC		= 1 << 2,
	NVME_CMD_EFFECTS_NIC		= 1 << 3,
	NVME_CMD_EFFECTS_CCC		= 1 << 4,
	NVME_CMD_EFFECTS_CSE_MASK	= 3 << 16,
};

struct nvme_effects_log {
	u32 acs[256];
	u32 iocs[256];
	u8   resv[2048];
};

enum {
	NVME_SMART_CRIT_SPARE		= 1 << 0,
	NVME_SMART_CRIT_TEMPERATURE	= 1 << 1,
	NVME_SMART_CRIT_RELIABILITY	= 1 << 2,
	NVME_SMART_CRIT_MEDIA		= 1 << 3,
	NVME_SMART_CRIT_VOLATILE_MEMORY	= 1 << 4,
};

enum {
	NVME_AER_ERROR			= 0,
	NVME_AER_SMART			= 1,
	NVME_AER_CSS			= 6,
	NVME_AER_VS			= 7,
	NVME_AER_NOTICE_NS_CHANGED	= 0x0002,
	NVME_AER_NOTICE_FW_ACT_STARTING = 0x0102,
};

// fid=1
struct nvme_arbitration_feat {
    u32		ab		:3;
    u32		rsvd	:5;
    u32		lpw		:8;
    u32		mpw		:8;
    u32		hpw		:8;
};

// fid=2
struct nvme_power_mgmt_feat {
	u32 	ps		:5;
	u32 	wh		:3;
	u32 	rsvd	:24;
};

// fid=3, pointer by PRP1
struct nvme_lba_range_type {
	u8			type;
	u8			attributes;
	u8			rsvd2[14];
	u64			slba;
	u64			nlb;
	u8			guid[16];
	u8			rsvd48[16];
};

#define NVME_LBART_ENTRY_SIZE    64
#define NVME_LBART_MAX_ENTRYS    64    // 4K/64B
#define NVME_LBART_TBL_SIZE     (NVME_LBART_ENTRY_SIZE*NVME_LBART_MAX_ENTRYS)
#define NVME_LBART_NUM_MASK 	0x3f   // cwd11

enum {
	NVME_LBART_TYPE_FS		= 0x01,
	NVME_LBART_TYPE_RAID	= 0x02,
	NVME_LBART_TYPE_CACHE	= 0x03,
	NVME_LBART_TYPE_SWAP	= 0x04,

	NVME_LBART_ATTRIB_TEMP	= 1 << 0,
	NVME_LBART_ATTRIB_HIDE	= 1 << 1,
};

// fid=4
struct nvme_tmperature_th_feat {
	u16 tmpth;
	u16 tmpsel		:4;
	u16 thsel		:2;  // 00-over  01-under
	u16 rsvd		:10;
};

enum {
	NVME_TMPRATURE_COMPOSITE = 0x00,
	NVME_TMPRATURE_SENSOR1 = 0x01,
	NVME_TMPRATURE_SENSOR2 = 0x02,
	NVME_TMPRATURE_SENSOR3 = 0x03,
	NVME_TMPRATURE_SENSOR4 = 0x04,
	NVME_TMPRATURE_SENSOR5 = 0x05,
	NVME_TMPRATURE_SENSOR6 = 0x06,
	NVME_TMPRATURE_SENSOR7 = 0x07,
	NVME_TMPRATURE_SENSOR8 = 0x08,
};

enum {
	NVME_TMPRATURE_OVER_TH = 0x0,
	NVME_TMPRATURE_UBDER_TH = 0x1,
};

// fid=5
struct nvme_error_recov_feat {
    u16		tler;
    u16		dulbe		:1;
    u16		rsvd		:15;
};

// fid=6
struct nvme_volatile_cache_feat {
    u32		wce			:1;
    u32		rsvd		:31;
};


// fid=7
struct nvme_queue_number {
    u16		nsqr;
    u16		ncqr;
};


struct nvme_reservation_status {
	u32	gen;
	u8	rtype;
	u8	regctl[2];
	u8	resv5[2];
	u8	ptpls;
	u8	resv10[13];
	struct {
		u16	cntlid;
		u8	rcsts;
		u8	resv3[5];
		u64	hostid;
		u64	rkey;
	} regctl_ds[];
};

enum nvme_async_event_type {
	NVME_AER_TYPE_ERROR	= 0,
	NVME_AER_TYPE_SMART	= 1,
	NVME_AER_TYPE_NOTICE	= 2,
};


/*
 * Descriptor subtype - lower 4 bits of nvme_(keyed_)sgl_desc identifier
 * only used in NVMe Over Fabric
 *
 * @NVME_SGL_FMT_ADDRESS:     absolute address of the data block
 * @NVME_SGL_FMT_OFFSET:      relative offset of the in-capsule data block
 * @NVME_SGL_FMT_TRANSPORT_A: transport defined format, value 0xA
 * @NVME_SGL_FMT_INVALIDATE:  RDMA transport specific remote invalidation
 *                            request subtype
 */
enum {
	NVME_SGL_FMT_ADDRESS		= 0x00,
	NVME_SGL_FMT_OFFSET		= 0x01,
	NVME_SGL_FMT_TRANSPORT_A	= 0x0A,
	NVME_SGL_FMT_INVALIDATE		= 0x0f,
};

/*
 * Descriptor type - upper 4 bits of nvme_(keyed_)sgl_desc identifier
 *
 * For struct nvme_sgl_desc:
 *   @NVME_SGL_FMT_DATA_DESC:		data block descriptor
 *   @NVME_SGL_FMT_SEG_DESC:		sgl segment descriptor
 *   @NVME_SGL_FMT_LAST_SEG_DESC:	last sgl segment descriptor
 *
 * For struct nvme_keyed_sgl_desc:
 *   @NVME_KEY_SGL_FMT_DATA_DESC:	keyed data block descriptor
 *
 * Transport-specific SGL types:
 *   @NVME_TRANSPORT_SGL_DATA_DESC:	Transport SGL data dlock descriptor
 */
enum {
	NVME_SGL_FMT_DATA_DESC		= 0x00,
	NVME_SGL_FMT_BIT_BUCKET		= 0x01,
	NVME_SGL_FMT_SEG_DESC		= 0x02,
	NVME_SGL_FMT_LAST_SEG_DESC	= 0x03,
	NVME_KEY_SGL_FMT_DATA_DESC	= 0x04,
	NVME_TRANSPORT_SGL_DATA_DESC	= 0x05,
};


struct nvme_sgl_desc {
	u64	addr;		// when type=bit_bucket, this is rsvd
	u32	length;
	u8	rsvd[3];
	u8	type;        // type(4bit) | subtype(4bit)
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

/*
 * Lowest two bits of our flags field (FUSE field in the spec):
 *
 * @NVME_CMD_FUSE_FIRST:   Fused Operation, first command
 * @NVME_CMD_FUSE_SECOND:  Fused Operation, second command
 *
 * Highest two bits in our flags field (PSDT field in the spec):
 *
 * @NVME_CMD_PSDT_SGL_METABUF:	Use SGLS for this transfer,
 *	If used, MPTR contains addr of single physical buffer (byte aligned).
 * @NVME_CMD_PSDT_SGL_METASEG:	Use SGLS for this transfer,
 *	If used, MPTR contains an address of an SGL segment containing
 *	exactly 1 SGL descriptor (qword aligned).
 */
enum {
	NVME_CMD_FUSE_FIRST	= (1 << 0),
	NVME_CMD_FUSE_SECOND	= (1 << 1),

	NVME_CMD_SGL_METABUF	= (1 << 6),
	NVME_CMD_SGL_METASEG	= (1 << 7),
	NVME_CMD_SGL_ALL	= NVME_CMD_SGL_METABUF | NVME_CMD_SGL_METASEG,
};


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

struct nvme_dsm_cmd {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[2];
	union nvme_data_ptr	dptr;
	u32			nr;
	u32			attributes;
	u32			rsvd12[4];
};

enum {
	NVME_DSMGMT_IDR		= 1 << 0,
	NVME_DSMGMT_IDW		= 1 << 1,
	NVME_DSMGMT_AD		= 1 << 2,
};

#define NVME_DSM_MAX_RANGES	256

struct nvme_dsm_range {
	u32			cattr;
	u32			nlb;
	u64			slba;
};

struct nvme_write_zeroes_cmd {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2;
	u64			metadata;
	union nvme_data_ptr	dptr;
	u64			slba;
	u16			length;
	u16			control;
	u32			dsmgmt;
	u32			reftag;
	u16			apptag;
	u16			appmask;
};

/* Features */

struct nvme_feat_auto_pst {
	u64 entries[32];
};

enum {
	NVME_HOST_MEM_ENABLE	= (1 << 0),
	NVME_HOST_MEM_RETURN	= (1 << 1),
};

enum {
	NVME_QUEUE_PHYS_CONTIG	= (1 << 0),
	NVME_CQ_IRQ_ENABLED		= (1 << 1),
	NVME_SQ_PRIO_URGENT		= (0 << 1),
	NVME_SQ_PRIO_HIGH		= (1 << 1),
	NVME_SQ_PRIO_MEDIUM		= (2 << 1),
	NVME_SQ_PRIO_LOW		= (3 << 1),
	NVME_FEAT_UNDEFID		= 0x00,
	NVME_FEAT_ARBITRATION	= 0x01,
	NVME_FEAT_POWER_MGMT	= 0x02,
	NVME_FEAT_LBA_RANGE		= 0x03,
	NVME_FEAT_TEMP_THRESH	= 0x04,
	NVME_FEAT_ERR_RECOVERY	= 0x05,
	NVME_FEAT_VOLATILE_WC	= 0x06,
	NVME_FEAT_NUM_QUEUES	= 0x07,
	NVME_FEAT_IRQ_COALESCE	= 0x08,
	NVME_FEAT_IRQ_CONFIG	= 0x09,
	NVME_FEAT_WRITE_ATOMIC	= 0x0a,
	NVME_FEAT_ASYNC_EVENT	= 0x0b,
	NVME_FEAT_AUTO_PST		= 0x0c,
	NVME_FEAT_HOST_MEM_BUF	= 0x0d,
	NVME_FEAT_TIMESTAMP		= 0x0e,
	NVME_FEAT_KATO			= 0x0f,
	NVME_FEAT_SW_PROGRESS	= 0x80,
	NVME_FEAT_HOST_ID		= 0x81,
	NVME_FEAT_RESV_MASK		= 0x82,
	NVME_FEAT_RESV_PERSIST	= 0x83,
	NVME_LOG_ERROR			= 0x01,
	NVME_LOG_SMART			= 0x02,
	NVME_LOG_FW_SLOT		= 0x03,
	NVME_LOG_CMD_EFFECTS	= 0x05,
	NVME_LOG_DISC			= 0x70,
	NVME_LOG_RESERVATION	= 0x80,
	NVME_FWACT_REPL			= (0 << 3),
	NVME_FWACT_REPL_ACTV	= (1 << 3),
	NVME_FWACT_ACTV			= (2 << 3),
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

#define NVME_IDENTIFY_DATA_SIZE 4096

typedef union 
{
	dword10;
	
	struct {
		u32 fid		:8;
		u32 rsvd	:23;
		u32 save	:1;
	} sf_dw10;

	struct {
		u32 fid		:8;
		u32 sel		:3;
		u32 rsvd	:21;
	} gf_dw10;
} feat_dw10;

enum {
	NVME_FEAT_SEL_CURRENT 	  = 0x0,
	NVME_FEAT_SEL_DEFAULT 	  = 0x1,
	NVME_FEAT_SEL_SAVED		  = 0x2,
	NVME_FEAT_SEL_SUPPORT_CAP = 0x3,
};

struct nvme_features {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[2];
	union nvme_data_ptr	dptr;
	feat_dw10   fid; 
	u32			dword11;
	u32         dword12;
	u32         dword13;
	u32         dword14;
	u32         dword15;
};

#define NVME_FEAT_ID_MASK   0xf   //fid only use cdw10 bit[0:7]

struct nvme_host_mem_buf_desc {
	u64			addr;
	u32			size;
	u32			rsvd;
};

struct nvme_create_cq {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			rsvd1[5];
	u64			prp1;
	u64			rsvd8;
	u16			cqid;
	u16			qsize;
	u16			cq_flags;  //bit0:PC   bit1:IEN
	u16			irq_vector;
	u32			rsvd12[4];
};

struct nvme_create_sq {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			rsvd1[5];
	u64			prp1;
	u64			rsvd8;
	u16			sqid;
	u16			qsize;
	u16			sq_flags;
	u16			cqid;
	u32			rsvd12[4];
};

struct nvme_delete_queue {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			rsvd1[9];
	u16			qid;
	u16			rsvd10;
	u32			rsvd11[5];
};

struct nvme_abort_cmd {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			rsvd1[9];
	u16			sqid;
	u16			cid;
	u32			rsvd11[5];
};

struct nvme_download_firmware {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			rsvd1[5];
	union nvme_data_ptr	dptr;
	u32			numd;
	u32			offset;
	u32			rsvd12[4];
};

struct nvme_format_cmd {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[4];
	u32			cdw10;
	u32			rsvd11[5];
};

struct nvme_get_log_page_command {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[2];
	union nvme_data_ptr	dptr;
	u8			lid;
	u8			rsvd10;
	u16			numdl;
	u16			numdu;
	u16			rsvd11;
	u32			lpol;
	u32			lpou;
	u32			rsvd14[2];
};

struct nvme_directive_cmd {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			nsid;
	u64			rsvd2[2];
	union nvme_data_ptr	dptr;
	u32			numd;
	u8			doper;
	u8			dtype;
	u16			dspec;
	u8			endir;
	u8			tdtype;
	u16			rsvd15;

	u32			rsvd16[3];
};

/*
 * Fabrics subcommands.
 */
enum nvmf_fabrics_opcode {
	nvme_fabrics_command		= 0x7f,
};

enum nvmf_capsule_command {
	nvme_fabrics_type_property_set	= 0x00,
	nvme_fabrics_type_connect	= 0x01,
	nvme_fabrics_type_property_get	= 0x04,
};

struct nvmf_common_command {
	u8	opcode;
	u8	resv1;
	u16	command_id;
	u8	fctype;
	u8	resv2[35];
	u8	ts[24];
};

/*
 * The legal cntlid range a NVMe Target will provide.
 * Note that cntlid of value 0 is considered illegal in the fabrics world.
 * Devices based on earlier specs did not have the subsystem concept;
 * therefore, those devices had their cntlid value set to 0 as a result.
 */
#define NVME_CNTLID_MIN		1
#define NVME_CNTLID_MAX		0xffef
#define NVME_CNTLID_DYNAMIC	0xffff

#define MAX_DISC_LOGS	255

/* Discovery log page entry */
struct nvmf_disc_rsp_page_entry {
	u8		trtype;
	u8		adrfam;
	u8		subtype;
	u8		treq;
	u16		portid;
	u16		cntlid;
	u16		asqsz;
	u8		resv8[22];
	char		trsvcid[NVMF_TRSVCID_SIZE];
	u8		resv64[192];
	char		subnqn[NVMF_NQN_FIELD_LEN];
	char		traddr[NVMF_TRADDR_SIZE];
	union tsas {
		char		common[NVMF_TSAS_SIZE];
		struct rdma {
			u8	qptype;
			u8	prtype;
			u8	cms;
			u8	resv3[5];
			u16	pkey;
			u8	resv10[246];
		} rdma;
	} tsas;
};

/* Discovery log page header */
struct nvmf_disc_rsp_page_hdr {
	u64		genctr;
	u64		numrec;
	u16		recfmt;
	u8		resv14[1006];
	struct nvmf_disc_rsp_page_entry entries[0];
};

struct nvmf_connect_command {
	u8		opcode;
	u8		resv1;
	u16		command_id;
	u8		fctype;
	u8		resv2[19];
	union nvme_data_ptr dptr;
	u16		recfmt;
	u16		qid;
	u16		sqsize;
	u8		cattr;
	u8		resv3;
	u32		kato;
	u8		resv4[12];
};

struct nvmf_connect_data {
	uuid_t		hostid;
	u16		cntlid;
	char		resv4[238];
	char		subsysnqn[NVMF_NQN_FIELD_LEN];
	char		hostnqn[NVMF_NQN_FIELD_LEN];
	char		resv5[256];
};

struct nvmf_property_set_command {
	u8		opcode;
	u8		resv1;
	u16		command_id;
	u8		fctype;
	u8		resv2[35];
	u8		attrib;
	u8		resv3[3];
	u32		offset;
	u64		value;
	u8		resv4[8];
};

struct nvmf_property_get_command {
	u8		opcode;
	u8		resv1;
	u16		command_id;
	u8		fctype;
	u8		resv2[35];
	u8		attrib;
	u8		resv3[3];
	u32		offset;
	u8		resv4[16];
};

struct nvme_dbbuf {
	u8			opcode;
	u8			flags;
	u16			command_id;
	u32			rsvd1[5];
	u64			prp1;
	u64			prp2;
	u32			rsvd12[6];
};

struct streams_directive_params {
	u16	msl;
	u16	nssa;
	u16	nsso;
	u8	rsvd[10];
	u32	sws;
	u16	sgs;
	u16	nsa;
	u16	nso;
	u8	rsvd2[6];
};


struct nvme_command {
	union {
		struct nvme_common_command common;
		struct nvme_rw_command rw;
		struct nvme_identify identify;
		struct nvme_features features;
		struct nvme_create_cq create_cq;
		struct nvme_create_sq create_sq;
		struct nvme_delete_queue delete_queue;
		struct nvme_download_firmware dlfw;
		struct nvme_format_cmd format;
		struct nvme_dsm_cmd dsm;
		struct nvme_write_zeroes_cmd write_zeroes;
		struct nvme_abort_cmd abort;
		struct nvme_get_log_page_command get_log_page;
		struct nvmf_common_command fabrics;
		struct nvmf_connect_command connect;
		struct nvmf_property_set_command prop_set;
		struct nvmf_property_get_command prop_get;
		struct nvme_dbbuf dbbuf;
		struct nvme_directive_cmd directive;
	};
};

enum {
	NVME_SCT_GENERIC		 = 0x0,
	NVME_SCT_CMD_SPECIFIC	 = 0x1,
	NVME_SCT_MEDIA_ERROR	 = 0x2,
	// 3-6  is RSVD
	NVME_SCT_VENDOR_SPECIFIC = 0x7,
};

enum {
	/*
	 * Generic Command Status:
	 */
	NVME_SC_SUCCESS			= 0x0,
	NVME_SC_INVALID_OPCODE		= 0x1,
	NVME_SC_INVALID_FIELD		= 0x2,
	NVME_SC_CMDID_CONFLICT		= 0x3,
	NVME_SC_DATA_XFER_ERROR		= 0x4,
	NVME_SC_POWER_LOSS		= 0x5,
	NVME_SC_INTERNAL		= 0x6,
	NVME_SC_ABORT_REQ		= 0x7,
	NVME_SC_ABORT_QUEUE		= 0x8,
	NVME_SC_FUSED_FAIL		= 0x9,
	NVME_SC_FUSED_MISSING		= 0xa,
	NVME_SC_INVALID_NS		= 0xb,
	NVME_SC_CMD_SEQ_ERROR		= 0xc,
	NVME_SC_SGL_INVALID_LAST	= 0xd,
	NVME_SC_SGL_INVALID_COUNT	= 0xe,
	NVME_SC_SGL_INVALID_DATA	= 0xf,
	NVME_SC_SGL_INVALID_METADATA	= 0x10,
	NVME_SC_SGL_INVALID_TYPE	= 0x11,

	NVME_SC_SGL_INVALID_OFFSET	= 0x16,
	NVME_SC_SGL_INVALID_SUBTYPE	= 0x17,

	NVME_SC_LBA_RANGE		= 0x80,
	NVME_SC_CAP_EXCEEDED		= 0x81,
	NVME_SC_NS_NOT_READY		= 0x82,
	NVME_SC_RESERVATION_CONFLICT	= 0x83,

	/*
	 * Command Specific Status:
	 */
	NVME_SC_CQ_INVALID		= 0x100,
	NVME_SC_QID_INVALID		= 0x101,
	NVME_SC_QUEUE_SIZE		= 0x102,
	NVME_SC_ABORT_LIMIT		= 0x103,
	NVME_SC_ABORT_MISSING		= 0x104,
	NVME_SC_ASYNC_LIMIT		= 0x105,
	NVME_SC_FIRMWARE_SLOT		= 0x106,
	NVME_SC_FIRMWARE_IMAGE		= 0x107,
	NVME_SC_INVALID_VECTOR		= 0x108,
	NVME_SC_INVALID_LOG_PAGE	= 0x109,
	NVME_SC_INVALID_FORMAT		= 0x10a,
	NVME_SC_FW_NEEDS_CONV_RESET	= 0x10b,
	NVME_SC_INVALID_QUEUE		= 0x10c,
	NVME_SC_FEATURE_NOT_SAVEABLE	= 0x10d,
	NVME_SC_FEATURE_NOT_CHANGEABLE	= 0x10e,
	NVME_SC_FEATURE_NOT_PER_NS	= 0x10f,
	NVME_SC_FW_NEEDS_SUBSYS_RESET	= 0x110,
	NVME_SC_FW_NEEDS_RESET		= 0x111,
	NVME_SC_FW_NEEDS_MAX_TIME	= 0x112,
	NVME_SC_FW_ACIVATE_PROHIBITED	= 0x113,
	NVME_SC_OVERLAPPING_RANGE	= 0x114,
	NVME_SC_NS_INSUFFICENT_CAP	= 0x115,
	NVME_SC_NS_ID_UNAVAILABLE	= 0x116,
	NVME_SC_NS_ALREADY_ATTACHED	= 0x118,
	NVME_SC_NS_IS_PRIVATE		= 0x119,
	NVME_SC_NS_NOT_ATTACHED		= 0x11a,
	NVME_SC_THIN_PROV_NOT_SUPP	= 0x11b,
	NVME_SC_CTRL_LIST_INVALID	= 0x11c,

	/*
	 * I/O Command Set Specific - NVM commands:
	 */
	NVME_SC_BAD_ATTRIBUTES		= 0x180,
	NVME_SC_INVALID_PI		= 0x181,
	NVME_SC_READ_ONLY		= 0x182,
	NVME_SC_ONCS_NOT_SUPPORTED	= 0x183,

	/*
	 * I/O Command Set Specific - Fabrics commands:
	 */
	NVME_SC_CONNECT_FORMAT		= 0x180,
	NVME_SC_CONNECT_CTRL_BUSY	= 0x181,
	NVME_SC_CONNECT_INVALID_PARAM	= 0x182,
	NVME_SC_CONNECT_RESTART_DISC	= 0x183,
	NVME_SC_CONNECT_INVALID_HOST	= 0x184,

	NVME_SC_DISCOVERY_RESTART	= 0x190,
	NVME_SC_AUTH_REQUIRED		= 0x191,

	/*
	 * Media and Data Integrity Errors:
	 */
	NVME_SC_WRITE_FAULT		= 0x280,
	NVME_SC_READ_ERROR		= 0x281,
	NVME_SC_GUARD_CHECK		= 0x282,
	NVME_SC_APPTAG_CHECK		= 0x283,
	NVME_SC_REFTAG_CHECK		= 0x284,
	NVME_SC_COMPARE_FAILED		= 0x285,
	NVME_SC_ACCESS_DENIED		= 0x286,
	NVME_SC_UNWRITTEN_BLOCK		= 0x287,

	NVME_SC_DNR			= 0x4000,
};

typedef union nvme_result {
	u16 bit16;
	u32 bit32;
	u64 bit64;
} cqe_qw0;

struct nvme_completion {
	/*
	 * Used by Admin and Fabrics commands to return data:
	 */
	//cqe_qw0 result;	
	union nvme_result result;
	u16	sq_head;	/* how much of this queue may be reclaimed */
	u16	sq_id;		/* submission queue that generated this entry */
	u16	command_id;	/* of the command which completed */
	u16	status;		/* did the command fail, and if so, why? */
};

// self define, not from kernel nvme.h, rw dw12
typedef union
{
    u16  ctrl;
	
    struct
    {
        u16 rsvd1 	:4;
		u16 dtype 	:4;
        u16 rsvd2 	:2;
		u16 prchk	:3;
		u16 pract	:1;
		u16 fua		:1;		// fua enable, write through, read not search cache
        u16 lr		:1;     // limit retry, controller should apply limited retry efforts.
    } bits;
} rw_ctrl_t;


typedef union
{
	u32 dw13;

	struct
	{
		u32 dsm_frequency 		:4;  // write band
		u32 dsm_latency			:2;  // read read1/2/3
		u32 dsm_seq_req 		:1;  
		u32 dsm_incompressible 	:1;
		u32 rsvd				:8;
		u32 dspec				:16; // Directive Specific
	} bits;
} dsm_dw13_t;

typedef union
{
	u32 dw10;

	struct
	{
		u32 lbaf 	:4;   // LBA format 16 of which one
		u32 mset	:1;   // metadata settings, 1-DIF   0-DIX
		u32 pi 		:3;   // PI type, type0 means not enable
		u32 pil 	:1;	  // PI location
		u32 ses		:3;	  // secure erase settings
		u32 rsvd	:20;
	} bits;
} fmt_dw10_t;

enum {
	NVME_META_DIX = 0,
	NVME_META_DIF = 1,
};

enum {
	NVME_PIL_LAST_8B =  0,
	NVME_PIL_FIRST_8B = 1,
};

// Format NVM command security erase setting field
enum {
    NO_SECURE_ERASE		= 0x0,
    UER_DATA_ERASE		= 0x1,
    CTYPTOGRAPHIC_ERASE	= 0x2,
};


typedef struct {
	u16 guard;   //PRCHK bit2    CRC-16 computed over the LB data
	u16 apptag;  //PRCHK bit1	 
	u32 reftag;  //PRCHK bit0
} pi_fmt;

// NVMe Standard completion queue structure
typedef union
{
    u16 status;
	
    struct
    {
        u16 phase    	:1;     // NVMe phase tag field
        u16 sc          :8;		// Status code
        u16 sct         :3;		// Status code type
        u16 rsv         :2;		// Reserved
        u16 more        :1;     // more status information for this command
        u16 dnr         :1;     // Do not retry
    } bits;
} cqsts;


#endif
