KERN_PATH?=/lib/modules/`uname -r`/build/

EXTRA_CFLAGS  += -DMODULE=1 -D__KERNEL__=1


petmem-y := 	main.o      \
		swap.o      \
		buddy.o     \
		file_io.o   \
		on_demand.o 

petmem-objs := $(petmem-y)
obj-m       := petmem.o


all:
	$(MAKE) -C $(KERN_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERN_PATH) M=$(PWD) clean

