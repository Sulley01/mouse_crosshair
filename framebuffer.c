#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include "framebuffer.h"

framebuffer init(){
	framebuffer f;
	// Open the file for reading and writing
    f.fbfd = open("/dev/fb0", O_RDWR);
    if (f.fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(f.fbfd, FBIOGET_FSCREENINFO, &f.finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(f.fbfd, FBIOGET_VSCREENINFO, &f.vinfo) == -1) {
     	perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", f.vinfo.xres, f.vinfo.yres, f.vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    f.screensize = f.vinfo.xres * f.vinfo.yres * f.vinfo.bits_per_pixel / 8;

    // Map the device to memory
    f.fbp = (char *)mmap(0, f.screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                        f.fbfd, 0);
    if ((long)f.fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }

    printf("The framebuffer device was mapped to memory successfully.\n");

    return f;
}