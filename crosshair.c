#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "framebuffer.h"
#include "point.h"
#include "line.h"
#include "polygon.h"

framebuffer f;
int dimension_x = 1366;
int dimension_y = 762;
int background_red = 255;
int background_green = 255;
int background_blue = 255;
int crosshair_red = 0;
int crosshair_green = 0;
int crosshair_blue = 0;
int crosshair_radius = 20;

void colorBackground() {
    for (int i = 0; i < dimension_x; i++) {
        for (int j = 0; j < dimension_y; j++) {
            draw_point(create_point(i, j, background_blue, background_green, background_red), f);
        }
    }
}

void eraseCrosshair(point p, int r) {
    line up = create_line(create_point(p.x, p.y-5, background_blue, background_green, background_red),
        create_point(p.x, p.y-r, background_blue, background_green, background_red));
    line down = create_line(create_point(p.x, p.y+5, background_blue, background_green, background_red),
        create_point(p.x, p.y+r, background_blue, background_green, background_red));
    line left = create_line(create_point(p.x-5, p.y, background_blue, background_green, background_red),
        create_point(p.x-r, p.y, background_blue, background_green, background_red));
    line right = create_line(create_point(p.x+5, p.y, background_blue, background_green, background_red),
        create_point(p.x+r, p.y, background_blue, background_green, background_red));

    draw_line(up, f);
    draw_line(down, f);
    draw_line(left, f);
    draw_line(right, f);
}

void drawCrosshair(point p, int r) {
    line up = create_line(create_point(p.x, p.y-5, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x, p.y-r, crosshair_blue, crosshair_green, crosshair_red));
    line down = create_line(create_point(p.x, p.y+5, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x, p.y+r, crosshair_blue, crosshair_green, crosshair_red));
    line left = create_line(create_point(p.x-5, p.y, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x-r, p.y, crosshair_blue, crosshair_green, crosshair_red));
    line right = create_line(create_point(p.x+5, p.y, crosshair_blue, crosshair_green, crosshair_red),
        create_point(p.x+r, p.y, crosshair_blue, crosshair_green, crosshair_red));

    draw_line(up, f);
    draw_line(down, f);
    draw_line(left, f);
    draw_line(right, f);
}

int isCrosshairMoveValid(point c, int dx, int dy) {
    return c.x+dx-crosshair_radius > 0 && c.x+dx+crosshair_radius < dimension_x && 
        c.y-dy-crosshair_radius > 0 && c.y-dy+crosshair_radius < dimension_y;
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

    // Initialize framebuffer and background
    f = init();
    colorBackground();

    // Mouse variables
    int fd, bytes;
    unsigned char data[3];

    const char *pDevice = "/dev/input/mice";

    int left, middle, right;
    signed char dx, dy;

    // Create crosshair
    point crosshair = create_point(dimension_x/2, (dimension_y)/2, crosshair_blue, crosshair_green, crosshair_red);
    drawCrosshair(crosshair, crosshair_radius);

    // Open mouse
    fd = open(pDevice, O_RDWR);
    if (fd == -1) {
        printf("ERROR Opening %s\n", pDevice);
        return -1;
    }

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
                eraseCrosshair(crosshair, crosshair_radius);

                crosshair.x += dx;
                crosshair.y -= dy;

                drawCrosshair(crosshair, crosshair_radius);
            }

            if (left > 0) {
                draw_point(crosshair, f);
            }
        }
    }
    return 0; 
}
