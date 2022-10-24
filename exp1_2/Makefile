BASEINCLUDE ?= /root/kernel
list_threadInfo_by_pid-objs := list_process_by_pid.o
obj-m := list_threadInfo_by_pid.o
all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;
clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;

