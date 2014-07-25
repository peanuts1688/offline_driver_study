#obj-m	:= tc_pcie_driver.o

#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#PWD       := $(shell pwd)

#CPPFLAGS += -include $(KERNELDIR)/include/generated/autoconf.h

all:
	#$(MAKE) -C $(KERNELDIR) M=$(PWD)
	gcc -o app tc_pcie_app.c
	
#install:
#	./tc_pcie_driver_load

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions app

