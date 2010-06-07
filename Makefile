obj-m := s3simple.o
KDIR := /usr/src/linux-headers-$(shell uname -r)/
PWD := $(shell pwd -L)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	g++ -lcurl getinmemory.cpp -o s3simple
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -rf *o s3simple
