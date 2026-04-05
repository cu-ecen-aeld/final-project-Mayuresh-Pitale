#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "vibra_sensor"
#define CLASS_NAME  "vibra"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mayuresh Pitale");
MODULE_DESCRIPTION("LSM6DSOX Character Driver Skeleton");

static int major_number;
static struct class* vibra_class  = NULL;
static struct device* vibra_device = NULL;

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Vibra: Device has been opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Vibra: Device successfully closed\n");
    return 0;
}

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
};

static int __init vibra_init(void) {
    // 1. Register major number
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) return major_number;

    // 2. Create the device class
    vibra_class = class_create(THIS_MODULE, CLASS_NAME);
    
    // 3. Create the device node (/dev/vibra_sensor)
    vibra_device = device_create(vibra_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    
    printk(KERN_INFO "Vibra: Initialized with major number %d\n", major_number);
    return 0;
}

static void __exit vibra_exit(void) {
    device_destroy(vibra_class, MKDEV(major_number, 0));
    class_unregister(vibra_class);
    class_destroy(vibra_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Vibra: Driver unloaded\n");
}

module_init(vibra_init);
module_exit(vibra_exit);
