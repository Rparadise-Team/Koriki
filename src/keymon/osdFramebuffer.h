#ifndef OSDFRAMEBUFFER_H
#define OSDFRAMEBUFFER_H

// OSD data
#define OSD_NONE	0
#define	OSD_VOLUME	1
#define OSD_BRIGHTNESS	2

int get_miyoo_v4();
int init_framebuffer();
void get_render_info();
void draw_line(int value, int cr, unsigned char cg, unsigned char cb, unsigned char ct);
void clear_line();
void close_framebuffer();
void osd_show(int item);

#endif

