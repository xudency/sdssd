  PWD := $(shell pwd)
  KVER := $(shell uname -r)
  KDIR := /lib/modules/$(KVER)/build
  #ccflags-y += -DEXPORT_SYMTAB
  
  # Cheader file
  export NOSTDINC_FLAGS
  NOSTDINC_FLAGS += -I$(PWD)
  
  ## nvme.ko
  nvme-objs := pci.o
  obj-m  = nvme.o

  ## nvme-core.ko
  nvme-core-objs := core.o lightnvm.o
  obj-m += nvme-core.o

all:
	$(MAKE) $(ccflags-y) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) $(ccflags-y) -C $(KDIR) M=$(PWD) clean
