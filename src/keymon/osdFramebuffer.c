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
#include "osdIcons.h"
#include "osdFramebuffer.h"

#define VOLUME_STEPS		69
#define BRIGHTNESS_STEPS	10

// Framebuffer data
int fb_fd = -1;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fb_addr = NULL;
char *fb_iconbackground = NULL;
int fb_lastframe=-1;
unsigned int fb_lastbuffersaved=-1;

// Miyoo model data
int miyoo_v4_mode = -1;

// OSD data
pthread_t thread_id;
int osd_running=0;
struct timeval osd_timer;
int osd_item=OSD_NONE;
int osd_volume;
int osd_headphones_connected = 0;
int osd_brightness;

// Variables para evitar redibujos innecesarios
static int last_volume = -1;
static int last_brightness = -1;

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

// Save bar background to memory (guarda todos los buffers)
void save_background_all_buffers() {
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

	// Calcula el tamaño total para guardar todos los buffers
	int num_buffers = vinfo.yres_virtual / height;
	int buffer_size = OSD_ICON_WIDTH * OSD_ICON_HEIGHT * (vinfo.bits_per_pixel / 8);
	
	// Get memory to save bar background de todos los buffers
	if(fb_iconbackground == NULL)
		fb_iconbackground = (char*)malloc(buffer_size * num_buffers);

	get_render_info();

	if(fb_iconbackground) {
		int x, y, buffer_idx;
		int margin = 10;
		long int location = 0, idx_background = 0;
		int bytes = vinfo.bits_per_pixel / 8;

		// Guarda el fondo de cada buffer
		for(buffer_idx = 0; buffer_idx < num_buffers; buffer_idx++) {
			int y_offset = buffer_idx * height;
			
			if(vinfo.bits_per_pixel == 32) {
				for (y = height - (OSD_ICON_HEIGHT + margin); y < (height - margin); y++)
					for (x = width - (OSD_ICON_WIDTH + margin); x < (width - margin); x++) {
						location = (x + vinfo.xoffset) * bytes + (y + y_offset) * width * bytes;
						*(unsigned long *)(fb_iconbackground + idx_background) = *(unsigned long *)(fb_addr + location);
						idx_background += 4;
					}
			} else if(vinfo.bits_per_pixel == 16) {
				for (y = height - (OSD_ICON_HEIGHT + margin); y < (height - margin); y++)
					for (x = width - (OSD_ICON_WIDTH + margin); x < (width - margin); x++) {
						location = (x + vinfo.xoffset) * bytes + (y + y_offset) * width * bytes;
						*(unsigned int *)(fb_iconbackground + idx_background) = *(unsigned int *)(fb_addr + location);
						idx_background += 2;
					}
			}
		}
	}
}

// Restore saved background to all buffers
void restore_background_all_buffers() {
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

	if(fb_iconbackground) {
		int x, y, buffer_idx;
		int margin = 10;
		long int location = 0, idx_background = 0;
		int bytes = vinfo.bits_per_pixel / 8;
		int num_buffers = vinfo.yres_virtual / height;

		// Restaura el fondo en cada buffer
		for(buffer_idx = 0; buffer_idx < num_buffers; buffer_idx++) {
			int y_offset = buffer_idx * height;
			
			if(vinfo.bits_per_pixel == 32) {
				for (y = height - (OSD_ICON_HEIGHT + margin); y < (height - margin); y++)
					for (x = width - (OSD_ICON_WIDTH + margin); x < (width - margin); x++) {
						location = (x + vinfo.xoffset) * bytes + (y + y_offset) * width * bytes;
						*(unsigned long *)(fb_addr + location) = *(unsigned long *)(fb_iconbackground + idx_background);
						idx_background += 4;
					}
			} else if(vinfo.bits_per_pixel == 16) {
				for (y = height - (OSD_ICON_HEIGHT + margin); y < (height - margin); y++)
					for (x = width - (OSD_ICON_WIDTH + margin); x < (width - margin); x++) {
						location = (x + vinfo.xoffset) * bytes + (y + y_offset) * width * bytes;
						*(unsigned int *)(fb_addr + location) = *(unsigned int *)(fb_iconbackground + idx_background);
						idx_background += 2;
					}
			}
		}
	}
}

// Calculate percent of icon to show, return an intermediate value of OSD_ICON_WIDTH 
int get_icon_cutoff() {
	switch(osd_item) {
		case OSD_VOLUME:
			return osd_volume*OSD_ICON_HEIGHT/69;	// volume values are between 0-69
		case OSD_BRIGHTNESS:
			return osd_brightness*OSD_ICON_HEIGHT/10;	// bright values are between 1-10
		default:
			return 0;
	}
}

// Nueva función: dibuja el icono en el buffer ACTUAL (yoffset actual)
void draw_icon_at_current_buffer() {
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

	int x, y;
	long int location = 0;
	int limit = get_icon_cutoff();
	int bytes = vinfo.bits_per_pixel / 8;
	int tintcolor = 2;
	
	char *icon = NULL;
	switch(osd_item) {
		case OSD_VOLUME:
			if(osd_headphones_connected) {
				if(osd_volume == 0)
					icon = volume_headmute_icon;
				else
					icon = volume_head_icon;
			} else {
				if(osd_volume == 0)
					icon = volume_mute_icon;
				else
					icon = volume_icon;
			}
			if(osd_volume >= 60)
				tintcolor = 4;
			else if(osd_volume >= 54)
				tintcolor = 3;
			break;
		case OSD_BRIGHTNESS:
			icon = brightness_icon;
			break;
		default:
			return;
	}

	int selectcolor = -1;
	int idx_icon = 0;
	int margin = 10;
	
	// Dibuja en el buffer ACTUAL usando vinfo.yoffset
	if(vinfo.bits_per_pixel == 32) {
		for (y = height - (OSD_ICON_HEIGHT + margin); y < (height - margin); y++) {
			for (x = width - (OSD_ICON_WIDTH + margin); x < (width - margin); x++) {
				location = (x + vinfo.xoffset) * bytes + (y + vinfo.yoffset) * width * bytes;
				char value = *(icon + idx_icon);
				
				switch(value) {
					case 0:
						if(y - (height - (OSD_ICON_HEIGHT + margin)) > limit)
							selectcolor = 0;
						else
							selectcolor = tintcolor;
						break;
					case 1:
						selectcolor = 1;
						break;
					case 2:
						selectcolor = 4;
						break;
					default:
						selectcolor = -1;
						break;
				}
				
				if(selectcolor >= 0) {
					*(fb_addr + location) = iconcolor[selectcolor][2];
					*(fb_addr + location + 1) = iconcolor[selectcolor][1];
					*(fb_addr + location + 2) = iconcolor[selectcolor][0];
					*(fb_addr + location + 3) = 0;
				}
				idx_icon++;
			}
		}
	} else if(vinfo.bits_per_pixel == 16) {
		for (y = height - (OSD_ICON_HEIGHT + margin); y < (height - margin); y++) {
			for (x = width - (OSD_ICON_WIDTH + margin); x < (width - margin); x++) {
				location = (x + vinfo.xoffset) * bytes + (y + vinfo.yoffset) * width * bytes;
				char value = *(icon + idx_icon);
				
				switch(value) {
					case 0:
						if(y - (height - (OSD_ICON_HEIGHT + margin)) > limit)
							selectcolor = tintcolor;
						else
							selectcolor = 0;
						break;
					case 1:
						selectcolor = 1;
						break;
					case 2:
						selectcolor = 4;
						break;
					default:
						selectcolor = -1;
						break;
				}
				
				if(selectcolor >= 0) {
					char c = iconcolor[selectcolor][2] << 11 | iconcolor[selectcolor][1] << 5 | iconcolor[selectcolor][0];
					*(fb_addr + location) = c;
				}
				idx_icon++;
			}
		}
	}
}

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
	// free memory and reset pointer and last frame saved
	if(fb_iconbackground)
		free(fb_iconbackground);
	fb_iconbackground = NULL;
	fb_lastframe = -1;
	miyoo_v4_mode = -1;
}

// Thread to draw the osd info - VERSIÓN HÍBRIDA: guarda fondo + dibuja cada frame
static void *osd_thread(void *param) {
	osd_running = 1;
	gettimeofday(&osd_timer, NULL);
	
	if(miyoo_v4_mode == -1)
		miyoo_v4_mode = get_miyoo_v4();
	
	init_framebuffer();
	
	// Guarda el fondo de TODOS los buffers al inicio
	save_background_all_buffers();
	
	// Guarda los valores actuales
	last_volume = osd_volume;
	last_brightness = osd_brightness;
	
	float elapsed;
	struct timeval now;
	int last_yoffset = -1;
	
	// Loop principal - dibuja el icono en cada frame para overlay
	do {
		usleep(8333); // ~120fps para capturar cambios de buffer rápidamente
		
		get_render_info();
		
		// Detecta si cambió el buffer (nuevo frame)
		if((int)vinfo.yoffset != last_yoffset) {
			last_yoffset = (int)vinfo.yoffset;
			
			// Espera a que termine el blit del overlay (si existe)
			usleep(1000); // pequeña espera para asegurar que overlay terminó
			
			// Dibuja el icono directamente en el buffer actual
			draw_icon_at_current_buffer();
		}
		
		// Detecta si cambió el valor del volumen o brillo
		int value_changed = 0;
		if(osd_item == OSD_VOLUME && osd_volume != last_volume) {
			value_changed = 1;
			last_volume = osd_volume;
		} else if(osd_item == OSD_BRIGHTNESS && osd_brightness != last_brightness) {
			value_changed = 1;
			last_brightness = osd_brightness;
		}
		
		// Si cambió el valor, reinicia el timer
		if(value_changed) {
			gettimeofday(&osd_timer, NULL);
		}
		
		gettimeofday(&now, NULL);
		elapsed = (now.tv_sec - osd_timer.tv_sec) * 1000.0f + 
		          (now.tv_usec - osd_timer.tv_usec) / 1000.0f;
	} while(elapsed < 3000); // Mantiene visible 3 segundos
	
	// Al finalizar, RESTAURA el fondo guardado en todos los buffers
	restore_background_all_buffers();
	
	close_framebuffer();
	fb_lastbuffersaved = -1;
	osd_item = OSD_NONE;
	osd_running = 0;
	
	return 0;
}

// Create the osd thread or update showing time
void osd_show(int item) {
	osd_item = item;
	if(!osd_running) {
		pthread_create(&thread_id, NULL, osd_thread, NULL);
		pthread_setschedprio(thread_id, 1);	// priority = 1
	} else {
		gettimeofday(&osd_timer, NULL);
	}
}
