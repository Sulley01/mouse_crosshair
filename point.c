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
#include "point.h"
#include "framebuffer.h"

point create_point(int x, int y, int c1, int c2, int c3){
	point p;
	p.x = x;
	p.y = y;
	p.c1 = c1;
	p.c2 = c2;
	p.c3 = c3;

	return p;
}

void draw_point(point p, framebuffer f){
	long int location = (p.x+f.vinfo.xoffset) * (f.vinfo.bits_per_pixel/8) + 
            (p.y+f.vinfo.yoffset) * f.finfo.line_length;
    *(f.fbp + location) = p.c1;
    *(f.fbp + location + 1) = p.c2;
    *(f.fbp + location + 2) = p.c3;
}