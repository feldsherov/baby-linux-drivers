obj-m += fun_module.o 
fun_module-y := fun.o fun_lib.o
 
PWD := $(CURDIR)
KBUILD := /lib/modules/$(shell uname -r)/build
 
all: 
	make -C $(KBUILD) M=$(PWD) modules 
 
clean: 
	make -C $(KBUILD) M=$(PWD) clean