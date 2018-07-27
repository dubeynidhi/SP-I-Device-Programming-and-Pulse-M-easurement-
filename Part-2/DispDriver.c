#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/param.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/delay.h>	
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/kthread.h>

#define DEVICENAME 	 "SPIMAT"
#define DEVICECLASS	 "DISPLAY"
#define CONFIG 1
#define MINNUMBER    0
#define MJNUMBER 154 // major number of spidev



static struct spidev_data *devdat;
static struct class *spimatclass;
static struct spi_message smes;
unsigned int design[16];
uint8_t snd_ar[10][24];
uint8_t tx_ar[2];
/*structure for spi transfer  for message exchange*/
struct spi_transfer spitx = {

	.tx_buf = tx_ar,
	.rx_buf = 0,
	.len = 2,
	.bits_per_word = 8,
	.speed_hz = 10000000,
	.delay_usecs = 1,
	.cs_change = 1,
};

struct spidev_data {
	dev_t  devt;
	struct spi_device *spi;
};

int spithrfunc(void *data){
	
	printk("\n inside thread function of spi display");
	int i,sltm,j,l;
	for(i=0;i<16;i++){

		l=design[i];
		sltm=i+1;
		if(design[i]==0 && design[i+1]==0){	
			tx_ar[0] = 0x0c;
		    tx_ar[1] = 0x00;
		    spi_message_init(&smes);
			spi_message_add_tail((void *)&spitx, &smes);
			gpio_set_value(15,0);
			spi_sync(devdat->spi, &smes);
			gpio_set_value(15,1);
			//printk("\n inside clear call");

 		}
 		else{
			for(j=0;j<24;j=j+2){
				tx_ar[0] = snd_ar[l][j];
	    		tx_ar[1] = snd_ar[l][j+1];
	    		
				spi_message_init(&smes);
				spi_message_add_tail((void *)&spitx, &smes);
				gpio_set_value(15,0);
				spi_sync(devdat->spi, &smes);
				gpio_set_value(15,1);
				//printk("\n inside pattern display call %d\n",j);
			}   
		}
		i=i+1;
		msleep(sltm*300);
	}
	return 0;	
}
/*open function for the spi matrix device driver where the gpio pins connected to the spi device are initialized*/

int openspimat(struct inode *i, struct file *f){
	printk(KERN_INFO" initializing the gpio pins for the spi LED matrix\n");
	gpio_export(24,true);
	gpio_export(25,true);
	gpio_export(30,true);
	gpio_export(31,true);
	gpio_export(42,true);
	gpio_export(43,true);
	gpio_export(46,true);
	gpio_export(44,true);
	gpio_export(72,true);
	gpio_export(15,true);
	gpio_export(5,true);
	gpio_export(7,true);
	gpio_direction_output(5,1);
	gpio_set_value(5,1);
	gpio_direction_output(7,1);
	gpio_set_value(7,1);
	gpio_direction_output(46,1);
	gpio_set_value(46,1);
	gpio_direction_output(30,1);
	gpio_set_value(30,0);
	gpio_direction_output(31,1);
	gpio_set_value(31,0);
	gpio_set_value(72,0);
	gpio_direction_output(24,1);
	gpio_set_value(24,0);
	gpio_direction_output(25,1);
	gpio_set_value(25,0);
	gpio_direction_output(44,1);
	gpio_set_value(44,1);
	gpio_direction_output(42,1);
	gpio_set_value(42,0);
	gpio_direction_output(43,1);
	gpio_set_value(43,0);
	gpio_direction_output(15,1);
	gpio_set_value(15,0);
	return 0;
}

/*release function for the spi matrix device driver where the gpio pins connected to the spi device are released at the end of the program*/

int spimatrelease(struct inode *i, struct file *flpptr){
	printk(KERN_INFO"\n unexporting the gpio pins and releasing when closing");
	gpio_unexport(24);
	gpio_free(24);
	gpio_unexport(25);
	gpio_free(25);
	gpio_unexport(30);
	gpio_free(30);
	gpio_unexport(31);
	gpio_free(31);
	gpio_unexport(42);
	gpio_free(42);
	gpio_unexport(43);
	gpio_free(43);
	gpio_unexport(46);
	gpio_free(46);
	gpio_unexport(44);
	gpio_free(44);
	gpio_unexport(72);
	gpio_free(72);
	gpio_unexport(15);
	gpio_free(15);
	gpio_unexport(5);
	gpio_free(5);
	gpio_unexport(7);
	gpio_free(7);
	return 0;
}

ssize_t spimatwrite(struct file *f,const char *spim, size_t sz, loff_t *loffptr){
	int itr;
	uint8_t patseq[16];
	struct task_struct *taskspimat;
	copy_from_user((void *)&patseq, (void * __user)spim, sizeof(patseq));
	for(itr=0;itr<16;itr++)
	{
		design[itr] = patseq[itr];
		printk(" %d ",design[itr]);
	}
	taskspimat = kthread_run(&spithrfunc, (void *)design,"kthread_spi_led");
	msleep(1000);
	printk("exiting");
	return 0;	
}
//ioctl function fot the spi matrix display
static long spi_ioctl(struct file *f,unsigned int cmd, unsigned long spicnt)  
{

	int i=0, j=0;
	uint8_t rcd_ar[10][24];

	int vret;
   	printk("ioctl Start\n");
	vret = copy_from_user((void *)&rcd_ar,(void *)spicnt, sizeof(rcd_ar));
	if(vret != 0)
	{
		printk("Failure : %d number of bytes that could not be copied.\n",vret);
	}
	for(i=0;i<10;i++)
	{
		//storing the pattern in the global array
	for(j=0;j<24;j++)
	{
			snd_ar[i][j] = rcd_ar[i][j];
			printk(" %d ",snd_ar[i][j]);
	}
	}
	printk("ioctl End\n");
	return 0;
}


static const struct file_operations my_operations = {
	.open = openspimat,
	.release = spimatrelease,
	.write	= spimatwrite,
	.unlocked_ioctl = spi_ioctl,
	
};
// probe function for the spi matrix display
static int probespimat(struct spi_device *spi){
	//struct spidev_data *spidev;
	int status = 0;
	struct device *dev;

	/* Allocate driver data */
	devdat = kzalloc(sizeof(*devdat), GFP_KERNEL);
	if(!devdat)
	{
		return -ENOMEM;
	}
	devdat->spi = spi;

	devdat->devt = MKDEV(MJNUMBER, MINNUMBER);

    dev = device_create(spimatclass, &spi->dev, devdat->devt, devdat, DEVICENAME);

    if(dev == NULL)
    {
		printk(" Failed while creating device\n");
		kfree(devdat);
		return -1;
	}
	printk("Spi matrix probled.\n");
	return status;
}

static int spi_remove(struct spi_device *spi){
	device_destroy(spimatclass, devdat->devt);
	kfree(devdat);
	printk(" Driver Removed.\n");
	return 0;
}

struct spi_device_id leddeviceid[] = {
{"spidev",0},
{}
};
static struct spi_driver driverspimat = {
         .driver = {
            .name =         "spidev",
            .owner =        THIS_MODULE,
         },
	 	 .id_table =   leddeviceid,
         .probe =        probespimat,
         .remove =       spi_remove,
		
};

int __init driverspimat_init(void)
{
	
	int vret;
	
	//Register the Device
	vret = register_chrdev(MJNUMBER, DEVICENAME, &my_operations);
	if(vret < 0)
	{
		printk(" Registration of the device ailed\n");
		return -1;
	}
	
	//Create the class
	spimatclass = class_create(THIS_MODULE, DEVICECLASS);
	if(spimatclass == NULL)
	{
		printk("Class Creation Failed\n");
		unregister_chrdev(MJNUMBER, driverspimat.driver.name);
		return -1;
	}
	
	//Register the Driver
	vret = spi_register_driver(&driverspimat);
	if(vret < 0)
	{
		printk("Driver Registraion Failed\n");
		class_destroy(spimatclass);
		unregister_chrdev(MJNUMBER, driverspimat.driver.name);
		return -1;
	}
	
	printk("Initialized.\n");
	return 0;
}

void __exit driverspimat_exit(void)
{
	spi_unregister_driver(&driverspimat);
	class_destroy(spimatclass);
	unregister_chrdev(MJNUMBER, driverspimat.driver.name);
	
}



module_init(driverspimat_init);
module_exit(driverspimat_exit);
MODULE_LICENSE("GPL v2");

