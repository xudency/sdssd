KERNROOT := /lib/modules/$(shell uname -r)/build

dftl:	
	$(MAKE) -C $(KERNROOT) M=$(shell pwd)
	
clean:
	@rm -rf *.tmp
	$(MAKE) -C $(KERNROOT) M=$(shell pwd) clean

.PHONY: all clean cnex

