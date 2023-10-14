#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>

#define MAX_DEV 2

static ssize_t rw_cdevice_read(struct file *, char *, size_t, loff_t *);

static ssize_t rw_cdevice_write(struct file *file, const char __user *user_buffer,
                    size_t size, loff_t * offset);

static int rw_cdevice_open(struct inode *inode, struct file *file);

static int rw_cdevice_release(struct inode *inode, struct file *file);

struct rw_cdevice_data {
    struct cdev cdev;
};

static struct file_operations f_ops = {
    .owner = THIS_MODULE,
    .open = rw_cdevice_open,
    .release = rw_cdevice_release,

    .read = rw_cdevice_read,
    .write = rw_cdevice_write,
};

static int dev_major = 0;
static const char rw_cdevice_dev_name[] = "rw_cdevice";
static struct rw_cdevice_data device_datas[MAX_DEV];

static ssize_t rw_cdevice_read(struct file *file, char *to, size_t max_read, loff_t *offset) {
    printk("rw_cdevice: Device read\n");

    const char test_string[] = "baby device text\n";
    const size_t data_size = strlen(test_string);

    if (*offset >= data_size) {
        return 0;
    }
    
    int err = 0;
    ssize_t len = min((size_t) (data_size - *offset), max_read);

    int rest = copy_to_user(to, test_string, len);
    int transfered = len - rest;
    
    *offset += transfered;
    
    if (transfered == 0) {
        err = ENOMEM;
        goto fail;   
    }

    printk("rw_device: written:%d", transfered);
    return transfered;

    fail:
    return -err;
}

static ssize_t rw_cdevice_write(struct file *file, const char __user *user_buffer,
                    size_t size, loff_t * offset)
{
    printk("rw_cdevice: Device write\n");
    return size;
}


static int rw_cdevice_open(struct inode *inode, struct file *file){
   printk("rw_cdevice: Device open\n");
   return 0;
}

static int rw_cdevice_release(struct inode *inode, struct file *file){
   printk("rw_cdevice: Device closed\n");
   return 0;
}

int init_module(void) {
    printk("rw_cdevice: init\n");
    
    dev_t dev;
    int err = alloc_chrdev_region(&dev, 0, MAX_DEV, rw_cdevice_dev_name);
    if (err != 0) {
        goto fail;
    }

    dev_major = MAJOR(dev);

    for (int i = 0; i < MAX_DEV; ++i) {
        struct cdev* current_dev = &device_datas[i].cdev;
        
        cdev_init(current_dev, &f_ops);
        current_dev->owner = THIS_MODULE;

        cdev_add(current_dev, MKDEV(dev_major, i), 1);
    }

    fail:
    return err;
}

void cleanup_module(void) {
    printk("rw_cdevice: cleaned up\n");

    unregister_chrdev_region(dev_major, dev_major);
    for (int i = 0; i < MAX_DEV; ++i) {
        cdev_del(&(device_datas[i].cdev));
    }

    unregister_chrdev_region(MKDEV(dev_major, 0), MAX_DEV);
}

MODULE_LICENSE("GPL");