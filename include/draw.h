/*this header file contains function prototypes for tings related to drawing, 
 * including making a color, plotting a point, swapping buffers, filling in 
 * polygons, etc.  Also contains global variables for the  front and back 
 * buffers, the file descriptor for the framebuffer and structures to hold 
 * screen info*/
#if !defined(DRAW_H)
#define DRAW_H
unsigned int pixel_color(unsigned char r, unsigned char g, unsigned char b);
void draw(unsigned int x, unsigned int y, unsigned int color);
void draw_line(Vector * a, Vector * b);
void fill_poly(Matrix * points, unsigned int color);
int is_in_poly(Matrix * points, Vector * test);
void swap_buffers(void);
void init_screen(int xres, int yres);
void restore_screen(void);
void clear_screen(void);

unsigned char * fbp, * bbp, * btmp; 
int fb_fd;
struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo, orig;
#endif
