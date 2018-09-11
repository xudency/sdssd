#ifndef __NVME_ADMIN_H__
#define __NVME_ADMIN_H__


typedef int (*set_feature_fn)(host_nvme_cmd_entry *);
typedef int (*get_feature_fn)(host_nvme_cmd_entry *);


typedef struct {
	struct nvme_arbitration_feat arbitration;
	struct nvme_power_mgmt_feat power_mgmt;
	struct nvme_lba_range_type lba_range[NVME_LBART_MAX_ENTRYS];
	struct nvme_tmperature_th_feat tmperature[];
	struct nvme_error_recov_feat err_recov;
	struct nvme_volatile_cache_feat vcache;
	struct nvme_queue_number qnumber;

	// TODO: other fid
	
} nvme_feature_cfg;


#endif
