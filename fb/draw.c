#include "draw.h"
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../linalg/linalg.h"
#include <sys/time.h>
#include <sys/resource.h>

/*takes three 8 byte rgb vlues and a pointer to a fb_var_screeninfo and returns 
 * a pixel with the rgb filled in*/
unsigned int pixel_color(unsigned char r, unsigned char g, unsigned char b) {
   return (r<<vinfo.red.offset) | (g<<vinfo.green.offset) | (b<<vinfo.blue.offset);
} 

/*draws a single pixel onto the front buffer, if it is in range*/
void draw(unsigned int x, unsigned int y, unsigned int color) {
    unsigned int offset = 0;
    if (x >= 0 && y >=0 && x < vinfo.xres && y < vinfo.yres) {
        offset = vinfo.bits_per_pixel/8 * x + y * finfo.line_length;
        *((unsigned int*)(bbp + offset)) = color;
    }
}

/*draws a line from the vector in a to the vector in b. must be in R2 */
void draw_line(Vector * a, Vector * b) {
    double slope, shift;
    int max, min, idx;
    unsigned int color = pixel_color(0xff, 0x00, 0xff);
    if (a != NULL && b != NULL && a->m == 2 && b->m == 2) {
        if (a->elements[0] - b->elements[0] == 0) { /*verticle line*/
            max = a->elements[1] > b->elements[1] ? a->elements[1]:b->elements[1];
            min = a->elements[1] < b->elements[1] ? a->elements[1]:b->elements[1];
            for (idx = min; idx < max && idx < vinfo.yres; idx++) 
                draw(a->elements[0], idx, color);

        } else { /*not verticle line*/
            slope = (a->elements[1] - b->elements[1])/(a->elements[0] - b->elements[0]);
            shift = a->elements[1] - slope * a->elements[0];
            max = a->elements[0] > b->elements[0] ? a->elements[0]:b->elements[0];
            min = a->elements[0] < b->elements[0] ? a->elements[0]:b->elements[0];
            for (idx = min; idx < max && idx < vinfo.xres; idx++)
                draw(idx, (int)(idx * slope + shift), color);

        }
    }
}

/*fills in a polygon whose points are represented as the vectors in the 
 * given matrix*/
void fill_poly(Matrix * points, unsigned int color) {
    double ymin = 0, ymax = vinfo.yres;
    int x, y, i, j = points->n - 1;
    double nodelist[30] = {0}; /*assuming there will be less than 30 crosses*/
    int nodes = 0, swap;

    if (points != NULL) {
        for (i = 0; i < points->n; i++) { /*get bounding points*/
            ymin = ymin < points->columns[i]->elements[1] ? ymin : points->columns[i]->elements[1];
            ymax = ymax > points->columns[i]->elements[1] ? ymax : points->columns[i]->elements[1];
        }

        for (y = ymin; y < ymax; y++) { /*loop through rows*/
            nodes = 0;
            for (i = 0; i <  points->n; j = i++) { /*get polygon bound points*/
                if (points->columns[i]->elements[1] < y && points->columns[j]->elements[1] >= y
                ||  points->columns[j]->elements[1] < y && points->columns[i]->elements[1] >= y) {
                    nodelist[nodes++] = points->columns[i]->elements[0] + (y - points->columns[i]->elements[1]) / (points->columns[j]->elements[1] - points->columns[i]->elements[1]) * (points->columns[j]->elements[0] - points->columns[i]->elements[0]);
                }
            }
            /*sort the nodes*/
            i = 0;
            while (i < nodes-1) {
                if(nodelist[i] > nodelist[i+1]) {
                    swap = nodelist[i];
                    nodelist[i] = nodelist[i+1];
                    nodelist[i+1] = swap;
                    if (i) i--;
                }
                else i++;
            }

            for (i = 0; i < nodes; i+=2) {
                if (nodelist[i] >= vinfo.yres) break;
                if (nodelist[i+1] > 0) {
                    if (nodelist[i] < 0) nodelist[i] = 0;
                    if (nodelist[i+1] > vinfo.yres) nodelist[i+1] = vinfo.yres;
                    for (x = nodelist[i]; x < nodelist[i+1]; x++) { /*loop through columns*/
                        draw(x,y,color);
                    }
                }
            }
        }
    }
}
/*tests if the given vector is within the given polygon matrix*/
int is_in_poly(Matrix * points, Vector * test) {
    int result = 0, i, j;
    if (points != NULL && test != NULL && points->columns[0]->m == 2 && test->m == 2) {
        for (i = 0, j = points->n -1; i < points->n; j = i++) {
            if ( ((points->columns[i]->elements[1] > test->elements[1]) != (points->columns[j]->elements[1] > test->elements[1])) &&
                    test->elements[0] < (points->columns[j]->elements[0] - points->columns[i]->elements[0]) * (test->elements[1] - points->columns[i]->elements[1]) / (points->columns[j]->elements[1] - points->columns[i]->elements[1]) + points->columns[i]->elements[0]) {
                result = !result;
            }
        }
    return result;
    }
}

/*swaps the front and back buffers, effectively refreshing the screen*/
void swap_buffers(void) {
    /*swap buffers*/
    for (i = 0; i < finfo.smem_len; i++) 
       fbp[i] = bbp[i]; 
}

/*inits the front and back buffers and sets the resolution, prepares the screen 
 * for writing*/
void init_screen(int xres, int yres) {
    /*open framebuffer*/
    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        printf("ERROR: Could not open screen");
        exit(-1);
    }
    /*get screen info*/
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    memcpy(&orig, &vinfo, sizeof(struct fb_var_screeninfo));
    vinfo.grayscale = 0;
    vinfo.bits_per_pixel = 32;
    vinfo.yres=xres;
    vinfo.xres=yres;
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    /*make buffer to write to screen*/
    fbp = mmap(NULL, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);
    bbp = (unsigned char *) malloc(finfo.smem_len);

}

/*restores the screen to its original state and frees any memory allocated for 
 * the framebuffer*/
void restore_screen(void) {
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &orig);
    munmap(fbp, finfo.smem_len);
    free(bbp);
    close(fb_fd);
}

void clear_screen(void) {
    memset(bbp, 0, finfo.smem_len);
}

