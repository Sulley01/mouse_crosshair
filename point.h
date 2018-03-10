#ifndef POINT_H
#define POINT_H

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

typedef struct point{
	int x;
	int y;
	int c1;
	int c2;
	int c3;
}point;

point create_point(int x, int y, int c1, int c2, int c3);

void draw_point(point p, framebuffer f);

#endif