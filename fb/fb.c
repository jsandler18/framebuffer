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
void fill_poly(Matrix * points, unsigned int color);
int is_in_poly(Matrix * points, Vector * test);

static unsigned char * fbp, * bbp, * btmp; /*fbp = front buffer pointer, bbp = back buffer pointer*/
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo, orig;

int main () {  
    struct timeval start, end, delta;
    Matrix * points, * mouse, *mtmp;
    Vector * shift1, *shift2 , * tmp;
    unsigned int color;
    int screensize, x, y, location;
    int idx,i;
    int mouse_fd;
    char mouse_event[3];
    char direction = 0, button = 0;
    char tmpstr[5];
    /*open mouse*/
    mouse_fd = open("/dev/input/mice", O_RDONLY);
    if (mouse_fd == -1) {
        printf("could not open mouse\n");
        exit(-1);
    }
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
    bbp = (unsigned char *) malloc(screensize);
    /*make mouse matrix*/
    mouse = new_matrix(2,7);

    mouse->columns[0]->elements[0] = 5;
    mouse->columns[0]->elements[1] = 5;

    mouse->columns[1]->elements[0] = 5;
    mouse->columns[1]->elements[1] = 15;

    mouse->columns[2]->elements[0] = 7;
    mouse->columns[2]->elements[1] = 13;

    mouse->columns[3]->elements[0] = 9;
    mouse->columns[3]->elements[1] = 19;

    mouse->columns[4]->elements[0] = 11;
    mouse->columns[4]->elements[1] = 19;

    mouse->columns[5]->elements[0] = 9;
    mouse->columns[5]->elements[1] = 13;

    mouse->columns[6]->elements[0] = 14;
    mouse->columns[6]->elements[1] = 13;


    /*make matrix of points*/
    points = new_matrix(2,4);   

    points->columns[0]->elements[0] = 10; 
    points->columns[0]->elements[1] = 10; 

    points->columns[1]->elements[0] = 30; 
    points->columns[1]->elements[1] = 10; 

    points->columns[2]->elements[0] = 30; 
    points->columns[2]->elements[1] = 30; 

    points->columns[3]->elements[0] = 10; 
    points->columns[3]->elements[1] = 30; 

    shift1 = new_vector(2);

    while (1) {
        if (idx == 0) gettimeofday(&start, NULL);
        read (mouse_fd, mouse_event, 3);
        button = mouse_event[0] & 0x0f;
        if (button == 10) color = pixel_color(0x00, 0xff, 0x00);
        if (button == 8) color = pixel_color(0x00, 0x00, 0xff);
        if (button == 11) break;
        memset(bbp, 0, screensize);
        sprintf(tmpstr, "%hhd", mouse_event[1]);
        shift1->elements[0] = atoi(tmpstr);
        sprintf(tmpstr, "%hhd", mouse_event[2]);
        shift1->elements[1] = -atoi(tmpstr);
        for (i = 0; i < mouse->n; i++) {
            tmp = mouse->columns[i];
            mouse->columns[i] = vector_add(tmp, shift1);
            free_vector(tmp);
        }
        if(is_in_poly(points, mouse->columns[0]) && button == 9) {
            for (i = 0; i < points->n; i++) {
                tmp = points->columns[i];
                points->columns[i] = vector_add(tmp, shift1);
                free_vector(tmp);
            }
        }

        /*draw something*/
        fill_poly(points, pixel_color(0xff, 0xff, 0x00));
        fill_poly(mouse, color);
        /*swap buffers*/
        for (i = 0; i < screensize; i++) 
           fbp[i] = bbp[i]; 
        if (idx == 0) gettimeofday(&end, NULL);
        //usleep(12000);
    }
    while (button != 8) {
        read (mouse_fd, mouse_event, 3);
        button = mouse_event[0] & 0x0f;
    }

    free_matrix(points);
    free_matrix(mouse);
    free_vector(shift1);
    delta = end;
    delta.tv_sec -= start.tv_sec;
    delta.tv_usec -= start.tv_usec;
    if (delta.tv_usec < 0) {
        delta.tv_usec +=1000000;
        delta.tv_sec--;
    }
    printf("%d.%06d s\n", delta.tv_sec, delta.tv_usec);
    
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &orig);
    munmap(fbp, screensize);
    close(fb_fd);
    close(mouse_fd);
    return 0;
}


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
