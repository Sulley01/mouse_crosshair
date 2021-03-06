#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "framebuffer.h"
#include "point.h"
#include "line.h"
#include "polygon.h"

#define RED_STATUS 2
#define NORMAL_STATUS 1
#define GREEN_STATUS 4
#define BLUE_STATUS 3

framebuffer f;
int dimension_x = 1366;
int dimension_y = 768;
int background_red = 255;
int background_green = 255;
int background_blue = 255;
int crosshair_red = 0;
int crosshair_green = 0;
int crosshair_blue = 0;
int crosshair_middle = 5;
int crosshair_radius = 20;
pthread_t threads[2];
int status = NORMAL_STATUS;
void colorBackground() {
    for (int i = 0; i < dimension_x; i++) {
        for (int j = 0; j < dimension_y; j++) {
            draw_point(create_point(i, j, background_blue, background_green, background_red), f);
        }
    }
}

int getRed(int x, int y) {
    int loc = (x+f.vinfo.xoffset) * (f.vinfo.bits_per_pixel/8) + 
            (y+f.vinfo.yoffset) * f.finfo.line_length;
    int c3 = *(f.fbp + loc + 2);
    if (c3 < 0) {
        c3 += 256;
    }
    return c3;
}

int getGreen(int x, int y) {
    int loc = (x+f.vinfo.xoffset) * (f.vinfo.bits_per_pixel/8) + 
            (y+f.vinfo.yoffset) * f.finfo.line_length;
    int c2 = *(f.fbp + loc + 1);
    if (c2 < 0) {
        c2 += 256;
    }
    return c2;
}

int getBlue(int x, int y) {
    int loc = (x+f.vinfo.xoffset) * (f.vinfo.bits_per_pixel/8) + 
            (y+f.vinfo.yoffset) * f.finfo.line_length;
    int c1 = *(f.fbp + loc);
    if (c1 < 0) {
        c1 += 256;
    }
    return c1;
}

void drawCrosshair(point p, int r) {
    line up = create_line(create_point(p.x, p.y-crosshair_middle, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x, p.y-r, crosshair_blue, crosshair_green, crosshair_red));
    line down = create_line(create_point(p.x, p.y+crosshair_middle, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x, p.y+r, crosshair_blue, crosshair_green, crosshair_red));
    line left = create_line(create_point(p.x-crosshair_middle, p.y, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x-r, p.y, crosshair_blue, crosshair_green, crosshair_red));
    line right = create_line(create_point(p.x+crosshair_middle, p.y, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x+r, p.y, crosshair_blue, crosshair_green, crosshair_red));

    draw_line(up, f);
    draw_line(down, f);
    draw_line(left, f);
    draw_line(right, f);
}

void readTempCrosshair(point parr[], point p) {
    int j = 0;
    for (int i = crosshair_middle; i <= crosshair_radius; i++) {
        parr[j] = create_point(p.x, p.y-i, getBlue(p.x, p.y-i), getGreen(p.x, p.y-i), getRed(p.x, p.y-i));
        parr[j+1] = create_point(p.x, p.y+i, getBlue(p.x, p.y+i), getGreen(p.x, p.y+i), getRed(p.x, p.y+i));
        parr[j+2] = create_point(p.x-i, p.y, getBlue(p.x-i, p.y), getGreen(p.x-i, p.y), getRed(p.x-i, p.y));
        parr[j+3] = create_point(p.x+i, p.y, getBlue(p.x+i, p.y), getGreen(p.x+i, p.y), getRed(p.x+i, p.y));
        j += 4;
    }
}

void drawTempCrosshair(point parr[]) {
    for (int i = 0; i < ((crosshair_radius-crosshair_middle)+1)*4; i++) {
        draw_point(parr[i], f);
    }
}

int isCrosshairMoveValid(point c, int dx, int dy) {
    return c.x+dx-crosshair_radius > 0 && c.x+dx+crosshair_radius < dimension_x && 
        c.y-dy-crosshair_radius > 0 && c.y-dy+crosshair_radius < dimension_y;
}


void *changeColor(void *arg) {
    system ("/bin/stty raw");
    char c;
    while (c = getchar()) {
        if (c == 'r') { //for color red
            crosshair_red = 255;
            crosshair_green = 0;
            crosshair_blue = 0;
            status = RED_STATUS;
        }
        else if (c == 'n') { //for back to normal black
            crosshair_red = 0;
            crosshair_green = 0;
            crosshair_blue = 0;
            status = NORMAL_STATUS;    
        } else if (c== 'b') { //for blue
            crosshair_red = 0;
            crosshair_green = 0;
            crosshair_blue = 255;
            status = BLUE_STATUS; 
        } else if (c== 'g') { //for green
            crosshair_red = 0;
            crosshair_green = 255;
            crosshair_blue = 0;
            status = GREEN_STATUS; 
        } else if (c== 1){
            colorBackground();
            drawToScreen(&f);
        }
        else {
            status = NORMAL_STATUS;
            break;
        }
    }
    system ("/bin/stty cooked");
}

void *paint(void *arg) {
    f = init();
    colorBackground();

    drawToScreen(&f);

    // Mouse variables
    int fd, bytes;
    unsigned char data[3];

    const char *pDevice = "/dev/input/mice";

    int left, middle, right;
    int prev = -1;
    signed char dx, dy;

    point crosshair_temp[((crosshair_radius-crosshair_middle)+1)*4];

    // Create crosshair
    point crosshair = create_point(dimension_x/2, (dimension_y)/2, crosshair_blue, crosshair_green, crosshair_red);
    readTempCrosshair(crosshair_temp, crosshair);
    drawCrosshair(crosshair, crosshair_radius);

    // Open mouse
    fd = open(pDevice, O_RDWR);
    if (fd == -1) {
        printf("ERROR Opening %s\n", pDevice);
        // return;
    }

    point p1 = create_point(-1, -1, 0, 0, 0);
    point p2 = create_point(-1, -1, 0, 0, 0);

    // Read mouse
    while (1) {
        bytes = read(fd, data, sizeof(data));

        if (bytes > 0) {
            left = data[0] & 0x1;
            right = data[0] & 0x2;
            middle = data[0] & 0x4;

            dx = data[1];
            dy = data[2];

            if (isCrosshairMoveValid(crosshair, dx, dy)) {
                drawTempCrosshair(crosshair_temp);

                crosshair.x += dx;
                crosshair.y -= dy;

                readTempCrosshair(crosshair_temp, crosshair);
                drawCrosshair(crosshair, crosshair_radius);
            }

            if (left > 0) {
                if(status == RED_STATUS) {
                    crosshair.c1 = 0;
                    crosshair.c2 = 0;
                    crosshair.c3 = 255;
                } else if(status == NORMAL_STATUS) {
                    crosshair.c1 = 0;
                    crosshair.c2 = 0;
                    crosshair.c3 = 0;
                } else if(status == GREEN_STATUS) {
                    crosshair.c1 = 0;
                    crosshair.c2 = 255;
                    crosshair.c3 = 0;
                } else if(status == BLUE_STATUS) {
                    crosshair.c1 = 255;
                    crosshair.c2 = 0;
                    crosshair.c3 = 0;
                }
                draw_point(crosshair, f);
            }

            if(right > 0){
                if(prev < 0){
                    p1.x = crosshair.x;
                    p1.y = crosshair.y;
                    prev = 1;
                }
            }
            else{
                if(prev > 0){
                    p2.x = crosshair.x;
                    p2.y = crosshair.y;

                    prev = -1;

                    if(status == RED_STATUS) {
                        p1.c1 = 0;
                        p1.c2 = 0;
                        p1.c3 = 255;
                        p2.c1 = 0;
                        p2.c2 = 0;
                        p2.c3 = 255;
                    } else if(status == NORMAL_STATUS) {
                        p1.c1 = 0;
                        p1.c2 = 0;
                        p1.c3 = 0;
                        p2.c1 = 0;
                        p2.c2 = 0;
                        p2.c3 = 0;
                    } else if(status == GREEN_STATUS) {
                        p1.c1 = 0;
                        p1.c2 = 255;
                        p1.c3 = 0;
                        p2.c1 = 0;
                        p2.c2 = 255;
                        p2.c3 = 0;
                    } else if(status == BLUE_STATUS) {
                        p1.c1 = 255;
                        p1.c2 = 0;
                        p1.c3 = 0;
                        p2.c1 = 255;
                        p2.c2 = 0;
                        p2.c3 = 0;
                    }

                    line l = create_line(p1, p2);
                    draw_line(l, f);
                }
            }

            drawToScreen(&f);
        }
    }
}

int main(int argc, char** argv) {
    // Input dimension
    if (argc != 3) {
        fprintf(stderr, "Please input dimension\n");
        fprintf(stderr, "Example: %s 1366 768\n", argv[0]);
        return -1;
    }

    dimension_x = atoi(argv[1]);
    dimension_y = atoi(argv[2])-6;
    pthread_create(threads, NULL, paint, NULL);
    pthread_create(threads+1, NULL, changeColor, NULL);
    pthread_exit(NULL);
    return 0; 
}
