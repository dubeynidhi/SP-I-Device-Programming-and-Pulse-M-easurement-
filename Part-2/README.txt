REPOSITORY CONTENTS:

-> sensor.c (Driver Program Source code for distance measurement)
-> DispDriver.c (Driver Program Source code for pattern display)
-> Makefile (Makefile for the Sourcecode)
-> README 
-> main.c(test program)

_____________________________________________________________________________________________________________________________
ABOUT: 

This project demonstrates the Distance measurement using an Ultrasonic sensor and displaying patterns according to the distance measured in a spi LED matrix display. Here the distance measurement is taken care by the device driver(sensor) and the display is controlled by the device driver - DispDriver. The animation changes according to the distance of the object from the sensor.
_____________________________________________________________________________________________________________________________

SYSTEM REQUIREMENTS:

-> Ultrasonic sensor
-> spi matrix LED display
-> LINUX KERNEL : Minimum version of 2.6.19 is expected.
-> SDK: iot-devkit-glibc-x86_64-image-full-i586-toolchain-1.7.2
-> GCC: Minimum version of 4.8 is required to run -pthread option while compiling.
-> Intel Galileo Gen2
_____________________________________________________________________________________________________________________________
SETUP:

-> Ultrasonic sensor and display should be connected to the GPIO pins and the GND pin should be grounded properly by connecting to the GND of Galileo board
-> Must boot Galileo from SD card with linux version 3.19.
-> Set up the board using setup guide.
_____________________________________________________________________________________________________________________________
COMPILATION:

-> type sudo screen /dev/ttyUSB0 115200 to communicate with board.
-> type ifconfig enp0s20f6 192.168.1.5 netmask 255.255.0.0 up
-> On the host, open directory in terminal in which files are present and compile library file. Type gcc led_ioctl.h.
-> Type make.
-> Open a new terminal using scp copy your sernor.ko, DispDriver.ko file, mainsp executable file to desired location on board.
-> Now use ssh root@192.168.1.5 and navigate to the folder where the executable file is present.
_____________________________________________________________________________________________________________________________
EXECUTION:

C program-
-> remove the existing spidev module using the rmmod command
->type insmod sensor.ko and next ins mod DispDriver.ko to insert the module
->type ./mainsp to run the code
_____________________________________________________________________________________________________________________________
EXPECTED OUTPUT:

-> On running the executable, you will see the patterns on the spa display.
-> You can bring the object near or move away from the sensor and you can notice the change in the patterns displayed
_____________________________________________________________________________________________________________________________
