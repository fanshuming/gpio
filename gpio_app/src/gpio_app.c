#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


#define MYLEDS_LED1_ON 	0
#define MYLEDS_LED1_OFF  1



void print_usage(char *file)
{
    printf("Usage:\n");
    printf("eg. \n");
    printf("%s led on\n", file);
    printf("%s led off\n", file);
}

int main(int argc , char** argv)
{
	int fd;

	if(argc != 3)
	{
		print_usage(argv[0]);
		return 0;
	}
	
	fd = open("/dev/gpio_drv" , O_RDWR|O_NONBLOCK);
	if(fd < 0)
	{
		printf("can't open /dev/gpio_drv\n");
		return -1;
	}

	if(!strcmp("led",argv[1]))
	{
		if(!strcmp("on", argv[2]))
		{
			ioctl(fd,MYLEDS_LED1_ON);
		}
		else if(!strcmp("off", argv[2]))
		{
			ioctl(fd,MYLEDS_LED1_OFF);
		}
		else
		{
			print_usage(argv[0]);
			return 0;
		}
	}
	else
	{
	 	print_usage(argv[0]);
		return 0;
	}
	return 0;
}

