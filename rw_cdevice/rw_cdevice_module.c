#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/circ_buf.h>

#define MAX_DEV 2
#define RWCDEV_BUFFER_SIZE (1 << 10)
#define RWCDEV_NAME "rw_cdevice"

static ssize_t rw_cdevice_read(struct file *, char *, size_t, loff_t *);

static ssize_t rw_cdevice_write(struct file *file, const char __user *user_buffer,
                    size_t size, loff_t * offset);

static int rw_cdevice_open(struct inode *inode, struct file *file);

static int rw_cdevice_release(struct inode *inode, struct file *file);


struct rw_cdevice_data {
    struct cdev cdev;
    char data[RWCDEV_BUFFER_SIZE];
    struct circ_buf crc;
};

static struct file_operations f_ops = {
    .owner = THIS_MODULE,
    .open = rw_cdevice_open,
    .release = rw_cdevice_release,

    .read = rw_cdevice_read,
    .write = rw_cdevice_write,
};

static int dev_major = 0;
static struct rw_cdevice_data device_datas[MAX_DEV];

// Returns how many bytes was transferred.
size_t copy_from_user_to_crc(const char *from, size_t size, struct circ_buf* crc) {
    size_t space = CIRC_SPACE_TO_END(crc->head, crc->tail, RWCDEV_BUFFER_SIZE);
    int to_write = min(space, size);
    int rest = 0;
    int transferred = 0;

    rest = copy_from_user(crc->buf + crc->head, from, to_write);
    transferred = to_write - rest;
    crc->head = (crc->head + transferred) & (RWCDEV_BUFFER_SIZE - 1);    

    return transferred;
}

// Returns how many bytes was transferred.
size_t copy_to_user_from_crc(char *to, size_t max_read, struct circ_buf* crc) {
    size_t data_size = CIRC_CNT_TO_END(crc->head, crc->tail, RWCDEV_BUFFER_SIZE);
    int to_read = min(max_read, data_size);
    int rest;
    int transferred;

    rest = copy_to_user(to, crc->buf + crc->tail, to_read);
    transferred = to_read - rest;
    
    crc->tail = (crc->tail + transferred) & (RWCDEV_BUFFER_SIZE - 1);
    return transferred;
}

static ssize_t rw_cdevice_read(struct file *file, char *to, size_t max_read, loff_t *offset) {
    int minor = iminor(file_inode(file));
    struct rw_cdevice_data *ddata = &device_datas[minor];
    int err = 0;
    int transferred = 0;

    printk("rw_cdevice: Device read\n");

    if (CIRC_CNT(ddata->crc.head, ddata->crc.tail, RWCDEV_BUFFER_SIZE) == 0) {
        return 0;
    }
    
    transferred = copy_to_user_from_crc(to, max_read, &ddata->crc);
    if (transferred == 0) {
        err = ENOMEM;
        goto fail;   
    }

    printk("rw_device: read:%d\n", transferred);
    return transferred;

    fail:
    printk("rw_device: fail:\n");
    return -err;
}

static ssize_t rw_cdevice_write(struct file *file, const char __user *user_buffer,
                    size_t size, loff_t * offset)
{
    int minor = iminor(file_inode(file));
    struct rw_cdevice_data *ddata = &device_datas[minor];
    
    printk("rw_cdevice: Device write\n");
    
    return copy_from_user_to_crc(user_buffer, size, &ddata->crc);
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
    dev_t dev;
    struct cdev* current_dev;
    int err;

    printk("rw_cdevice: init\n");
    
    err = alloc_chrdev_region(&dev, 0, MAX_DEV, RWCDEV_NAME);
    if (err != 0) {
        goto fail;
    }

    dev_major = MAJOR(dev);

    for (int i = 0; i < MAX_DEV; ++i) {
        device_datas[i].crc.buf = device_datas[i].data;
        device_datas[i].crc.head = device_datas[i].crc.tail = 0;
        
        current_dev = &device_datas[i].cdev;
        
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