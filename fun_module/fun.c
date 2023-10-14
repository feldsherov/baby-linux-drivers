#include "fun_lib.h"

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Fun kernel module");
MODULE_LICENSE("GPL");

static int fun_init(void)
{
        fun_do_init();
        return 0;
}

static void fun_exit(void)
{
        fun_do_exit();
}

module_init(fun_init);
module_exit(fun_exit);