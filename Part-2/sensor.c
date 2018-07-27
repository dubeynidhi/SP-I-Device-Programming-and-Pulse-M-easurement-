// Ultrasonic driver function


#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#define  DEVICE_NAME   "pulse"


static dev_t pulse_dnum;            // Allotted device number

struct pulse_dev
{
struct cdev cdev;               /* The cdev structure */
char name[20];                  /* Name of device*/
char in_string[256];            /* buffer for the input string */
int current_write_pointer;      // current file ptr
unsigned int flag;
unsigned long long int trise;
unsigned long long int tfall;
int  irq;
} *pulse_ptr;



static struct class*  pulseclass  = NULL;
static struct device* pulsedevice = NULL;


static int     pulse_open(struct inode *, struct file *);
static int     pulse_release(struct inode *, struct file *);
static ssize_t pulse_write(struct file *, const char *, size_t, loff_t *);
static ssize_t pulse_read(struct file *,  char *, size_t, loff_t *);

static __inline__ unsigned long long rdtsc(void)
{
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}


int irq_line;
static int edge=0; // 0 is Rising, 1 is falling

// Open function
int pulse_open(struct inode *inode, struct file *file)
{
	struct pulse_dev *pulse_ptr;
	printk("\nopening\n");

	/* Get the per-device structure that contains this cdev */
	pulse_ptr = container_of(inode->i_cdev, struct pulse_dev, cdev);
	/* Easy access to cmos_devp from rest of the entry points */

	file->private_data = pulse_ptr;
	printk("\n%s is openning \n", pulse_ptr->name);

	return 0;
}


// Release function
int pulse_release(struct inode *inode, struct file *file)
{
	struct pulse_dev *pulse_ptr = file->private_data;
    pulse_ptr->flag = 0;

	printk("\n%s is closing\n", pulse_ptr->name);

	return 0;
}

// Write function
ssize_t pulse_write(struct file *file, const char *buf,size_t count, loff_t *ppos)
{
	int retValue = 0;

	if(pulse_ptr->flag == 1)
	{
		retValue = -EBUSY;
		return -EBUSY;
	}

//generate trigger pulse
	gpio_set_value_cansleep(13, 0);
	udelay(15);
	gpio_set_value_cansleep(13, 1);
	udelay(20);
	gpio_set_value_cansleep(13, 0);
	pulse_ptr->flag = 1;

	return retValue;
}


// Read function
ssize_t pulse_read(struct file *file,  char *buf, size_t count, loff_t *ppos)
{
	int retValue=0;
	unsigned int dis;
	unsigned long long diff;
	
	if(pulse_ptr->flag == 1)
	{
		printk("Read is busy\n");
		return -EBUSY;
	}
	else
	{
		if(pulse_ptr->trise == 0 && pulse_ptr->tfall == 0)
		{
			printk("Trigger the measure first\n");
		}
		else
		{
			diff = pulse_ptr->tfall - pulse_ptr->trise;
			dis = div_u64(diff,400);
			retValue = copy_to_user((void *)buf, (const void *)&dis, sizeof(dis));
		}
	}
	printk("pulse_read() End\n");
	return retValue;
}


static irqreturn_t change_irq(int irq, void *dev_id)
{
	//printk("Change_irq Start\n");
	if(edge==0)
	{
		//printk("Rising edge interrupt\n");
		pulse_ptr->trise = rdtsc();
	        irq_set_irq_type(irq, IRQF_TRIGGER_FALLING);
	        edge=1;
	}
	else
	{
		//printk("Falling edge interrupt\n");
		pulse_ptr->tfall = rdtsc();
	        irq_set_irq_type(irq, IRQF_TRIGGER_RISING);
	        edge=0;
		pulse_ptr->flag = 0;
	}
	//printk("change_irq End\n");
	return IRQ_HANDLED;

}

static struct file_operations fops =
{
   .owner = THIS_MODULE,
   .open = pulse_open,
   .write = pulse_write,
   .read=pulse_read,
   .release = pulse_release,

};

// Driver module init

static int __init pulse_init(void)
{
	int ret,irq_req_rising;

	printk("Pulse driver initialized.\n");// '%s'\n",gmem_devp->in_string);
	printk("Setting direction .......\n");// '%s'\n",gmem_devp->in_string);

	gpio_request(13, "sysfs");          // gpioLED is requested
	gpio_request(14, "sysfs");
	gpio_request(34, "sysfs");
	gpio_request(16, "sysfs");          // gpioLED is requested
	gpio_request(17, "sysfs");
	gpio_request(37, "sysfs");
	gpio_request(77, "sysfs");          // gpioLED is requested
	gpio_request(76, "sysfs");
	gpio_request(64, "sysfs");

	gpio_export(13, false);
	gpio_export(14, false);
	gpio_export(34, false);
	gpio_export(16, false);
	gpio_export(17, false);
	gpio_export(37, false);
	gpio_export(77, false);
	gpio_export(76, false);
	gpio_export(64, false);

	gpio_direction_output(13,0);   // Set the gpio to be in output mode and on
	gpio_direction_input(14);
	gpio_direction_output(34,0);
	gpio_direction_output(16,1);   // Set the gpio to be in output mode and on
	gpio_direction_output(17,0);
	gpio_direction_output(37,0);
	gpio_direction_output(77,0);   // Set the gpio to be in output mode and on
	gpio_direction_output(76,0);
	gpio_direction_output(64,0);

    // Request dynamic allocation of a device major number
    if (alloc_chrdev_region(&pulse_dnum, 0, 1, DEVICE_NAME) < 0) {
    	printk(KERN_DEBUG "Can't register device\n"); return -1;
  	}

 	// Populate sysfs entries
    pulseclass = class_create(THIS_MODULE, DEVICE_NAME);

  	/* Allocate memory for the per-device structure */
	pulse_ptr = kmalloc(sizeof(struct cdev), GFP_KERNEL);

	/* Request I/O region */
    sprintf(pulse_ptr->name, DEVICE_NAME);

    // Connect the file operations with the cdev
	cdev_init(&pulse_ptr->cdev, &fops);
	pulse_ptr->cdev.owner = THIS_MODULE;

    // Connect the major/minor number to the cdev
    ret = cdev_add(&pulse_ptr->cdev, (pulse_dnum), 1);

    if (ret) {
    	printk("Bad cdev\n");
    	return ret;
    }

    //Send uevents to udev, so it'll create /dev nodes
    pulsedevice = device_create(pulseclass, NULL, MKDEV(MAJOR(pulse_dnum), 0), NULL, DEVICE_NAME);   
    //device_create_file(pulse_dev_device, &dev_attr_xxx);

	irq_line = gpio_to_irq(14);

	if(irq_line < 0)
	{
		printk("Here Gpio %d cannot be used as interrupt",14);

		ret=-EINVAL;
	}
	pulse_ptr->irq = irq_line;
	pulse_ptr->current_write_pointer = 0;

	pulse_ptr->trise=0;
	pulse_ptr->tfall=0;

	irq_req_rising = request_irq(irq_line, change_irq, IRQF_TRIGGER_RISING, "intr_chg", pulse_ptr);
	if(irq_req_rising)
	{
		printk("Unable to claim irq %d; error %d\n ", irq_line, irq_req_rising);
		return 0;
	}
	printk("\n irq request no. %d ",irq_req_rising);
	printk("\n irq no. %d ",pulse_ptr->irq);
	printk("The pin is mapped to IRQ: %d\n", irq_line);

	return 0;
}


static void __exit pulse_exit(void)
{
	cdev_del(&pulse_ptr->cdev);
	 /* Destroy device */
  	device_destroy (pulseclass, MKDEV(MAJOR(pulse_dnum), 0));
    /* Destroy driver_class */
    class_destroy(pulseclass);
    /* Release the major number */
 	unregister_chrdev_region((pulse_dnum), 1);
 
	kfree(pulse_ptr);
    free_irq(irq_line,(void *)pulse_ptr);

	gpio_unexport(13);               // Unexport the Button GPIO
	gpio_free(13);                      // Free the LED GPIO
	gpio_unexport(14);               // Unexport the Button GPIO
	gpio_free(14);                      // Free the LED GPIO
	gpio_unexport(34);               // Unexport the Button GPIO
	gpio_free(34);                      // Free the LED GPIO
	gpio_unexport(16);               // Unexport the Button GPIO
	gpio_free(16);                      // Free the LED GPIO
	gpio_unexport(17);               // Unexport the Button GPIO
	gpio_free(17);                      // Free the LED GPIO
	gpio_unexport(37);               // Unexport the Button GPIO
	gpio_free(37);                      // Free the LED GPIO
	gpio_unexport(77);               // Unexport the Button GPIO
	gpio_free(77);                      // Free the LED GPIO
	gpio_unexport(76);               // Unexport the Button GPIO
	gpio_free(76);                      // Free the LED GPIO
	gpio_unexport(64);               // Unexport the Button GPIO
	gpio_free(64);                      // Free the LED GPIO
 
    printk("Pulse sensor driver removed.\n");
}



module_init(pulse_init);
module_exit(pulse_exit);
MODULE_LICENSE("GPL v2");
