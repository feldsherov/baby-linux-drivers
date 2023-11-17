obj-m += rw_cdevice_module.o 
 
PWD := $(CURDIR) 
 
all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
 
clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insmod:
	insmod rw_cdevice_module.ko

rmmod:
	rmmod rw_cdevice_module.ko

test:
	./test.sh