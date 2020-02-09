#include<linux/init.h>
#include<linux/module.h>
#include<linux/version.h>
#include<linux/kernel.h>
#include<linux/types.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>
#include<linux/random.h>
#include<linux/errno.h>


#define MAGIC_NUMBER 'A'
#define CHANNEL_SELECT_SEQNO 1
#define RESULT_ALIGNMENT_SEQNO 2

#define CHANNEL_SELECT _IOW(MAGIC_NUMBER, CHANNEL_SELECT_SEQNO, unsigned int)
#define RESULT_ALIGNMENT _IOW(MAGIC_NUMBER, RESULT_ALIGNMENT_SEQNO, unsigned int)


static dev_t dd_ff;		

static struct cdev c_dev;	
static struct class *cluss;	


uint16_t channel;
uint16_t align;



static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "ADC8");
	return 0;
}

static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "close()/");
	return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	uint16_t result;
	printk(KERN_INFO "ADC8 device : read()\n");
	get_random_bytes(&result, 2);
	printk(KERN_INFO "radm value %d\n",result);
	result = result%1024;
	copy_to_user(buf, &result, sizeof(result));
	printk(KERN_INFO "value from chanel - %d is %d\n",channel, result);
	return 0;
}

static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{

	switch(cmd){
		case CHANNEL_SELECT:
		printk(KERN_INFO "SELECT ADC CHANNEL_SELECT");
		copy_from_user(&channel, (uint16_t *) arg, sizeof(channel));
		printk(KERN_INFO "ADC8 device : Selected Channel - %d\n",channel);
		break;
		case RESULT_ALIGNMENT:
		printk(KERN_INFO "ADC8 device : IOCTL - RESULT_ALIGNMENT\n");
		copy_from_user(&align, (uint16_t *) arg, sizeof(align));
		printk(KERN_INFO "ADC8 device : Selected Alignment - %d\n",align);
		break;
		default:
		return -ENOTTY;
		}
	return 0;
}

static struct file_operations fops = 
				{
				.owner	= THIS_MODULE,
				.open	= my_open,
				.release= my_close,
				.read	= my_read,
				.unlocked_ioctl = my_ioctl
				};

static int __init adc_init(void)
{
	printk(KERN_INFO "8-Channel ADC Device Registration\n\n");

	//Step-1 : Reserve Major ans Minor Numbers for ADC8
	if(alloc_chrdev_region(&dd_ff, 0, 1, "adc8") < 0)
	{
		return -1;
	}
	printk(KERN_INFO "<Major , Minor>: <%d, %d>\n", MAJOR(dd_ff), MINOR(dd_ff));

	//Step-2 : Creation of ADC8 device file
	if((cluss = class_create(THIS_MODULE, "chardev")) == NULL)
	{
		unregister_chrdev_region(dd_ff,1);
		return -1;
	}
	
	if(device_create(cluss, NULL, dd_ff, NULL, "adc8") == NULL)
	{
		class_destroy(cluss);
		unregister_chrdev_region(dd_ff,1);
		return -1;
	}

	//Step-3 : Link fops and cdev to the device node
	cdev_init(&c_dev, &fops);
	if(cdev_add(&c_dev, dd_ff, 1) == -1)
	{
		device_destroy(cluss, dd_ff);
		class_destroy(cluss);
		unregister_chrdev_region(dd_ff,1);
		return -1;
	}

	return 0;
}

static void __exit adc_exit(void)
{
	cdev_del(&c_dev);
	device_destroy(cluss, dd_ff);
	class_destroy(cluss);
	unregister_chrdev_region(dd_ff, 1);
	printk(KERN_INFO "8-Channel ADC Device Unregistrated");	
}

module_init(adc_init);
module_exit(adc_exit);

MODULE_DESCRIPTION("8-Channel ADC");
MODULE_AUTHOR("Vineet Tiwari");
MODULE_LICENSE("General Purpose License");


