ifeq ($(KERNELRELEASE),)
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)


default:
	${MAKE} -C ${KERNELDIR} M=${PWD} modules

clean:
	${MAKE} -C $(KERNELDIR) M=${PWD} clean

else
	obj-m := pid_csw.o

endif