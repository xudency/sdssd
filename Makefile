SUBDIR := drivers/nvme/host 
SUBDIR += drivers/fscftl
#SUBDIR += drivers/lightnvm
all:
	$(foreach N,$(SUBDIR),make -C $(N);)

clean:
	$(foreach N,$(SUBDIR),make clean -C $(N);)
