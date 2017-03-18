PWD := $(shell pwd)

SUBDIR := drivers/nvme/host 
SUBDIR += drivers/fscftl
#SUBDIR += drivers/lightnvm
all:
	$(foreach N,$(SUBDIR),make -C $(N);)
	
	cp drivers/nvme/host/nvme.ko $(PWD)/
	cp drivers/fscftl/fscftl.ko $(PWD)/

clean:
	$(foreach N,$(SUBDIR),make clean -C $(N);)
	rm $(PWD)/*.ko
