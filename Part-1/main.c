#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>
#include <poll.h>


#define SPI_DEVICE_PATH "/dev/spidev1.0"

typedef unsigned long long ticks;
int fd76,fd77,fd34,fd16,fd13,fd13v,fd64,fd14,fd14_val,fd14_edge;
int dir=1; // default direction
double dist_measured,dist_measured_p=0;
pthread_mutex_t lock; // mutex for locking critical section
long delay=1000000;
int fd_spi;
uint8_t tr_array[2];
int fd5,fd15,fd7,fd24,fd42,fd30,fd25,fd43,fd31,fd44,fd46;

uint8_t array [] = {
    0x0F, 0x00,
    0x0C, 0x01,
    0x09, 0x00,
    0x0A, 0x0F,
    0x0B, 0x07,
    0x01, 0x06,
    0x02, 0x06,
    0x03, 0x06,
    0x04, 0x83,
    0x05, 0x7F,
    0x06, 0x24,
    0x07, 0x22,
    0x08, 0x63,
};
uint8_t arrayrr [] = {
	0x0F, 0x00,
	0x0C, 0x01,
	0x09, 0x00,
	0x0A, 0x0F,
	0x0B, 0x07,
	0x01, 0x06,
	0x02, 0x06,
	0x03, 0x06,
	0x04, 0x03,
	0x05, 0x7F,
	0x06, 0xA2,
	0x07, 0x32,
	0x08, 0x16,
};
uint8_t arrayl [] = {
	0x0F, 0x00,
	0x0C, 0x01,
	0x09, 0x00,
	0x0A, 0x0F,
	0x0B, 0x07,
	0x01, 0x60,
	0x02, 0x60,
	0x03, 0x60,
	0x04, 0xC1,
	0x05, 0xFE,
	0x06, 0x24,
	0x07, 0x44,
	0x08, 0xC6,
};
uint8_t arraylr [] = {
	0x0F, 0x00,
	0x0C, 0x01,
	0x09, 0x00,
	0x0A, 0x0F,
	0x0B, 0x07,
	0x01, 0x60,
	0x02, 0x60,
	0x03, 0x60,
	0x04, 0xC0,
	0x05, 0xFE,
	0x06, 0x45,
	0x07, 0x4C,
	0x08, 0x68,
};
/* timestamp is used for marking the time at which the edge is detected*/
static inline ticks timestamp(void){
      unsigned a, d;
      asm("cpuid");
      asm volatile("rdtsc" : "=a" (a), "=d" (d));
      return (((ticks)a) | (((ticks)d) << 32));
 }
 /* export function is used for exporting gpio pins associated with ultrasonic sensor*/
void export(){
	int fdexport;
	fdexport = open("/sys/class/gpio/export", O_WRONLY);
	if (fdexport < 0)
	{
		printf("\n gpio export failed while opening");
	}

	if(0> write(fdexport,"77",2))
		printf("error fdexport 77 \n");
	if(0> write(fdexport,"76",2))
		printf("error fdexport 76 \n");
	if(0> write(fdexport,"13",2))
		printf("error fdexport 13 \n");
	if(0> write(fdexport,"14",2))
		printf("error fdexport 14 \n");
	if(0> write(fdexport,"34",2))
		printf("error fdexport 34 \n");
	if(0> write(fdexport,"16",2))
		printf("error fdexport 16 \n");
	if(0> write(fdexport,"64",2))
		printf("error fdexport 64 \n");

	close(fdexport);

}
/* export function is used for unexporting gpio pins associated with ultrasonic sensor*/
void unexport(){
	int fdunexport;
	fdunexport = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fdunexport < 0)
	{
		printf("\n gpio unexport failed while opening");
	}

	if(0> write(fdunexport,"77",2))
		printf("error in fdunexport 77 while writing \n");
	if(0> write(fdunexport,"76",2))
		printf("error in fdunexport 76 while writing \n");
	if(0> write(fdunexport,"13",2))
		printf("error in fdunexport 13 while writing \n");
	if(0> write(fdunexport,"14",2))
		printf("error in fdunexport 14 while writing \n");
	if(0> write(fdunexport,"34",2))
		printf("error in fdunexport 34 while writing \n");
	if(0> write(fdunexport,"16",2))
		printf("error in fdunexport 16 while writing \n");
	if(0> write(fdunexport,"64",2))
		printf("error in fdunexport 64 while writing \n");

	close(fdunexport);

}
/* set_direction function is used for setting direction for gpio pins associated with ultrasonic sensor*/
void set_direction(){

	fd34 = open("/sys/class/gpio/gpio34/direction", O_WRONLY);
	if (fd34 < 0)
	{
		printf("\n gpio34 direction file could not be opened");
	}
	fd16 = open("/sys/class/gpio/gpio16/direction", O_WRONLY);
	if (fd16 < 0)
	{
		printf("\n gpio16 direction file could not be opened");
	}
	fd13 = open("/sys/class/gpio/gpio13/direction", O_WRONLY);
	if (fd13 < 0)
	{
		printf("\n gpio13 direction file could not be opened");
	}
	fd14 = open("/sys/class/gpio/gpio14/direction", O_WRONLY);
	if (fd14 < 0)
	{
		printf("\n gpio14 direction file could not be opened");
	}


	if(0> write(fd13,"out",3))
		printf(" \ndirection writing error - fd13");

	if(0> write(fd14,"out",3))
		printf(" \ndirection writing error - fd14");

	if(0> write(fd34,"out",3))
		printf(" \ndirection writing error - fd34");

	if(0> write(fd16,"out",3))
		printf(" \ndirection writing error - fd16");

}
/* set_value function is used for setting the value gpio pins associated with ultrasonic sensor*/
void set_value(){

	    fd77 = open("/sys/class/gpio/gpio77/value", O_WRONLY);
		if (fd77 < 0)
		{
			printf("\n gpio77 value file could not be opened");
		}
		fd76 = open("/sys/class/gpio/gpio76/value", O_WRONLY);
        if (fd76 < 0)
		{
			printf("\n gpio76 value file could not be opened");
		}
		fd34 = open("/sys/class/gpio/gpio34/value", O_WRONLY);
		if (fd34 < 0)
		{
			printf("\n gpio34 value file could not be opened");
		}
		fd16 = open("/sys/class/gpio/gpio16/value", O_WRONLY);
        if (fd16 < 0)
		{
			printf("\n gpio16 value file could not be opened");
		}
		fd13v = open("/sys/class/gpio/gpio13/value", O_WRONLY);
        if (fd13v<0)
		{
			printf("\n gpio13 value file could not be opened");
        }

		fd14_val = open("/sys/class/gpio/gpio14/value", O_RDWR);
		if (fd14_val< 0)
		{
			printf("\n gpio14 value file could not be opened");
		}
		fd64 = open("/sys/class/gpio/gpio64/value", O_WRONLY);
        if (fd64 < 0)
		{
			printf("\n gpio64 value file could not be opened");
		}



		if(0> write(fd77,"0",1))
			printf("\nerror in writing value for pin 77 ");
		if(0> write(fd76,"0",1))
			printf("\nerror in writing value for pin 76 ");
		if(0> write(fd34,"0",1))
			printf("\nerror in writing value for pin 34 ");
		if(0> write(fd16,"1",1))
			printf("\nerror in writing value for pin 16 ");
		if(0> write(fd13v,"0",1))
			printf("\nerror in writing value for pin 13 ");
		if(0> write(fd14_val,"0",1))
			printf("\nerror in writing value for pin 14 ");
		if(0> write(fd64,"0",1))
			printf("\nerror in writing value for pin 64 ");

}
/* export function is used for exporting gpio pins associated with spi device*/
void spiexport(){
    int fdexport;
    fdexport = open("/sys/class/gpio/export", O_WRONLY);
    if (fdexport < 0)
    {
        printf("\n gpio export failed while opening");
    }

    if(0> write(fdexport,"5",2))
        printf("error fdexport 5 \n");
    if(0> write(fdexport,"15",2))
        printf("error fdexport 15 \n");
    if(0> write(fdexport,"7",2))
        printf("error fdexport 7 \n");
    if(0> write(fdexport,"24",2))
        printf("error fdexport 24 \n");
    if(0> write(fdexport,"42",2))
        printf("error fdexport 42 \n");
    if(0> write(fdexport,"30",2))
        printf("error fdexport 30 \n");
    if(0> write(fdexport,"25",2))
        printf("error fdexport 25 \n");
    if(0> write(fdexport,"43",2))
        printf("error fdexport 43 \n");
    if(0> write(fdexport,"31",2))
        printf("error fdexport 31 \n");
    if(0> write(fdexport,"44",2))
        printf("error fdexport 44 \n");
    if(0> write(fdexport,"46",2))
        printf("error fdexport 46 \n");

        close(fdexport);

}
/* unexport function is used for unexporting gpio pins associated with spi device*/
void spiunexport(){
    int fdunexport;
    fdunexport = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fdunexport < 0)
    {
        printf("\n gpio unexport failed while opening");
    }

    if(0> write(fdunexport,"5",2))
        printf("error in fdunexport 5  while writing\n");
    if(0> write(fdunexport,"15",2))
        printf("error in fdunexport 15 while writing \n");
    if(0> write(fdunexport,"7",2))
        printf("error in fdunexport 7  while writing\n");
    if(0> write(fdunexport,"24",2))
        printf("error in fdunexport 24 while writing \n");
    if(0> write(fdunexport,"42",2))
        printf("error in fdunexport 42 while writing \n");
    if(0> write(fdunexport,"30",2))
        printf("error in fdunexport 30 while writing \n");
    if(0> write(fdunexport,"25",2))
        printf("error in fdunexport 25 while writing \n");
    if(0> write(fdunexport,"43",2))
        printf("error in fdunexport 43 while writing \n");
    if(0> write(fdunexport,"31",2))
        printf("error in fdunexport 31 while writing \n");
    if(0> write(fdunexport,"44",2))
        printf("error in fdunexport 44 while writing \n");
    if(0> write(fdunexport,"46",2))
        printf("error in fdunexport 46 while writing \n");

    close(fdunexport);

}
/* set direction function is used for setting the direction gpio pins associated with spi device*/
void spi_set_direction(){

    if (fd5 < 0)
    {
        printf("\n gpio5 direction file could not be opened");
    }

    fd15 = open("/sys/class/gpio/gpio15/direction", O_WRONLY);
    if (fd15 < 0)
    {
        printf("\n gpio15 direction file could not be opened");
    }
    fd7 = open("/sys/class/gpio/gpio7/direction", O_WRONLY);
    if (fd7 < 0)
    {
        printf("\n gpio7 direction file could not be opened");
    }

    fd24 = open("/sys/class/gpio/gpio24/direction", O_WRONLY);
    if (fd24 < 0)
    {
        printf("\n gpio24 direction file could not be opened");
    }
    fd42 = open("/sys/class/gpio/gpio42/direction", O_WRONLY);
    if (fd42 < 0)
    {
        printf("\n gpio42 direction file could not be opened");
    }
    fd30 = open("/sys/class/gpio/gpio30/direction", O_WRONLY);
    if (fd30 < 0)
    {
        printf("\n gpio24 direction file could not be opened");
    }
    fd25 = open("/sys/class/gpio/gpio25/direction", O_WRONLY);
    if (fd25 < 0)
    {
        printf("\n gpio25 direction file could not be opened");
    }
    fd43 = open("/sys/class/gpio/gpio43/direction", O_WRONLY);
    if (fd43 < 0)
    {
        printf("\n gpio43 direction file could not be opened");
    }
    fd31 = open("/sys/class/gpio/gpio31/direction", O_WRONLY);
    if (fd31 < 0)
    {
        printf("\n gpio31 direction file could not be opened");
    }
    fd44 = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
    if (fd44 < 0)
    {
        printf("\n gpio44 direction file could not be opened");
    }
    fd46 = open("/sys/class/gpio/gpio46/direction", O_WRONLY);
    if (fd46 < 0)
    {
        printf("\n gpio46 direction file could not be opened");
    }


    if(0> write(fd5,"out",3))
        printf("\nerror while writing out for pin 5");
    if(0> write(fd15,"out",3))
        printf("\nerror while writing out for pin 15");
    if(0> write(fd7,"out",3))
        printf("\nerror while writing out for pin 7");
    if(0> write(fd24,"out",3))
        printf("\nerror while writing out for pin 24");
    if(0> write(fd42,"out",3))
        printf("\nerror while writing out for pin 42");
    if(0> write(fd30,"out",3))
        printf("\nerror while writing out for pin 30");
    if(0> write(fd25,"out",3))
        printf("\nerror while writing out for pin 25");
    if(0> write(fd43,"out",3))
        printf("\nerror while writing out for pin 43");
    if(0> write(fd31,"out",3))
        printf("\nerror while writing out for pin 31");
    if(0> write(fd44,"out",3))
        printf("\nerror while writing out for pin 44");
    if(0> write(fd46,"out",3))
        printf("\nerror while writing out for pin 46");


    close(fd5);
    close(fd15);
    close(fd7);
    close(fd24);
    close(fd42);
    close(fd30);
    close(fd25);
    close(fd43);
    close(fd31);
    close(fd44);
    close(fd46);

}
/* set value function is used for setting the value for gpio pins associated with spi device*/
void spi_set_value(){

    fd5 = open("/sys/class/gpio/gpio5/value", O_WRONLY);
    if (fd5 < 0)
            {
                printf("\n gpio5 value file could not be opened");
            }
    fd15 = open("/sys/class/gpio/gpio15/value", O_WRONLY);
            if (fd15 < 0)
            {
                printf("\n gpio15 value file could not be opened");
            }
    fd7 = open("/sys/class/gpio/gpio7/value", O_WRONLY);
    if (fd7 < 0)
            {
                printf("\n gpio7 value file could not be opened");
            }
    fd24 = open("/sys/class/gpio/gpio24/value", O_WRONLY);
            if (fd24 < 0)
            {
                printf("\n gpio24 value file could not be opened");
            }
    fd42 = open("/sys/class/gpio/gpio42/value", O_WRONLY);
        if (fd42<0)
            {
                printf("\n gpio42 value file could not be opened");
                }

    fd30 = open("/sys/class/gpio/gpio30/value", O_WRONLY);
    if (fd30< 0)
            {
                printf("\n gpio30 value file could not be opened");
            }
    fd25 = open("/sys/class/gpio/gpio25/value", O_WRONLY);
    if (fd25 < 0)
            {
                printf("\n gpio25 value file could not be opened");
            }
    fd43 = open("/sys/class/gpio/gpio43/value", O_WRONLY);
            if (fd43 < 0)
            {
                printf("\n gpio43 value file could not be opened");
            }
    fd31 = open("/sys/class/gpio/gpio31/value", O_WRONLY);
    if (fd31 < 0)
            {
                printf("\n gpio31 value file could not be opened");
            }
    fd44 = open("/sys/class/gpio/gpio44/value", O_WRONLY);
            if (fd44 < 0)
            {
                printf("\n gpio44 value file could not be opened");
            }
    fd46 = open("/sys/class/gpio/gpio46/value", O_WRONLY);
        if (fd46<0)
            {
                printf("\n gpio46 value file could not be opened");
                }



    if(0> write(fd5,"1",1))
        printf("\nerror while writing in pin 5 value");
    if(0> write(fd15,"0",1))
        printf("\nerror while writing in pin 15 value");
    if(0> write(fd7,"0",1))
        printf("\nerror while writing in pin 7 value");
    if(0> write(fd24,"0",1))
        printf("\nerror while writing in pin 24 value");
    if(0> write(fd42,"0",1))
        printf("\nerror while writing in pin 42 value");
    if(0> write(fd30,"0",1))
        printf("\nerror while writing in pin 30 value");
    if(0> write(fd25,"0",1))
        printf("\nerror while writing in pin 25 value");
    if(0> write(fd43,"0",1))
        printf("\nerror while writing in pin 43 value");
    if(0> write(fd31,"0",1))
        printf("\nerror while writing in pin 31 value");
    if(0> write(fd44,"1",1))
        printf("\nerror while writing in pin 44 value");
    if(0> write(fd46,"1",1))
        printf("\nerror while writing in pin 46 value");

}
/* This function is for determining the direction of the direction and the speed of the display*/
void delay_and_dir(double dist ){
    dist_measured =dist;
    double dist_interval= dist_measured - dist_measured_p;
    double distance_min = dist_measured / 10.0;
    printf("Object is %0.3f cm away!!\n",dist_measured);
    if(dist_interval > distance_min){
        dir=3;
        delay=30000;
    }
    else if(dist_interval < (-distance_min)){
        dir=1;
        delay=10000;
    }
    dist_measured_p = dist_measured;
}
/*This is the thread body fuction to determine the distance by using the time delay obtained by the ultrasonic sensor*/
void* US_distance_meas(void* arg){
    int ret;
    int timeout = 2000;
    double ts_rise_e=0; // rising edge time
    double ts_fall_e=0; // falling edge time
    long diff;
    struct pollfd edge_poll;
    char* buff[64];

    /*  Opening trigger and echo pins of the ultrasonic sensor  */
    fd14 = open("/sys/class/gpio/gpio14/direction", O_WRONLY);
    if (fd14 < 0)
	printf("\n gpio14 direction file could not be opened");
    if (0> write(fd14,"in",2))
	printf(" \n in direction writing error fd14");
    fd14_val = open("/sys/class/gpio/gpio14/value", O_RDWR);
    if (fd14_val<0)
	   printf("\n gpio14 value file could not be opened");
	fd14_edge = open("/sys/class/gpio/gpio14/edge", O_WRONLY);
	if (fd14_edge< 0)
		printf("\n gpio14 value file could not be opened");
	fd13v = open("/sys/class/gpio/gpio13/value", O_WRONLY);
	if (fd14_val< 0)
		printf("\n gpio13 value file could not be opened");
	
	edge_poll.fd = fd14_val;
	edge_poll.events = POLLPRI|POLLERR;
	edge_poll.revents=0;

    A:  while(1){
	        ts_fall_e=0;
	        ts_rise_e=0;
			lseek(fd14_val,0,SEEK_SET);
			/* setting the edge to be detected as rising edge */
			write(fd14_edge,"rising",6);
		    /* setting a trigger */
			write(fd13v,"1",1);
			/* trigger pulse duration */
			usleep(14);
			write(fd13v,"0",1);
			/* start polling the rising edge for the returned wave */
			ret=poll(&edge_poll,1,timeout);
	        if(ret>0){
			    if (edge_poll.revents & POLLPRI){
				/* record the time */
				     ts_rise_e = timestamp();
				     read(fd14_val,buff,1);
			    }
	            else{
	                printf("Error in detecting Rising Edge\n");
	                goto A;
	            }

			}		
	 		lseek(fd14_val,0,SEEK_SET);
	 	      /* poll for the falling edge */
			write(fd14_edge,"falling",7);
			ret = poll(&edge_poll,1,timeout);
	        if(ret>0){
			if (edge_poll.revents & POLLPRI){
	        /* record the time when falling edge is detected*/
				ts_fall_e= timestamp();
				read(fd14_val,buff,1);
	        }
	        else{
	            printf("Error in detecting Falling Edge\n");
	            goto A;
	        }
		}
    printf("\n timestamp of falling edge  %lf",ts_fall_e );
    printf("\n timestamp of rising edge  %lf",ts_rise_e );
    pthread_mutex_lock(&lock);
    diff = (long)(ts_fall_e-ts_rise_e);
    printf("\n the difference in no. of ticks is %li",diff );
    double time2dist =(diff*340.00)/(4000000.00*2.00);
    printf("\n distance measured in cm is %lf", time2dist);
    delay_and_dir(time2dist);
    pthread_mutex_unlock(&lock);
    usleep(500000);
    }
    return 0;
}
/*This is the thread body fuction to set the animation in the spi LED matrix display*/
void* display_function(void* arg){
    int  i = 0;
    int ret;
    struct spi_ioc_transfer spi_msg =
    {
        .tx_buf = (unsigned long)tr_array,
        .rx_buf = 0,
        .len = 2,
        .delay_usecs = 1,
        .speed_hz = 10000000,
        .bits_per_word = 8,
        .cs_change = 1,
    };
    fd_spi= open(SPI_DEVICE_PATH,O_WRONLY);
    if(fd_spi==-1)
    {
        printf("file %s either does not exit or is currently used by an another user\n", SPI_DEVICE_PATH);
        exit(-1);
    }
    while(1){
        i=0;
        while (i < 26){
       		/* changing the animation according to the direction set by the ultra sonic sensor*/
	        if(dir ==1){
	            tr_array[0] = array [i];
	            tr_array[1] = array [i+1];
	        }
	        else if(dir==2){
	            tr_array[0] = arrayrr [i];
	            tr_array[1] = arrayrr [i+1];
	        }
	        else if(dir==3){
	            tr_array[0] = arrayl [i];
	            tr_array[1] = arrayl [i+1];
	        }
	        else if(dir==4){
	            tr_array[0] = arraylr [i];
	            tr_array[1] = arraylr [i+1];
	        }
	        // setting the chip select pin to low to enable the communication with spi device
	        if(0>write(fd15,"0",1))
	                printf("\nerror Fd15 value");
	        ret = ioctl(fd_spi, SPI_IOC_MESSAGE (1), &spi_msg);
	        if(ret<0)
	            printf("\n error");
	        usleep(10000);
	        // disabling the communication with spi device by setting the chip select pin to high
	        if(0> write(fd15,"1",1))
	            printf("\nerror Fd15 value");
	        i = i + 2;
        }
        usleep(delay);

        // changing the animation according to the direction
        if(dir==3)
            dir=4;
        else if(dir==4)
            dir=3;
        if(dir==1)
            dir=2;
        else if(dir==2)
            dir=1;

    }
    ret = ioctl(fd_spi, SPI_IOC_MESSAGE (1), &spi_msg);
    if(ret<0)
        printf("\n error occured while ioctl transaction");
    close (fd_spi);
    return 0;
}

int main(){
    int spi_ret;
    // initializing the gpio pins for ultrasonic sensor and spi display device
    unexport();
    spiunexport();
    export();
    set_direction();
    set_value();
    spiexport();
    spi_set_direction();
    spi_set_value();
    pthread_t distance_thread;
    pthread_t spidisplay_thread;
	pthread_create (&distance_thread, NULL, &US_distance_meas, NULL);
    spi_ret = pthread_create(&spidisplay_thread, NULL, &display_function, NULL);
    if (spi_ret != 0)
        printf("\ncould not create the thread for display in spi device\n");
	pthread_join(distance_thread, NULL);
    pthread_join (spidisplay_thread, NULL);
    return 0;
}

/*
References:
Polling function-  http://man7.org/linux/man-pages/man2/poll.2.html
ioctl- https://elixir.free-electrons.com/linux/v3.5/source/arch/alpha/include/asm/ioctl.h#L39
spi_ioc_transfer- https://elixir.free-electrons.com/linux/v3.5/ident/spi_ioc_transfer
*/
