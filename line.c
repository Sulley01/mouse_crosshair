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
#include "line.h"

line create_line(point p1, point p2){
	line l;
	l.p1.x = p1.x;
	l.p1.y = p1.y;
	l.p1.c1 = p1.c1;
	l.p1.c2 = p1.c2;
	l.p1.c3 = p1.c3;

	l.p2.x = p2.x;
	l.p2.y = p2.y;
	l.p2.c1 = p2.c1;
	l.p2.c2 = p2.c2;
	l.p2.c3 = p2.c3;

	return l;
}

void bresenhamLow(line l, framebuffer f) {
    int dx = l.p2.x - l.p1.x;
    int dy = l.p2.y - l.p1.y;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = 2*dy - dx;
    int y = l.p1.y;

    point p;

    for (int x = l.p1.x; x <= l.p2.x; x++) {
    	p = create_point(x, y, l.p1.c1, l.p1.c2, l.p1.c3);
        draw_point(p, f);
        if (D > 0) {
            y += yi;
            D -= 2*dx;
        }
        D += 2*dy;
    }
}

void bresenhamHigh(line l, framebuffer f) {
    int dx = l.p2.x - l.p1.x;
    int dy = l.p2.y - l.p1.y;
    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int D = 2*dx - dy;
    int x = l.p1.x;

    point p;

    for (int y = l.p1.y; y <= l.p2.y; y++) {
        p = create_point(x, y, l.p1.c1, l.p1.c2, l.p1.c3);
        draw_point(p, f);
        if (D > 0) {
            x += xi;
            D -= 2*dy;
        }
        D += 2*dx;
    }
}

void draw_line(line l, framebuffer f) {
    if (abs(l.p2.y - l.p1.y) < abs(l.p2.x - l.p1.x)) {
        if (l.p1.x > l.p2.x) {
        	line temp = create_line(l.p2, l.p1);
            bresenhamLow(temp, f);
        }
        else {
            bresenhamLow(l, f);
        }
    }
    else {
        if (l.p1.y > l.p2.y) {
        	line temp = create_line(l.p2, l.p1);
            bresenhamHigh(temp, f);
        }
        else {
            bresenhamHigh(l, f);
        }
    }
}