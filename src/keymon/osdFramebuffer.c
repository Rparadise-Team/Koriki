#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <cstring>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "osdFramebuffer.h"

#define VOLUME_STEPS		69
#define BRIGHTNESS_STEPS	10

// Framebuffer data
int fb_fd = -1;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fb_addr = NULL;
char *fb_barbackground = NULL;
int fb_lastframe;

// Miyoo model data
int miyoo_v4_mode = -1;

// OSD data
pthread_t thread_id;
int osd_running=0;
struct timeval osd_timer;
int osd_item=OSD_NONE;
int osd_volume;
int osd_brightness;

/*void LOG(int n, int m) {
	FILE *file= fopen("/mnt/SDCARD/logk.txt","w");
	if(file) {
		char txt[150];
		sprintf(txt,"Valor 1: %d\nValor 2: %d", n,m);
		fwrite(txt,1,sizeof(txt),file);
		fclose(file);
	}
}*/

// get Miyoo v4 model
int get_miyoo_v4() {
	FILE* miyoov4 = fopen("/tmp/new_res_available", "r");
	
	if (miyoov4) { //get miyoo v4 mode
		FILE *fp;
		char buffer[64];
		const char *cmd = "pgrep retroarch";
    
		fp = popen(cmd, "r");
		if (fp == NULL) {
		fclose(miyoov4);
		return 1;
		}
    
		if (fgets(buffer, sizeof(buffer), fp) != NULL) {
			fclose(miyoov4);
			pclose(fp);
			return 2;
		}
	}
	
	if (!miyoov4)
		return 0;
		
	fclose(miyoov4);
	return 0;
}

// Get framebuffer resolution
void get_render_info() {
	if (fb_fd == -1)
		return;

	// Get variable screen information
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
		return;
}

// Initializa framebuffer
int init_framebuffer() {
	if (fb_fd == -1)
		fb_fd = open("/dev/fb0", O_RDWR);

	// Get fixed screen information
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1)
		return 0;

	// Map the device to memory
	fb_addr = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (fb_addr == MAP_FAILED) {
		close(fb_fd);
		fb_fd = -1;
		return 0;
	}

	// Get render resolution
	get_render_info();

	return 1;
}

// Save bar background to memory
void save_background() {
	// Determine the correct resolution at runtime
	int width, height;
	if (miyoo_v4_mode == 1) {
		width = 640;
		height = 480;
	} else if (miyoo_v4_mode == 2) {
		width = 752;
		height = 560;
	} else {
		width = vinfo.xres;
		height = vinfo.yres;
	}

	// Get memory to save bar background
	if(fb_barbackground == NULL)
		fb_barbackground = (char*)malloc(height*6*(vinfo.bits_per_pixel/8));

	// only save background when it's new
	if(fb_barbackground && fb_lastframe != (int)vinfo.yoffset) {
		int x, y;
		long int location = 0, idx_background = 0;
		fb_lastframe = vinfo.yoffset;

		for (y = 0; y < height; y++)
			for (x = width-6; x < width; x++) {
				location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * width*(vinfo.bits_per_pixel/8);
				if(vinfo.bits_per_pixel == 32) {
					*(fb_barbackground + idx_background) = *(fb_addr + location);
					*(fb_barbackground + idx_background +1) = *(fb_addr + location +1);
					*(fb_barbackground + idx_background +2) = *(fb_addr + location +2);
					*(fb_barbackground + idx_background +3) = *(fb_addr + location +3);
					idx_background += 4;
				} else if(vinfo.bits_per_pixel == 24) {
					*(fb_barbackground + idx_background) = *(fb_addr + location);
					*(fb_barbackground + idx_background +1) = *(fb_addr + location +1);
					*(fb_barbackground + idx_background +2) = *(fb_addr + location +2);
					idx_background += 3;
				} else if(vinfo.bits_per_pixel == 16) {
					*(fb_barbackground + idx_background) = *(fb_addr + location);
					*(fb_barbackground + idx_background +1) = *(fb_addr + location +1);
					idx_background += 2;
				} else if(vinfo.bits_per_pixel == 8) {
					*(fb_barbackground + idx_background) = *(fb_addr + location);
					idx_background++;
				}
			}
	}
}

// Restore saved background to screen
void restore_background() {
	// Determine the correct resolution at runtime
	int width, height;
	if (miyoo_v4_mode == 1) {
		width = 640;
		height = 480;
	} else if (miyoo_v4_mode == 2) {
		width = 752;
		height = 560;
	} else {
		width = vinfo.xres;
		height = vinfo.yres;
	}

	if(fb_barbackground) {
		int x, y;
		long int location = 0, idx_background = 0;
		fb_lastframe = vinfo.yoffset;

		for (y = 0; y < height; y++)
			for (x = width-6; x < width; x++) {
				location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * width*(vinfo.bits_per_pixel/8);
				if(vinfo.bits_per_pixel == 32) {
					*(fb_addr + location) = *(fb_barbackground + idx_background);
					*(fb_addr + location +1) = *(fb_barbackground + idx_background +1);
					*(fb_addr + location +2) = *(fb_barbackground + idx_background +2);
					*(fb_addr + location +3) = *(fb_barbackground + idx_background +3);
					idx_background += 4;
				} else if(vinfo.bits_per_pixel == 24) {
					*(fb_addr + location) = *(fb_barbackground + idx_background);
					*(fb_addr + location +1) = *(fb_barbackground + idx_background +1);
					*(fb_addr + location +2) = *(fb_barbackground + idx_background +2);
					idx_background += 3;
				} else if(vinfo.bits_per_pixel == 16) {
					*(fb_addr + location) = *(fb_barbackground + idx_background);
					*(fb_addr + location +1) = *(fb_barbackground + idx_background +1);
					idx_background += 2;
				} else if(vinfo.bits_per_pixel == 8) {
					*(fb_addr + location) = *(fb_barbackground + idx_background);
					idx_background++;
				}
			}
		// free memory and reset pointer and last frame saved
		free(fb_barbackground);
		fb_barbackground = NULL;
		fb_lastframe = -1;
		miyoo_v4_mode = -1;
	}
}

// Draw a line with multiple colors
void draw_multiline(int value, int step, int top1, int top2, int top3, float alpha) {
	int x = 0, y = 0;
	long int location = 0;
	unsigned char cr=0, cg=255, cb=0, ct=0;

	get_render_info();
	save_background();
	
	// Determine the correct resolution at runtime
	int width, height;
	if (miyoo_v4_mode == 1) {
		width = 640;
		height = 480;
	} else if (miyoo_v4_mode == 2) {
		width = 752;
		height = 560;
	} else {
		width = vinfo.xres;
		height = vinfo.yres;
	}

	int limit=value*height/step;
	top1=top1*height/step;
	top2=top2*height/step;
	top3=top3*height/step;

	for (y = 0; y < height; y++)
		for (x = width-6; x < width; x++) {
			location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * width*(vinfo.bits_per_pixel/8);
			
			// Calculate blended color
			unsigned char blended_b = (1 - alpha) * *(fb_addr + location) + alpha * cb;
			unsigned char blended_g = (1 - alpha) * *(fb_addr + location + 1) + alpha * cg;
			unsigned char blended_r = (1 - alpha) * *(fb_addr + location + 2) + alpha * cr;
           	unsigned char blended_t = (1 - alpha) * *(fb_addr + location + 3) + alpha * ct;
			
			if(y<limit) {
				// color level 1
				if(y<top1) {
					if (vinfo.bits_per_pixel == 32) {
						*(fb_addr + location) = cb;		// blue
						*(fb_addr + location + 1) = cg;		// green
						*(fb_addr + location + 2) = cr;		// red
						*(fb_addr + location + 3) = blended_t;		// transparency
					} else if (vinfo.bits_per_pixel == 24) {
						*(fb_addr + location) = cb;		// blue
						*(fb_addr + location + 1) = cg;		// green
						*(fb_addr + location + 2) = cr;		// red
					} else if (vinfo.bits_per_pixel == 16) {
						int b = cb;	// blue
						int g = cg;	// green
						int r = cr;	// red
						unsigned short int t = r<<11 | g << 5 | b;
						*((unsigned short int*)(fb_addr + location)) = t;
					}
				// color level 2
				} else if(y<top2) {
					cr=255;
					cg=128;
					if (vinfo.bits_per_pixel == 32) {
						*(fb_addr + location) = cb;		// blue
						*(fb_addr + location + 1) = cg;		// green
						*(fb_addr + location + 2) = cr;		// red
						*(fb_addr + location + 3) = blended_t;		// transparency
					} else if (vinfo.bits_per_pixel == 24) {
						*(fb_addr + location) = cb;		// blue
						*(fb_addr + location + 1) = cg;		// green
						*(fb_addr + location + 2) = cr;		// red
					} else if (vinfo.bits_per_pixel == 16) {
						int b = cb;	// blue
						int g = cg;	// green
						int r = cr;	// red
						unsigned short int t = r<<11 | g << 5 | b;
						*((unsigned short int*)(fb_addr + location)) = t;
					}
				//color level 3
				} else if(y<top3) {
					cr=255;
					cg=0;
					if (vinfo.bits_per_pixel == 32) {
						*(fb_addr + location) = cb;		// blue
						*(fb_addr + location + 1) = cg;		// green
						*(fb_addr + location + 2) = cr;		// red
						*(fb_addr + location + 3) = blended_t;		// transparency
					} else if (vinfo.bits_per_pixel == 24) {
						*(fb_addr + location) = cb;		// blue
						*(fb_addr + location + 1) = cg;		// green
						*(fb_addr + location + 2) = cr;		// red
					} else if (vinfo.bits_per_pixel == 16) {
						int b = cb;	// blue
						int g = cg;	// green
						int r = cr;	// red
						unsigned short int t = r<<11 | g << 5 | b;
						*((unsigned short int*)(fb_addr + location)) = t;
					}
				} /*else {
					if (vinfo.bits_per_pixel == 32) {
						*(fb_addr + location) = blended_b;
						*(fb_addr + location + 1) = blended_g;
						*(fb_addr + location + 2) = blended_r;
						*(fb_addr + location + 3) = blended_t;
					} else if (vinfo.bits_per_pixel == 24) {
						*(fb_addr + location) = blended_b;		// blue
						*(fb_addr + location + 1) = blended_g;		// green
						*(fb_addr + location + 2) = blended_r;		// red
					} else if (vinfo.bits_per_pixel == 16) {
						int b = blended_b;
						int g = blended_g;
						int r = blended_r;
						unsigned short int t = r<<11 | g << 5 | b;
						*((unsigned short int*)(fb_addr + location)) = t;
					}
				}*/
			// over limit, paint black
			} else {
				if (vinfo.bits_per_pixel == 32) {
					*(fb_addr + location) = blended_b;
					*(fb_addr + location + 1) = blended_g;
					*(fb_addr + location + 2) = blended_r;
					*(fb_addr + location + 3) = blended_t;
				} else if (vinfo.bits_per_pixel == 24) {
					*(fb_addr + location) = blended_b;
					*(fb_addr + location + 1) = blended_g;
					*(fb_addr + location + 2) = blended_r;
				} else if (vinfo.bits_per_pixel == 16) {
					int b = blended_b;
					int g = blended_g;
					int r = blended_r;
					unsigned short int t = r<<11 | g << 5 | b;
					*((unsigned short int*)(fb_addr + location)) = t;
				}
			}
		}
}

// Draw a colored line
void draw_line(int value, int step, int cr, unsigned char cg, unsigned char cb, unsigned char ct, float alpha) {
	int x = 0, y = 0;
	long int location = 0;

	get_render_info();
	save_background();
	
	// Determine the correct resolution at runtime
	int width, height;
	if (miyoo_v4_mode == 1) {
		width = 640;
		height = 480;
	} else if (miyoo_v4_mode == 2) {
		width = 752;
		height = 560;
	} else {
		width = vinfo.xres;
		height = vinfo.yres;
	}
	
	int limit=value*height/step;

	for (y = 0; y < height; y++)
		for (x = width-6; x < width; x++) {
			location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * width*(vinfo.bits_per_pixel/8);
			
			// Calculate blended color
			unsigned char blended_b = (1 - alpha) * *(fb_addr + location) + alpha * cb;
			unsigned char blended_g = (1 - alpha) * *(fb_addr + location + 1) + alpha * cg;
			unsigned char blended_r = (1 - alpha) * *(fb_addr + location + 2) + alpha * cr;
			unsigned char blended_t = (1 - alpha) * *(fb_addr + location + 3) + alpha * ct;
			
			// if below limit, paint color
			if(y<limit) {
				if (vinfo.bits_per_pixel == 32) {
					*(fb_addr + location) = cb;		// blue
					*(fb_addr + location + 1) = cg;		// green
					*(fb_addr + location + 2) = cr;		// red
					*(fb_addr + location + 3) = blended_t;		// transparency
				} else if (vinfo.bits_per_pixel == 24) {
					*(fb_addr + location) = cb;		// blue
					*(fb_addr + location + 1) = cg;		// green
					*(fb_addr + location + 2) = cr;		// red
				} else if (vinfo.bits_per_pixel == 16) {
					int b = cb;	// blue
					int g = cg;	// green
					int r = cr;	// red
					unsigned short int t = r<<11 | g << 5 | b;
					*((unsigned short int*)(fb_addr + location)) = t;
				}
			// over limit, paint black
			} else {
				if (vinfo.bits_per_pixel == 32) {
					*(fb_addr + location) = blended_b;
					*(fb_addr + location + 1) = blended_g;
					*(fb_addr + location + 2) = blended_r;
					*(fb_addr + location + 3) = blended_t;
				} else if (vinfo.bits_per_pixel == 24) {
					*(fb_addr + location) = blended_b;
					*(fb_addr + location + 1) = blended_g;
					*(fb_addr + location + 2) = blended_r;
				} else if (vinfo.bits_per_pixel == 16) {
					int b = blended_b;
					int g = blended_g;
					int r = blended_r;
					unsigned short int t = r<<11 | g << 5 | b;
					*((unsigned short int*)(fb_addr + location)) = t;
				}
			}
		}
}

// Draw a black line
/*void clear_line() {
	int x = 0, y = 0;
	long int location = 0;

	get_render_info();
	
	// Check Miyoo v4 mode
    int miyoo_v4_mode = get_miyoo_v4();

    // Determine the correct resolution at runtime
    int width, height;
    if (miyoo_v4_mode == 1) {
        width = 640;
        height = 480;
    } else if (miyoo_v4_mode == 2) {
        width = 752;
        height = 560;
    } else {
        width = vinfo.xres;
        height = vinfo.yres;
    }
	
	for (y = 0; y < height; y++)
		for (x = width-6; x < width; x++) {
			location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * width*(vinfo.bits_per_pixel/8);
			
			if (vinfo.bits_per_pixel == 32) {
				*(fb_addr + location) = 0;	// black
				*(fb_addr + location + 1) = 0;	// black
				*(fb_addr + location + 2) = 0;	// black
				*(fb_addr + location + 3) = 0;	// transparency
			} else if (vinfo.bits_per_pixel == 24) {
				*(fb_addr + location) = 0;		// black
				*(fb_addr + location + 1) = 0;		// black
				*(fb_addr + location + 2) = 0;		// black
			} else if (vinfo.bits_per_pixel == 16) {
				int b = 0;	// black
				int g = 0;	// black
				int r = 0;	// black
				unsigned short int t = r<<11 | g << 5 | b;
				*((unsigned short int*)(fb_addr + location)) = t;
			}
		}
}*/

// Close framebuffer memory
void close_framebuffer() {
	if(fb_addr) {
		munmap(fb_addr, finfo.smem_len);
		fb_addr = NULL;
	}
	if(fb_fd) {
		close(fb_fd);
		fb_fd = -1;
	}
}

// Thread to draw the osd info
static void *osd_thread(void *param) {
	osd_running=1;
	gettimeofday(&osd_timer, NULL);
	if(miyoo_v4_mode == -1)
		miyoo_v4_mode=get_miyoo_v4();
	init_framebuffer();
	save_background();

	float elapsed;
	struct timeval now;
	do {
		switch(osd_item) {
			case OSD_VOLUME:
				draw_multiline(osd_volume,VOLUME_STEPS,54,60,70,0);
				break;
			case OSD_BRIGHTNESS:
				draw_line(osd_brightness,BRIGHTNESS_STEPS,255,255,128,0,0);
				break;
		}
		usleep(1);
		gettimeofday(&now, NULL);
		elapsed=(now.tv_sec - osd_timer.tv_sec) * 1000.0f + (now.tv_usec - osd_timer.tv_usec) / 1000.0f;
	} while(elapsed<3000);

	//clear_line();
	restore_background();
	close_framebuffer();
	osd_item=OSD_NONE;
	osd_running=0;
	return 0;
}

// Create the osd thread or update showing time
void osd_show(int item) {
	osd_item=item;
	if(!osd_running) {
		pthread_create(&thread_id, NULL, osd_thread, NULL);
	} else {
		gettimeofday(&osd_timer, NULL);
	}
}
