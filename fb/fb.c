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
#include <math.h>

#define dnu

unsigned int pixel_color(unsigned char r, unsigned char g, unsigned char b);
void draw(unsigned int x, unsigned int y, unsigned int color);
void draw_line(Vector * a, Vector * b);
void paint(void);

static unsigned char * fbp, * bbp, * btmp; /*fbp = front buffer pointer, bbp = back buffer pointer*/
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo, orig;

int main () {  
    double centx, centy;
    struct timeval start, end, delta;
    Matrix * points, * rotation, *mtmp;
    Vector * shift1, *shift2 , * tmp;
    int screensize, x, y, location;
    int idx,i;
    /*open framebuffer*/
    int fb_fd = open("/dev/fb0", O_RDWR);
    /*get screen info*/
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    memcpy(&orig, &vinfo, sizeof(struct fb_var_screeninfo));
    vinfo.grayscale = 0;
    vinfo.bits_per_pixel = 32;
    vinfo.yres=240;
    vinfo.xres=320;
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    /*make buffer to write to screen*/
    screensize = finfo.smem_len;
    fbp = mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);

    
    /*make buffer to modify without messing with framebuffer, init to 0*/
    points = new_matrix(2,4);   

    points->columns[0]->elements[0] = 10; 
    points->columns[0]->elements[1] = 10; 

    points->columns[1]->elements[0] = 30; 
    points->columns[1]->elements[1] = 10; 

    points->columns[3]->elements[0] = 10; 
    points->columns[3]->elements[1] = 30; 

    points->columns[2]->elements[0] = 30; 
    points->columns[2]->elements[1] = 30; 

    shift1 = new_vector(2);
    shift2 = new_vector(2);

    rotation = new_matrix(2,2);
    rotation->columns[0]->elements[0] = .99985;
    rotation->columns[0]->elements[1] = -.01745;
    rotation->columns[1]->elements[1] = .99985;
    rotation->columns[1]->elements[0] = .01745;

    for (idx = 0; idx < 360; idx++){
        if (idx == 0) gettimeofday(&start, NULL);
        memset(fbp, 0, screensize);
        centx = (points->columns[0]->elements[0] + points->columns[1]->elements[0] + points->columns[2]->elements[0] + points->columns[3]->elements[0]) / 4; 
        centy = (points->columns[0]->elements[1] + points->columns[1]->elements[1] + points->columns[2]->elements[1] + points->columns[3]->elements[1]) / 4; 
        shift1->elements[0] = -centx;
        shift1->elements[1] = -centy;
        shift2->elements[0] = centx;
        shift2->elements[1] = centy;
        for (i = 0; i < points->n; i++) {
            tmp = points->columns[i];
            points->columns[i] = vector_add(tmp, shift1);
            free_vector(tmp);
        }
        mtmp = points;
        points = matrix_multiply(rotation, points);
        free_matrix(mtmp);
        for (i = 0; i < points->n; i++) {
            tmp = points->columns[i];
            points->columns[i] = vector_add(tmp, shift2);
            free_vector(tmp);
        }

        /*draw something*/
        draw_line(points->columns[0], points->columns[1]);
        draw_line(points->columns[1], points->columns[2]);
        draw_line(points->columns[2], points->columns[3]);
        draw_line(points->columns[3], points->columns[0]);
        if (idx == 0) gettimeofday(&end, NULL);
        usleep(12000);
#if !defined(dnu)
        /*swap buffers*/
        //for (i = 0; i < screensize; i++) 
        //   fbp[i] = bbp[i]; 
        if (vinfo.yoffset == 0)
            vinfo.yoffset = screensize;
        else
            vinfo.yoffset = 0;
        ioctl(fb_fd, FBIOPAN_DISPLAY, &vinfo);

        btmp = fbp;
        fbp = bbp;
        bbp = btmp;

        clear(bbp, &vinfo, &finfo);

#endif
    }

    free_matrix(points);
    free_vector(shift2);
    free_vector(shift1);
    delta = end;
    delta.tv_sec -= start.tv_sec;
    delta.tv_usec -= start.tv_usec;
    if (delta.tv_usec < 0) {
        delta.tv_usec +=1000000;
        delta.tv_sec--;
    }
    printf("%d.%06d s", delta.tv_sec, delta.tv_usec);
    
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &orig);
    munmap(fbp, screensize);
    close(fb_fd);
    printf("%d", vinfo.bits_per_pixel);
    return 0;
}

void paint(void) {

}

/*takes three 8 byte rgb vlues and a pointer to a fb_var_screeninfo and returns 
 * a pixel with the rgb filled in*/
unsigned int pixel_color(unsigned char r, unsigned char g, unsigned char b) {
   return (r<<vinfo.red.offset) | (g<<vinfo.green.offset) | (b<<vinfo.blue.offset);
} 

/*draws a single pixel onto the front buffer, if it is in range*/
void draw(unsigned int x, unsigned int y, unsigned int color) {
    unsigned int offset = 0;;
    if (x >= 0 && y >=0 && x < vinfo.xres && y < vinfo.yres) {
        offset = vinfo.bits_per_pixel/8 * x + y * finfo.line_length;
        *((unsigned int*)(fbp + offset)) = color;
    }
}

/*draws a line from the vector in a to the vector in b. must be in R2 */
void draw_line(Vector * a, Vector * b) {
    double slope, shift;
    int max, min, idx;
    unsigned int color = pixel_color(0xff, 0x00, 0xff);
    if (a->m == 2 && b->m == 2) {
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


