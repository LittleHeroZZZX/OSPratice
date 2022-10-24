BASEINCLUDE ?= /root/kernel
my_demo_list_all_threads-objs := my_process_list.o
obj-m := my_demo_list_all_threads.o
# Use these options only to determine compiler include directories
all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;
clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;

