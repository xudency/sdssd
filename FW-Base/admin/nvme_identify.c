

#define MAX_NSID  15     // 0 1 2 ... 15
#define TOTAL_NS_NUM  (MAX_NSID + 1)


struct nvme_id_ns gat_identify_namespaces[TOTAL_NS_NUM];

struct nvme_id_ctrl gat_identify_controller;


struct nvme_id_ns *get_identify_ns(u32 nsid)
{
	return gat_identify_namespaces + nsid;
}

void identify_data_init(void)
{
	int i;
	
	for (i = 0; i < TOTAL_NS_NUM; i++) {
		identify_namespace_init(i);
	}

	identify_controller_init();

	return 0;
}

void identify_namespace_init(u32 nsid)
{
	struct nvme_id_ns *nsdata = get_identify_ns(nsid);

	nsdata->nsze = ;
	nsdata->ncap = ;

	// TODO:
}
