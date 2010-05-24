obj-m := hellofs.o
KDIR := /usr/src/linux-headers-2.6.31-17-generic/
PWD := $(shell pwd -L)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	g++ -lcurl getinmemory.cpp -o getinmemory
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rf *o getinmemory *~
