obj-m := hellofs.o
KDIR := /usr/src/linux-headers-$(shell uname -r)/
PWD := $(shell pwd -L)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	g++ -lcurl getinmemory.cpp -o getinmemory
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rf *o getinmemory
