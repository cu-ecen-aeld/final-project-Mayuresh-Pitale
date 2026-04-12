#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>   // NEW: For IRQs
#include <linux/gpio.h>        // NEW: For GPIO 4
#include <linux/workqueue.h>   // NEW: For Bottom-Half
#include <linux/wait.h>        // NEW: For sleeping the read function
#include <linux/mutex.h>       // NEW: To protect our data buffer

#define DEVICE_NAME "vibra_sensor"
#define CLASS_NAME  "vibra"
#define LSM6DSOX_ADDR 0x6A
#define GPIO_INT_PIN  4        // BCM GPIO 4 (Physical Pin 7)

// LSM6DSOX Registers
#define LSM6DSOX_WHO_AM_I  0x0F
#define LSM6DSOX_CTRL1_XL  0x10  // Accel Control
#define LSM6DSOX_INT1_CTRL 0x0D  // NEW: Interrupt Control Register
#define LSM6DSOX_OUTX_L_A  0x28  // Data Register

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mayuresh Pitale");
MODULE_DESCRIPTION("LSM6DSOX I2C Interrupt-Driven Driver");

static int major_number;
static struct class* vibra_class  = NULL;
static struct device* vibra_device = NULL;
static struct i2c_client *lsm6dsox_client;

// --- NEW: Interrupt & Concurrency Variables ---
static unsigned int irq_number;
static struct work_struct sensor_work;

static u8 latest_data[6];             // Global buffer for X, Y, Z
static DEFINE_MUTEX(data_mutex);      // Protects latest_data
static DECLARE_WAIT_QUEUE_HEAD(data_wait); // Allows .read to sleep
static bool new_data_ready = false;   // Flag to wake up .read

// ---------------------------------------------------------
// 1. BOTTOM HALF: The Workqueue (Runs in background)
// ---------------------------------------------------------
static void sensor_work_func(struct work_struct *work) {
    int ret;
    u8 temp_data[6];

    // Read I2C (Safe to do here, because workqueues are allowed to sleep)
    ret = i2c_smbus_read_i2c_block_data(lsm6dsox_client, LSM6DSOX_OUTX_L_A, 6, temp_data);
    
    if (ret == 6) {
        // Lock the mutex, update our global buffer, unlock
        mutex_lock(&data_mutex);
        memcpy(latest_data, temp_data, 6);
        new_data_ready = true;
        mutex_unlock(&data_mutex);

        // Wake up anyone trying to 'read' from /dev/vibra_sensor
        wake_up_interruptible(&data_wait);
    }
}

// ---------------------------------------------------------
// 2. TOP HALF: The Hard IRQ (Must be incredibly fast)
// ---------------------------------------------------------
static irqreturn_t sensor_irq_handler(int irq, void *dev_id) {
    // Acknowledge the hardware tap, schedule the bottom half, and exit immediately!
    schedule_work(&sensor_work);
    return IRQ_HANDLED;
}

// ---------------------------------------------------------
// 3. FILE OPERATIONS
// ---------------------------------------------------------
static int dev_open(struct inode *inodep, struct file *filep) { return 0; }
static int dev_release(struct inode *inodep, struct file *filep) { return 0; }

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    if (len < 6) return -EINVAL;

    // Go to sleep until the Workqueue says "new_data_ready == true"
    if (wait_event_interruptible(data_wait, new_data_ready)) {
        return -ERESTARTSYS; // Handle user pressing Ctrl+C while waiting
    }

    // Data is ready! Lock it, copy it, and reset the flag.
    mutex_lock(&data_mutex);
    if (copy_to_user(buffer, latest_data, 6) != 0) {
        mutex_unlock(&data_mutex);
        return -EFAULT;
    }
    new_data_ready = false;
    mutex_unlock(&data_mutex);

    return 6;
}

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .release = dev_release,
};

// ---------------------------------------------------------
// 4. INIT & EXIT
// ---------------------------------------------------------
static int __init vibra_init(void) {
    struct i2c_adapter *adapter;
    struct i2c_board_info board_info = { I2C_BOARD_INFO("lsm6dsox", LSM6DSOX_ADDR) };

    // Set up Char Device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    vibra_class = class_create(THIS_MODULE, CLASS_NAME);
    vibra_device = device_create(vibra_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);

    // Set up I2C
    adapter = i2c_get_adapter(1);
    lsm6dsox_client = i2c_new_client_device(adapter, &board_info);
    i2c_put_adapter(adapter);

    // Initialize the Workqueue
    INIT_WORK(&sensor_work, sensor_work_func);

    // Set up GPIO 4 as an Interrupt
    gpio_request(GPIO_INT_PIN, "sysfs");
    gpio_direction_input(GPIO_INT_PIN);
    irq_number = gpio_to_irq(GPIO_INT_PIN);
    
    // Request the IRQ to trigger on a Rising Edge (Low to High voltage)
    request_irq(irq_number, sensor_irq_handler, IRQF_TRIGGER_RISING, "vibra_irq", NULL);
    printk(KERN_INFO "Vibra: Mapped GPIO %d to IRQ %d\n", GPIO_INT_PIN, irq_number);

    // Configure LSM6DSOX Hardware
    // 1. Enable Data-Ready Interrupt on INT1 pin (Bit 0 = 1)
    i2c_smbus_write_byte_data(lsm6dsox_client, LSM6DSOX_INT1_CTRL, 0x01);
    
    // 2. Turn on Accelerometer at 104Hz
    i2c_smbus_write_byte_data(lsm6dsox_client, LSM6DSOX_CTRL1_XL, 0x40);

    return 0;
}

static void __exit vibra_exit(void) {
    // Turn off sensor and interrupts
    if (lsm6dsox_client) {
        i2c_smbus_write_byte_data(lsm6dsox_client, LSM6DSOX_CTRL1_XL, 0x00);
        i2c_smbus_write_byte_data(lsm6dsox_client, LSM6DSOX_INT1_CTRL, 0x00);
        i2c_unregister_device(lsm6dsox_client);
    }
    
    // Free IRQ, GPIO, and Workqueue
    free_irq(irq_number, NULL);
    gpio_free(GPIO_INT_PIN);
    cancel_work_sync(&sensor_work); // Ensure workqueue is dead before exiting

    device_destroy(vibra_class, MKDEV(major_number, 0));
    class_unregister(vibra_class);
    class_destroy(vibra_class);
    unregister_chrdev(major_number, DEVICE_NAME);
}

module_init(vibra_init);
module_exit(vibra_exit);
