#include "fun_lib.h"

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

void fun_do_init(void) {
    printk("Fun init");
}

void fun_do_exit(void) {
    printk("Fun exit");
}