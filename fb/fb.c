#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "linalg.h"
#include <sys/resource.h>
#include "draw.h"

int main () {  
    Matrix * points, * mouse, *mtmp;
    Vector * shift1, *shift2 , * tmp;
    unsigned int color;
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
    init_screen(240, 320);
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
        read (mouse_fd, mouse_event, 3);
        button = mouse_event[0] & 0x0f;
        if (button == 10) color = pixel_color(0x00, 0xff, 0x00);
        if (button == 8) color = pixel_color(0x00, 0x00, 0xff);
        if (button == 11) break;
        clear_screen();
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
        swap_buffers();
    }
    while (button != 8) {
        read (mouse_fd, mouse_event, 3);
        button = mouse_event[0] & 0x0f;
    }

    free_matrix(points);
    free_matrix(mouse);
    free_vector(shift1);
    
    restore_screen();

    close(mouse_fd);
    return 0;
}

