KERNROOT := /lib/modules/$(shell uname -r)/build

fscftl:
	#@cp -f ../linux_drv/Module.symvers ./
	#@cat ../linux_lib/Module.symvers >> ./Module.symvers
	
	$(MAKE) -C $(KERNROOT) M=$(shell pwd)
	
	#$(MAKE) -C $(FTL_MANAGERROOT) M=$(shell pwd)

clean:
	@rm -rf *.tmp
	$(MAKE) -C $(KERNROOT) M=$(shell pwd) clean
	#$(MAKE) -C $(FTL_MANAGERROOT) M=$(shell pwd) clean

.PHONY: all clean cnex

