#ifndef _PPA_OPS_H_
#define _PPA_OPS_H_

#include "../fscftl.h"

int nvm_rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
					  int nr_ppas, u16 ctrl, void *databuf, void *metabuf);



void run_testcase(struct nvm_exdev *exdev);

#endif
