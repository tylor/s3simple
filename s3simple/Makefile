obj-m := hellofs.o
KDIR := /usr/src/linux-headers-2.6.31-17-generic/
PWD := $(shell pwd -L)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
