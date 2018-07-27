#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sched.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <pthread.h>
#define SENSOR "/dev/pulse"
#define SPI_DEVICE "/dev/SPIMAT"
#define TRIG 2
#define ECHO 3


pthread_mutex_t mutex;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
int i; 


uint8_t animation_sequence[16];


uint8_t animation_sequence1[16] = {0,250,1,250,2,250,1,250,0,250,1,250,3,250,0,0};
uint8_t animation_sequence2[16] = {4,250,5,250,6,250,4,250,5,250,6,250,4,250,0,0};
uint8_t animation_sequence3[16] = {7,250,8,250,9,250,8,250,7,250,8,250,9,250,0,0};
	
uint8_t patarray[10][24]= {
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x00, 0x02, 0x24, 0x03, 0x00, 0x04, 0x04, 0x05, 0x00, 0x06, 0xc0, 0x07, 0x94, 0x08, 0xc0,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x00, 0x02, 0x24, 0x03, 0x00, 0x04, 0x04, 0x05, 0x00, 0x06, 0x0a, 0x07, 0x0e, 0x08, 0x00,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x0c, 0x02, 0x24, 0x03, 0x0c, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08, 0x00,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x60, 0x02, 0x20, 0x03, 0x60, 0x04, 0x02, 0x05, 0x04, 0x06, 0x28, 0x07, 0x10, 0x08, 0x00,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x18, 0x02, 0x3c, 0x03, 0x7e, 0x04, 0x3c, 0x05, 0x7e, 0x06, 0xff, 0x07, 0x18, 0x08, 0x18,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x98, 0x02, 0x3c, 0x03, 0x7e, 0x04, 0x3c, 0x05, 0x7e, 0x06, 0xff, 0x07, 0x18, 0x08, 0x18,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x19, 0x02, 0x3c, 0x03, 0x7e, 0x04, 0x3c, 0x05, 0x7e, 0x06, 0xff, 0x07, 0x18, 0x08, 0x18,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x00, 0x02, 0x07, 0x03, 0x02, 0x04, 0x7e, 0x05, 0x66, 0x06, 0x55, 0x07, 0x55, 0x08, 0x66,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x00, 0x02, 0x07, 0x03, 0x02, 0x04, 0x7e, 0x05, 0x66, 0x06, 0x99, 0x07, 0x99, 0x08, 0x66,},
	{0x0C, 0x01, 0x09, 0x00, 0x0A, 0x0F, 0x0B, 0x07, 0x01, 0x00, 0x02, 0x07, 0x03, 0x02, 0x04, 0x7e, 0x05, 0x66, 0x06, 0xaa, 0x07, 0xaa, 0x08, 0x66,},	
};

int write_pulse(int fd)
{
    int ret=0;
    char* buf;
    
    buf = (char *)malloc(1);
    while(1)
    {
        ret = write(fd, &buf,sizeof(buf));
        if(ret < 0)
        {
            printf("Trigger Failure\n");    
        }
        else
        {
            printf("Trigger Successful\n");
            break;
        }
        usleep(100000);
    }
    free(buf);
    return ret;
}


int read_pulse(int fd)
{
    int ret=0;
    unsigned int rbuf =0;
    while(1)
    {
        ret = read(fd, &rbuf, sizeof(rbuf));
        
        if(ret < 0)
        {
            printf("Read Failure\n");
        }
        else
        {
            printf("Read Successful\n");
            break;
        }
        usleep(100000);
    }
    return rbuf;
}

void * display_function()
{		
	int fd,fd1,retw;
	fd = open(SPI_DEVICE, O_RDWR);
	if(fd<0)
	printf("opening error");
	fd1=ioctl(fd,1,patarray);
   
	if(fd1<0){
		//errno = EINVAL;
		printf("error in ioctl\n");
	} 

    while(1)
    {
    	pthread_mutex_lock(&lock1);
 
		retw=write(fd,(void*)animation_sequence , sizeof(animation_sequence));
		if(retw<0){
			//errno = EINVAL;
			printf("error in reading\n");
		}
		pthread_mutex_unlock(&lock1);
	}
	close(fd);
	return 0;
}



void * distance_meas()
{
    int fd1,distance;
	fd1=open(SENSOR,O_RDWR);

    if(fd1<0)
    {
        printf("unable to open US driver\n");
    }

    while(1)
    {
        write_pulse(fd1); 
        distance=read_pulse(fd1);
        pthread_mutex_lock(&lock);
        distance = distance * 0.017;
        if(distance>0 && distance<150 )
		{	printf("distance is %d\n",distance );
			if(distance<25)
			{
				for(i=0;i<16;i++)
				{
					animation_sequence[i] = animation_sequence1[i];
				}
			}
			else if(distance>=25 && distance<=50)
			{
				for(i=0;i<16;i++)
				{
					animation_sequence[i] = animation_sequence2[i];
				}
			}
			else if(distance>50)
			{
				for(i=0;i<16;i++)
				{
					animation_sequence[i] = animation_sequence3[i];
				}
			}
		}
			pthread_mutex_unlock(&lock);	
			usleep(50000);
	}
    return 0;
}


// Thread function to control LED matrix
//=======================================

int main()
{   
	pthread_t distance_tid;
	pthread_t display_tid;
	
    pthread_mutex_init(&mutex, NULL);
	pthread_create (&distance_tid, NULL, &distance_meas, NULL);
	pthread_create(&display_tid, NULL, &display_function, NULL);

   	pthread_join(distance_tid, NULL);
    pthread_join(display_tid, NULL);
	return 0;
}


