#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

#define MYLEDS_LED1_ON  0
#define MYLEDS_LED1_OFF 1


volatile unsigned long *GPIO40_MODE;
volatile unsigned long *GPIO40_DATA;
volatile unsigned long *GPIO40_DIR;
volatile unsigned long *GPIO40_SETDATA;
volatile unsigned long *GPIO40_RESETDATA;


/****************  基本定义 **********************/
//内核空间缓冲区定义
#if 0
	#define KB_MAX_SIZE 20
	#define kbuf[KB_MAX_SIZE];
#endif


//加密函数参数内容： _IOW(IOW_CHAR , IOW_NUMn , IOW_TYPE)
//加密函数用于gpio_drv_ioctl函数中
//使用举例：ioctl(fd , _IOW('L',0x80,long) , 0x1);
//#define NUMn gpio_drv , if you need!
#define IOW_CHAR 'L'
#define IOW_TYPE  long
#define IOW_NUM1  0x80


//初始化函数必要资源定义
//用于初始化函数当中
//device number;
	dev_t dev_num;
//struct dev
	struct cdev gpio_drv_cdev;
//auto "mknode /dev/gpio_drv c dev_num minor_num"
struct class *gpio_drv_class = NULL;
struct device *gpio_drv_device = NULL;


/**************** 结构体 file_operations 成员函数 *****************/
//open
static int gpio_drv_open(struct inode *inode, struct file *file)
{
	printk("gpio_drv drive open...\n");

	*GPIO40_RESETDATA |=(1u<<8);

	return 0;
}

//close
static int gpio_drv_close(struct inode *inode , struct file *file)
{
	printk("gpio_drv drive close...\n");


	return 0;
}

//read
static ssize_t gpio_drv_read(struct file *file, char __user *buffer,
			size_t len, loff_t *pos)
{
	int ret_v = 0;
	ret_v = *GPIO40_SETDATA & (1u<<8);
	printk("gpio_drv drive read:%d\n",ret_v);
	
	copy_to_user(buffer, &ret_v, 4);

	return 4;
}

//write
static ssize_t gpio_drv_write( struct file *file , const char __user *buffer,
			   size_t len , loff_t *offset )
{
	int ret_v = 0;

	copy_from_user(&ret_v, buffer, 4);

	printk("gpio_drv drive write:%d\n",ret_v);

	if(ret_v == 0)
	{
		*GPIO40_DATA &= ~(1<<8);
	}else{
		*GPIO40_DATA |=  (1<<8);
	}

	return 4;
}

//unlocked_ioctl
static int gpio_drv_ioctl (struct file *filp , unsigned int cmd , unsigned long arg)
{
	int ret_v = 0;
	printk("gpio_drv drive ioctl...\n");

	switch(cmd)
	{
		//常规：
		//cmd值自行进行修改
		 case MYLEDS_LED1_ON:
		      *GPIO40_RESETDATA |= (1u<<8);
		 break;

		 case MYLEDS_LED1_OFF:
             *GPIO40_SETDATA |= (1u<<8);
		 break;
		//带密码保护：
		//请在"基本定义"进行必要的定义
		case _IOW(IOW_CHAR,IOW_NUM1,IOW_TYPE):
		{
			if(arg == 0x1) //第二条件
			{
				
			}

		}
		break;

		default:
			break;
	}

	return ret_v;
}


/***************** 结构体： file_operations ************************/
//struct
static const struct file_operations gpio_drv_fops = {
	.owner   = THIS_MODULE,
	.open	 = gpio_drv_open,
	.release = gpio_drv_close,	
	.read	 = gpio_drv_read,
	.write   = gpio_drv_write,
	.unlocked_ioctl	= gpio_drv_ioctl,
};


/*************  functions: init , exit*******************/
//条件值变量，用于指示资源是否正常使用
unsigned char init_flag = 0;
unsigned char add_code_flag = 0;

//init
static __init int gpio_drv_init(void)
{
	int ret_v = 0;
	printk("gpio_drv drive init...\n");

	//函数alloc_chrdev_region主要参数说明：
	//参数2： 次设备号
	//参数3： 创建多少个设备
	if( ( ret_v = alloc_chrdev_region(&dev_num,0,1,"gpio_drv") ) < 0 )
	{
		goto dev_reg_error;
	}
	init_flag = 1; //标示设备创建成功；

	printk("The drive info of gpio_drv:\nmajor: %d\nminor: %d\n",
		MAJOR(dev_num),MINOR(dev_num));

	cdev_init(&gpio_drv_cdev,&gpio_drv_fops);
	if( (ret_v = cdev_add(&gpio_drv_cdev,dev_num,1)) != 0 )
	{
		goto cdev_add_error;
	}

	gpio_drv_class = class_create(THIS_MODULE,"gpio_drv");
	if( IS_ERR(gpio_drv_class) )
	{
		goto class_c_error;
	}

	gpio_drv_device = device_create(gpio_drv_class,NULL,dev_num,NULL,"gpio_drv");
	if( IS_ERR(gpio_drv_device) )
	{
		goto device_c_error;
	}
	printk("auto mknod success!\n");

	//------------   请在此添加您的初始化程序  --------------//
       
        GPIO40_MODE = (volatile unsigned long *)ioremap(0x10000064, 4);
        GPIO40_DATA = (volatile unsigned long *)ioremap(0x10000624, 4);
        GPIO40_DIR =  (volatile unsigned long *)ioremap(0x10000604, 4);

    	*GPIO40_MODE |= (1u<<8);
    	*GPIO40_DIR  |=  (1u<<8);

	GPIO40_SETDATA=(volatile unsigned long *)ioremap(0x10000634,4);
    	GPIO40_RESETDATA=(volatile unsigned long *)ioremap(0x10000644,4);
 	

        //如果需要做错误处理，请：goto gpio_drv_error;	

	 add_code_flag = 1;
	//----------------------  END  ---------------------------// 

	goto init_success;

dev_reg_error:
	printk("alloc_chrdev_region failed\n");	
	return ret_v;

cdev_add_error:
	printk("cdev_add failed\n");
 	unregister_chrdev_region(dev_num, 1);
	init_flag = 0;
	return ret_v;

class_c_error:
	printk("class_create failed\n");
	cdev_del(&gpio_drv_cdev);
 	unregister_chrdev_region(dev_num, 1);
	init_flag = 0;
	return PTR_ERR(gpio_drv_class);

device_c_error:
	printk("device_create failed\n");
	cdev_del(&gpio_drv_cdev);
 	unregister_chrdev_region(dev_num, 1);
	class_destroy(gpio_drv_class);
	init_flag = 0;
	return PTR_ERR(gpio_drv_device);

//------------------ 请在此添加您的错误处理内容 ----------------//
gpio_drv_error:
	add_code_flag = 0;
	return -1;
//--------------------          END         -------------------//
    
init_success:
	printk("gpio_drv init success!\n");
	return 0;
}

//exit
static __exit void gpio_drv_exit(void)
{
	printk("gpio_drv drive exit...\n");	

	if(add_code_flag == 1)
 	{   
           //----------   请在这里释放您的程序占有的资源   ---------//
	    printk("free your resources...\n");	               

	 	 //iounmap(GPIOMODE);
		 iounmap(GPIO40_DIR);
		 iounmap(GPIO40_SETDATA);
		 iounmap(GPIO40_RESETDATA);

	    printk("free finish\n");		               
	    //----------------------     END      -------------------//
	}					            

	if(init_flag == 1)
	{
		//释放初始化使用到的资源;
		cdev_del(&gpio_drv_cdev);
 		unregister_chrdev_region(dev_num, 1);
		device_unregister(gpio_drv_device);
		class_destroy(gpio_drv_class);
	}
}


/**************** module operations**********************/
//module loading
module_init(gpio_drv_init);
module_exit(gpio_drv_exit);

//some infomation
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("from fanshuming");
MODULE_DESCRIPTION("gpio_drv drive");



/*********************  The End ***************************/
