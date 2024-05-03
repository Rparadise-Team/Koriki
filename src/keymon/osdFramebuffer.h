#ifndef OSDFRAMEBUFFER_H
#define OSDFRAMEBUFFER_H

// OSD data
#define OSD_NONE	0
#define	OSD_VOLUME	1
#define OSD_BRIGHTNESS	2

int get_miyoo_v4();
int init_framebuffer();
void save_background();
void restore_background(int frame);
int get_icon_cutoff();
void get_render_info();
void close_framebuffer();
void osd_show(int item);

#endif

