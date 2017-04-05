#ifndef __FSCFTL_BACKEND_DEV__
#define __FSCFTL_BACKEND_DEV__

#include "../fscftl.h"

#define FSCFTL_IOCTL_USER_PPA_CMD    _IOW('M', 0x90, struct nvme_user_io)

int backend_miscdev_create(struct nvm_exdev *dev);
void backend_miscdev_delete(struct nvm_exdev *dev);

#endif
