  PWD := $(shell pwd)
  KVER := $(shell uname -r)
  KDIR := /lib/modules/$(KVER)/build
  # ccflags-y += -Werror
  
  ## pblk.ko
  pblk-objs := core.o pblk.o 
  pblk-objs += pblk-init.o pblk-core.o pblk-rb.o pblk-write.o pblk-cache.o pblk-read.o
  pblk-objs += pblk-gc.o pblk-recovery.o pblk-map.o pblk-rl.o pblk-sysfs.o
  obj-m  = pblk.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm .tmp_versions/ -rf
