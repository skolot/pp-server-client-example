KERNEL_SOURCE_DIR?=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)
V?=0
VERBOSE?=${V}
EXTRA_CFLAGS=

obj-m:=mycom.o

all:
	$(MAKE) -C $(KERNEL_SOURCE_DIR) M=$(PWD) KBUILD_VERBOSE=${VERBOSE} modules

distclean: clean
clean:
	${MAKE} -C $(KERNEL_SOURCE_DIR) M=$(PWD) KBUILD_VERBOSE=${VERBOSE} clean

install:
	${MAKE} -C $(KERNEL_SOURCE_DIR) M=$(PWD) KBUILD_VERBOSE=${VERBOSE} modules_install
