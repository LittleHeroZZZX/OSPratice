BASEINCLUDE ?= /usr/src/linux-headers-`uname -r`
my_copy_file-objs := copyfile.o
obj-m := my_copy_file.o
all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;
clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;

