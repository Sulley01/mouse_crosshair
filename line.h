#ifndef LINE_H
#define LINE_H

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

typedef struct line{
	point p1;
	point p2;
}line;

line create_line(point p1, point p2);

void bresenhamLow(line l, framebuffer f);

void bresenhamHigh(line l, framebuffer f);

void draw_line(line l, framebuffer f);

#endif