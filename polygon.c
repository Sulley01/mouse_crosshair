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
#include "line.h"
#include "framebuffer.h"
#include "polygon.h"

void allocate_memory(polygon *pol, int size){
	pol->size = size;
	pol->arr = (int **)malloc((pol->size+1) * sizeof(int *));

	for (int i=0; i<(pol->size+1); i++){
         pol->arr[i] = (int *)malloc(2 * sizeof(int));
    }
}

polygon create_polygon_from_file(char *filename, int c1, int c2, int c3, int x_resolution, int y_resolution){
	polygon pol;
	pol.c1 = c1;
	pol.c2 = c2;
	pol.c3 = c3;
	pol.x_resolution = x_resolution;
	pol.y_resolution = y_resolution;

	FILE *fp;  
    
    int size;
    fp = fopen(filename, "r");
    fscanf(fp, "%d", &pol.size);

    allocate_memory(&pol, pol.size);

    int i = 0;
    int j = 0;

    int temp;
    while(fscanf(fp, "%d", &temp)!=EOF){  
        if(j%2==0){
            pol.arr[i][0] = temp;
        }
        else{
            pol.arr[i][1] = temp;
            i++;
        }
        j++;  
    }
    
    fclose(fp);

    //Find center of polygon
    int maxX = INT_MIN;
    int minX = INT_MAX;

    int maxY = INT_MIN;
    int minY = INT_MAX;

    for(int i=0; i<pol.size; i++){
    	if(maxX < pol.arr[i][0]){
    		maxX = pol.arr[i][0];
    	}
    	if(minX > pol.arr[i][0]){
    		minX = pol.arr[i][0];
    	}

    	if(maxY < pol.arr[i][1]){
    		maxY = pol.arr[i][1];
    	}
    	if(minY > pol.arr[i][1]){
    		minY = pol.arr[i][1];
    	}
    }

    pol.x_center = minX + (maxX - minX)/2;
    pol.y_center = minY + (maxY - minY)/2;

    return pol;
}

void draw_polygon(polygon pol, framebuffer f){
	int i,j,k,gd,gm,dy,dx;
    int x,y,temp;
    int xi[pol.size];
    float slope[pol.size];
   
    // Initiate Variables
    pol.arr[pol.size][0]=pol.arr[0][0];
    pol.arr[pol.size][1]=pol.arr[0][1];
  
    // Calculate slope
    for(i=0;i<pol.size;i++) {
        dy=pol.arr[i+1][1]-pol.arr[i][1];
        dx=pol.arr[i+1][0]-pol.arr[i][0];
        if(dy==0) slope[i]=1.0;
        if(dx==0) slope[i]=0.0;
        if((dy!=0)&&(dx!=0)) {
            slope[i]=(float) dx/dy;
        }
    }
  
    // Fill color
    for(y=0;y<pol.y_resolution;y++) {
        k=0;
        // Initiate xi
        for(i=0;i<pol.size;i++) {
            if (((pol.arr[i][1]<=y)&&(pol.arr[i+1][1]>y)) || ((pol.arr[i][1]>y)&&(pol.arr[i+1][1]<=y))) {
                xi[k]=(int)(pol.arr[i][0]+slope[i]*(y-pol.arr[i][1]));
                k++;
            }
        }
        // Sort xi
        for(j=0;j<k-1;j++)
            for(i=0;i<k-1;i++) {
                if(xi[i]>xi[i+1]) {
                    temp=xi[i];
                    xi[i]=xi[i+1];
                    xi[i+1]=temp;
                }
            }
        // Fill
        for(i=0;i<k;i+=2) {
        	point p1 = create_point(xi[i], y, pol.c1, pol.c2, pol.c3);
        	point p2 = create_point(xi[i+1]+1,y,pol.c1,pol.c2,pol.c3);
            
            line l = create_line(p1, p2);
            draw_line(l, f);
        }
    }
}

void set_color_polygon(polygon *pol, int c1, int c2, int c3){
	pol->c1 = c1;
	pol->c2 = c2;
	pol->c3 = c3;
}

int findIntersect (int is_y_known, int point_idx, int x1, int y1, int x2, int y2, polygon frame) {
	char known;
	if (is_y_known) {
		known = 'y';
	} else {
		known = 'x';
	}
	//printf("Intersecting (%d,%d) to (%d,%d) on %c id %d: ",x1,y1,x2,y2,known,point_idx);
	int x,y,result,known_val;
	if (is_y_known) {
		if (known_val == 1) {
			known_val = y1;
		} else {
			known_val = y2;
		}
	} else {
		if (known_val == 1) {
			known_val = x1;
		} else {
			known_val = x2;
		}
	}

	if (x1 == x2) {
		
		//printf("Vertical line. ");
		if (is_y_known) {
			if (x1 < frame.arr[0][0]) {
				result = frame.arr[0][0];
			} else if (x1 > frame.arr[1][0]) {
				result = frame.arr[1][0];
			} else {
				result =  x1;
			}
		} else {
			y = x1 == known_val ? y1 : y2;
			if (y < frame.arr[0][1]) {
				result = frame.arr[0][1];
			} else if (y > frame.arr[2][1]) {
				result = frame.arr[2][1];
			} else {
				result = y;
			}
		}
	} else {
		int m = ((y2 - y1)/(x2 - x1));
		if (m == 0) {
			//printf("Horizontal Line. ");
			if (is_y_known) {
				x = point_idx == 1 ? x1 : x2;
				if (x < frame.arr[0][0]) {
					result = frame.arr[0][0];
				} else if (x > frame.arr[1][0]) {
					result = frame.arr[1][0];
				} else {
					result = x;
				}
			} else {
				if (y1 < frame.arr[0][1]) {
					result = frame.arr[0][1];
				} else if (y1 > frame.arr[2][1]) {
					result = frame.arr[2][1];
				} else {
					result = y1;
				}
			}
		} else {
			int c = y2 - (m * x2);
			if (is_y_known) {
				result = (int)((known_val - c)/m);
				if (result < frame.arr[0][0]) {
					result = frame.arr[0][0];
				} else if (result > frame.arr[1][0]) {
					result = frame.arr[1][0];
				}
			} else {
				result = (int)(m * known_val) + c;
				if (result < frame.arr[0][1]) {
					result = frame.arr[0][1];
				} else if (result > frame.arr[2][1]) {
					result = frame.arr[2][1];
				}
			}
		}
	}
	//printf("Returning %d\n",result);
	return result;

}

void add_point_to_polygon (polygon *res, int x, int y) {
	//printf("Adding (%d,%d) to %d\n",x,y,res->size);
	if (res->size == 0){
		res->arr[0][0] = x;
		res->arr[0][1] = y;
		res->size = res->size + 1;
		//printf("Added (%d,%d) to %d\n",x,y,res->size - 1);
	} else {
		if (res->arr[res->size-1][0] != x || res->arr[res->size-1][1] != y) {
			res->arr[res->size][0] = x;
			res->arr[res->size][1] = y;
			res->size = res->size + 1;
			//printf("Added (%d,%d) to %d\n",x,y,res->size - 1);
		} else {
			//printf("Duplicate found. Ingnoring.\n");
		}
	}
}

void swap_id(int *id) {
	if (*id == 1) {
		*id = 2;
	} else {
		*id = 1;
	}
}

polygon dilate(polygon pol, polygon frame, int scale, int x_center, int y_center){

	//printf("Starting dilatation ...\n");

	int flag_counter = 0;
	int x1, x2, y1, y2, xmin, xmax, ymin, ymax, pidx, id;
	polygon res;	

	res.c1 = pol.c1;
	res.c2 = pol.c2;
	res.c3 = pol.c3;
	res.size = pol.size * 2 + 1;
	res.x_resolution = pol.x_resolution;
	res.y_resolution = pol.y_resolution;
	res.x_center = pol.x_center;
	res.y_center = pol.y_center;

	allocate_memory(&res, res.size);
	res.size = 0;

    for (int i = 0; i < (pol.size - 1); i++) {

		//res.arr[i][0] = pol.arr[i][0] * scale - (scale-1) * x_center;
        //res.arr[i][1] = pol.arr[i][1] * scale - (scale-1) * y_center;
        
    	//printf("Iteration =  %d\n", i);

        x1 = pol.arr[i][0] * scale - (scale-1) * x_center;
        y1 = pol.arr[i][1] * scale - (scale-1) * y_center;
		//printf("Found (%d,%d) to ",x1,y1);

        x2 = pol.arr[i+1][0] * scale - (scale-1) * x_center;
        y2 = pol.arr[i+1][1] * scale - (scale-1) * y_center;
        //printf("(%d,%d) ",x2,y2);

        xmin = x1 > x2 ? x2 : x1;
		xmax = x1 > x2 ? x1 : x2;
		ymin = y1 > y2 ? y2 : y1;
		ymax = y1 > y2 ? y1 : y2;

		//printf("Mapping to frame (%d - %d, %d - %d)\n",frame.arr[0][0],frame.arr[1][0],frame.arr[0][1],frame.arr[2][1]);

		flag_counter = 0;

		if (x1 < frame.arr[0][0]) {
			add_point_to_polygon(&res, frame.arr[0][0],findIntersect(0,1,x1,y1,x2,y2,frame));
		} else if (x1 > frame.arr[1][0]) {
			add_point_to_polygon(&res, frame.arr[1][0],findIntersect(0,1,x1,y1,x2,y2,frame));
		} else {
			if (y1 < frame.arr[0][1]) {
				add_point_to_polygon(&res, findIntersect(1,1,x1,y1,x2,y2,frame),frame.arr[0][1]);
			} else if (y1 > frame.arr[2][1]) {
				add_point_to_polygon(&res, findIntersect(1,1,x1,y1,x2,y2,frame),frame.arr[2][1]);
			} else {
				add_point_to_polygon(&res, x1,y1);
			}
		}

		if (x2 < frame.arr[0][0]) {
			add_point_to_polygon(&res, frame.arr[0][0],findIntersect(0,2,x1,y1,x2,y2,frame));
		} else if (x2 > frame.arr[1][0]) {
			add_point_to_polygon(&res, frame.arr[1][0],findIntersect(0,2,x1,y1,x2,y2,frame));
		} else {
			if (y2 < frame.arr[0][1]) {
				add_point_to_polygon(&res, findIntersect(1,2,x1,y1,x2,y2,frame),frame.arr[0][1]);
			} else if (y2 > frame.arr[2][1]) {
				add_point_to_polygon(&res, findIntersect(1,2,x1,y1,x2,y2,frame),frame.arr[2][1]);
			} else {
				add_point_to_polygon(&res, x2,y2);
			}
		}
		
    }

    for (int i=0; i<pol.size; i++) {
    	//printf("(%d,%d) ", pol.arr[i][0], pol.arr[i][1]);
    }
    printf("\n");
	for (int i=0; i<res.size; i++) {
    	//printf("(%d,%d) ", res.arr[i][0], res.arr[i][1]);
    }
    //printf("\n");

    //printf("Dilatation ended with new size of %d. Returning results ...\n", res.size);

    return res;
}

polygon translate(polygon pol, polygon frame, int x_distance, int y_distance){
	polygon res;

    int x1, y1, x2, y2;

    res.c1 = pol.c1;
	res.c2 = pol.c2;
	res.c3 = pol.c3;
	res.size = pol.size * 2 + 1;
	res.x_resolution = pol.x_resolution;
	res.y_resolution = pol.y_resolution;
	res.x_center = pol.x_center + x_distance;
	res.y_center = pol.y_center + y_distance;

	allocate_memory(&res, pol.size);
	res.size = 0;

	for(int i=0; i<(pol.size-1); i++){

		x1 = pol.arr[i][0] + x_distance;
        y1 = pol.arr[i][1] + y_distance;
		//printf("Found (%d,%d) to ",x1,y1);

        x2 = pol.arr[i+1][0] + x_distance;
        y2 = pol.arr[i+1][1] + y_distance;
        //printf("(%d,%d) ",x2,y2);

		// res.arr[i][0] = pol.arr[i][0] + x_distance;
		// res.arr[i][1] = pol.arr[i][1] + y_distance;

        if (x1 < frame.arr[0][0]) {
			add_point_to_polygon(&res, frame.arr[0][0],findIntersect(0,1,x1,y1,x2,y2,frame));
		} else if (x1 > frame.arr[1][0]) {
			add_point_to_polygon(&res, frame.arr[1][0],findIntersect(0,1,x1,y1,x2,y2,frame));
		} else {
			if (y1 < frame.arr[0][1]) {
				add_point_to_polygon(&res, findIntersect(1,1,x1,y1,x2,y2,frame),frame.arr[0][1]);
			} else if (y1 > frame.arr[2][1]) {
				add_point_to_polygon(&res, findIntersect(1,1,x1,y1,x2,y2,frame),frame.arr[2][1]);
			} else {
				add_point_to_polygon(&res, x1,y1);
			}
		}

		if (x2 < frame.arr[0][0]) {
			add_point_to_polygon(&res, frame.arr[0][0],findIntersect(0,2,x1,y1,x2,y2,frame));
		} else if (x2 > frame.arr[1][0]) {
			add_point_to_polygon(&res, frame.arr[1][0],findIntersect(0,2,x1,y1,x2,y2,frame));
		} else {
			if (y2 < frame.arr[0][1]) {
				add_point_to_polygon(&res, findIntersect(1,2,x1,y1,x2,y2,frame),frame.arr[0][1]);
			} else if (y2 > frame.arr[2][1]) {
				add_point_to_polygon(&res, findIntersect(1,2,x1,y1,x2,y2,frame),frame.arr[2][1]);
			} else {
				add_point_to_polygon(&res, x2,y2);
			}
		}

	}

	//res = dilate(res, frame, 1, 0, 0);

	return res;
}

polygon rotate(polygon pol, int x_center, int y_center) {
    polygon res;
    allocate_memory(&res, pol.size);

    res.c1 = pol.c1;
	res.c2 = pol.c2;
	res.c3 = pol.c3;
	res.size = pol.size;
	res.x_resolution = pol.x_resolution;
	res.y_resolution = pol.y_resolution;
	res.x_center = pol.x_center;
	res.y_center = pol.y_center;

    for (int i = 0; i < res.size; i++) {
        res.arr[i][0] = (int)((float)(pol.arr[i][0]-x_center)*sqrt(2.0)/2.0 - (float)(pol.arr[i][1]-y_center)*sqrt(2.0)/2.0) + x_center;
     	res.arr[i][1] = (int)((float)(pol.arr[i][0]-x_center)*sqrt(2.0)/2.0 + (float)(pol.arr[i][1]-y_center)*sqrt(2.0)/2.0) + y_center;
    }

    return res;
}