KERNROOT := /lib/modules/$(shell uname -r)/build

fscftl:
	@cat ../nvme/host/Module.symvers >> ./Module.symvers
	
	$(MAKE) -C $(KERNROOT) M=$(shell pwd)
	

clean:
	@rm -rf *.tmp
	$(MAKE) -C $(KERNROOT) M=$(shell pwd) clean

.PHONY: all clean cnex

