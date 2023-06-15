#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <linux/input.h>
#include <linux/fb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "cJSON.h"

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

//	Button Defines
#define	BUTTON_MENU		KEY_ESC
#define BUTTON_POWER		KEY_POWER
#define BUTTON_SELECT		KEY_RIGHTCTRL
#define BUTTON_START		KEY_ENTER
#define	BUTTON_L1		KEY_E
#define	BUTTON_R1		KEY_T
#define	BUTTON_L2		KEY_TAB
#define	BUTTON_R2		KEY_BACKSPACE
#define BUTTON_A		KEY_SPACE
#define BUTTON_B		KEY_LEFTCTRL
#define BUTTON_X		KEY_LEFTSHIFT
#define BUTTON_Y		KEY_LEFTALT
#define BUTTON_UP		KEY_UP
#define BUTTON_DOWN		KEY_DOWN
#define BUTTON_LEFT		KEY_LEFT
#define BUTTON_RIGHT	KEY_RIGHT
#define BUTTON_VOLUMEUP	KEY_VOLUMEUP
#define BUTTON_VOLUMEDOWN	KEY_VOLUMEDOWN

#define CPUSAVE "/mnt/SDCARD/.simplemenu/cpu.sav"

#define BRIMAX		10
#define BRIMIN		1

// for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

// Set Volume (Raw)
#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE 0x4008690d

// Global Variables
static struct input_event	ev;
static int input_fd = 0;
//static uint32_t *fb_addr;
//static int fb_fd;
//static uint8_t *fbofs;
//static struct fb_fix_screeninfo finfo;
//static struct fb_var_screeninfo vinfo;
//static uint32_t stride, bpp;
//static uint8_t *savebuf;


char* load_file(char const* path) {
	char* buffer = 0;
	long length = 0;
	FILE * f = fopen(path, "rb"); //was "rb"
	
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = (char*) malloc((length+1)*sizeof(char));
		if (buffer) {
			fread(buffer, sizeof(char), length, f);
		}
		fclose(f);
	}
	buffer[length] = '\0';
	
	return buffer;
}

// Increments between -60 and -3
int setVolumeRaw(int volume, int add) {
	int recent_volume = 0;
	int fd = open("/dev/mi_ao", O_RDWR);
	if (fd >= 0) {
		int buf2[] = {0, 0};
		uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
		ioctl(fd, MI_AO_GETVOLUME, buf1);
		recent_volume = buf2[1];
		if (add) {
			buf2[1] += add;
			if (buf2[1] > -3) buf2[1] = -3;
			else if (buf2[1] < -60) buf2[1] = -60;
		} else buf2[1] = volume;
		if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
		close(fd);
	}

	// Increase/Decrease Volume
	cJSON* request_json = NULL;
	cJSON* itemVol;
	cJSON* itemMute;
	
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[128];
			int flash_detected = 0;
			
			while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
				if (strstr(buffer, "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1") != NULL) {
					flash_detected = 1;
					break;
				}
			}
			
			pclose(pipe);
			
			if (flash_detected) {
				settings_file = "/mnt/SDCARD/system.json";
			} else {
				settings_file = "/appconfigs/system.json";
			}
		}
	}
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemVol = cJSON_GetObjectItem(request_json, "vol");
	itemMute = cJSON_GetObjectItem(request_json, "mute");
	int vol = cJSON_GetNumberValue(itemVol);
	if (add == 3 && vol < 20) vol++;
	if (add == -3 && vol > 0) vol--;
	if (add != 0) {
		cJSON_SetNumberValue(itemVol, vol);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
	}
	if (vol == 0) {
		cJSON_SetNumberValue(itemMute, 1);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
		int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, 1};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
	}
	} else if (vol > 0) {
		cJSON_SetNumberValue(itemMute, 0);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
		int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
	}
	}	
	
	cJSON_Delete(request_json);
	free(request_body);
	
	return recent_volume;
}

// Increments between 0 and 20
int setVolume(int volume, int add) {
	int recent_volume = 0;
	int rawVolumeValue = 0;
	int rawAdd = 0;
	
	rawVolumeValue = (volume * 3) - 60;
	rawAdd = (add * 3);
	
	recent_volume = setVolumeRaw(rawVolumeValue, rawAdd);
	return (int)((recent_volume/3)+20);
}

// Increase/Decrease Brightness
void modifyBrightness(int inc) {
	cJSON* request_json = NULL;
	cJSON* itemBrightness;
	
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[128];
			int flash_detected = 0;
			
			while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
				if (strstr(buffer, "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1") != NULL) {
					flash_detected = 1;
					break;
				}
			}
			
			pclose(pipe);
			
			if (flash_detected) {
				settings_file = "/mnt/SDCARD/system.json";
			} else {
				settings_file = "/appconfigs/system.json";
			}
		}
	}
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemBrightness = cJSON_GetObjectItem(request_json, "brightness");
	int brightness = cJSON_GetNumberValue(itemBrightness);
	
	if (inc == 1 && brightness < BRIMAX) brightness++;
	if (inc == -1 && brightness > BRIMIN) brightness--;
	if (inc != 0) {
		cJSON_SetNumberValue(itemBrightness, brightness);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
	}
	
	cJSON_Delete(request_json);
	free(request_body);
	
	int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%d", brightness * 10);
		close(fd);
	}
}

void setcpu(int cpu) {
	if (cpu == 0) {
		FILE *file;
		char cpuValue[10];
		
		file = fopen(CPUSAVE, "r");
		fgets(cpuValue, sizeof(cpuValue), file);
		fclose(file);
		
		FILE *cpuFile0 = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "w");
		FILE *cpuFile1 = fopen("/sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq", "w");
		
		fprintf(cpuFile0, "%s", cpuValue);
		fprintf(cpuFile1, "%s", cpuValue);
		
		fclose(cpuFile0);
		fclose(cpuFile1);
	} else if (cpu == 1) {
		system("echo 400000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
		system("echo 400000 > /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq");
	}
}

void setmute(int mute) {
	cJSON* request_json = NULL;
	cJSON* itemMute;
	
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[128];
			int flash_detected = 0;
			
			while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
				if (strstr(buffer, "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1") != NULL) {
					flash_detected = 1;
					break;
				}
			}
			
			pclose(pipe);
			
			if (flash_detected) {
				settings_file = "/mnt/SDCARD/system.json";
			} else {
				settings_file = "/appconfigs/system.json";
			}
		}
	}
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemMute = cJSON_GetObjectItem(request_json, "mute");
	
	if (mute == 1 ){
		cJSON_SetNumberValue(itemMute, mute);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
		int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, mute};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
    }
	}
	if (mute == 0) {
		cJSON_SetNumberValue(itemMute, mute);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
		int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, mute};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
    }
	}
	
	cJSON_Delete(request_json);
	free(request_body);
}

void sethibernate(int hibernate) {
	cJSON* request_json = NULL;
	cJSON* itemHibernate;
	
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[128];
			int flash_detected = 0;
			
			while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
				if (strstr(buffer, "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1") != NULL) {
					flash_detected = 1;
					break;
				}
			}
			
			pclose(pipe);
			
			if (flash_detected) {
				settings_file = "/mnt/SDCARD/system.json";
			} else {
				settings_file = "/appconfigs/system.json";
			}
		}
	}
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemHibernate = cJSON_GetObjectItem(request_json, "hibernate");
	
	if (hibernate == 1 ){
		cJSON_SetNumberValue(itemHibernate, hibernate);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
	}
	
	if (hibernate == 0) {
		cJSON_SetNumberValue(itemHibernate, hibernate);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
	}
	
	cJSON_Delete(request_json);
	free(request_body);
}

void restorevolume(int valuevol) {
	cJSON* request_json = NULL;
	cJSON* itemVol;
	cJSON* itemFix;
	
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[128];
			int flash_detected = 0;
			
			while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
				if (strstr(buffer, "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1") != NULL) {
					flash_detected = 1;
					break;
				}
			}
			
			pclose(pipe);
			
			if (flash_detected) {
				settings_file = "/mnt/SDCARD/system.json";
			} else {
				settings_file = "/appconfigs/system.json";
			}
		}
	}
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemVol = cJSON_GetObjectItem(request_json, "vol");
	itemFix = cJSON_GetObjectItem(request_json, "audiofix");
	int volumesave = cJSON_GetNumberValue(itemVol);
	int audiofix = cJSON_GetNumberValue(itemFix);
	if (valuevol == 0) {
		if (access("/tmp/volsav", F_OK) == 0) {
			if (audiofix == 1) {
				int fd = open("/dev/mi_ao", O_RDWR);
				if (fd >= 0) {
					int buf2[] = {0, -60};
					uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
					ioctl(fd, MI_AO_SETVOLUME, buf1);
					close(fd);
				}
			}
			int set = 0;
			set = 40; //tinymix work in 100-40 // 0-(-60)
			char command[100];
			sprintf(command, "tinymix set 6 %d", set);
			system(command);
			
		} else { 
			system("touch /tmp/volsav");
			char command[100];
			sprintf(command, "echo %d > /tmp/volsav", volumesave);
			system(command);
			if (audiofix == 1) {
				int fd = open("/dev/mi_ao", O_RDWR);
				if (fd >= 0) {
					int buf2[] = {0, -60};
					uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
					ioctl(fd, MI_AO_SETVOLUME, buf1);
					close(fd);
				}
			}
			int set = 0;
			set = 40; //tinymix work in 100-40 // 0-(-60)
			char command2[100];
			sprintf(command2, "tinymix set 6 %d", set);
			system(command2);
		}
	} else if (valuevol == 1){
		if (access("/tmp/volsav", F_OK) == 0) {
			int set = 0;
			set = ((volumesave*3)+40); //tinymix work in 100-40 // 0-(-60)
			char command[100];
			sprintf(command, "tinymix set 6 %d", set);
			system(command);
			if (audiofix == 1) {
				int recent_volume = 0;
				int add;
				int fd = open("/dev/mi_ao", O_RDWR);
				if (fd >= 0) {
					int buf2[] = {0, 0};
					buf2[1] = (volumesave * 3) - 60;
					add = volumesave * 3;
					uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
					ioctl(fd, MI_AO_GETVOLUME, buf1);
					recent_volume = buf2[1];
					if (add) {
						buf2[1] += add;
						if (buf2[1] > -3) buf2[1] = -3;
						else if (buf2[1] < -60) buf2[1] = -60;
					} else buf2[1] = (volumesave * 3) - 60;
					if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
					close(fd);
				}
			}
		} else { 
			system("touch /tmp/volsav");
			char command[100];
			sprintf(command, "echo %d > /tmp/volsav", volumesave);
			system(command);
		}
	}
}

void display_init(void)
{
    // Open and mmap FB
    //fb_fd = open("/dev/fb0", O_RDWR);
    //ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    //fb_addr = (uint32_t *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE,
    //                           MAP_SHARED, fb_fd, 0);
}

void display_setScreen(int value) {
	//stride = finfo.line_length;
	//ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	//bpp = vinfo.bits_per_pixel / 8; // byte per pixel
	//fbofs = (uint8_t *)fb_addr + (vinfo.yoffset * stride);
	
	if (value == 0) {
		system("echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable");
    	// Save display area and clear
    	//if ((savebuf = (uint8_t *)malloc(DISPLAY_WIDTH * bpp * DISPLAY_HEIGHT))) {
        //	uint32_t i, ofss, ofsd;
        //	ofss = ofsd = 0;
        //	for (i = DISPLAY_HEIGHT; i > 0;
        //    	 i--, ofss += stride, ofsd += DISPLAY_WIDTH * bpp) {
        //    	memcpy(savebuf + ofsd, fbofs + ofss, DISPLAY_WIDTH * bpp);
        //    	memset(fb_addr, 0, vinfo.xres * vinfo.yres * bpp);
        //	}
    	//}
	} else if (value == 1) {
		system("echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable");
		// Restore display area
    	//if (savebuf) {
        //	uint32_t i, ofss, ofsd;
        //	ofss = ofsd = 0;
        //	for (i = DISPLAY_HEIGHT; i > 0;
        //    	 i--, ofsd += stride, ofss += DISPLAY_WIDTH * bpp) {
        //    	memcpy(fbofs + ofsd, savebuf + ofss, DISPLAY_WIDTH * bpp);
        //	}
        //	free(savebuf);
        //	savebuf = NULL;
    	//}
	}
}

void keyinput_send(unsigned short code, signed int value)
{
	char cmd[100];
	sprintf(cmd, "/mnt/SDCARD/Koriki/bin/sendkeys %d %d", code, value);
	printf("Send keys: code=%d, value=%d\n", code, value);
	system(cmd);
	printf("Keys sent");
}

int main (int argc, char *argv[]) {
	input_fd = open("/dev/input/event0", O_RDONLY);
	
	//display_init();
	//display_setScreen(1);
	modifyBrightness(0);
	setcpu(0);
	sethibernate(0);
	setmute(0);
	setVolume(0,0);
	int volume = 0;
	int power_pressed_duration = 0;
	int sleep = 0;
	
	//READ Volume valor from system
	cJSON* request_json = NULL;
	cJSON* itemVol;
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[128];
			int flash_detected = 0;
			
			while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
				if (strstr(buffer, "[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1") != NULL) {
					flash_detected = 1;
					break;
				}
			}
			
			pclose(pipe);
			
			if (flash_detected) {
				settings_file = "/mnt/SDCARD/system.json";
			} else {
				settings_file = "/appconfigs/system.json";
			}
		}
	}
	
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemVol = cJSON_GetObjectItem(request_json, "vol");
	int vol = cJSON_GetNumberValue(itemVol);
	volume = vol;
	
	// Main Loop
	register uint32_t val;
	register uint32_t menu_pressed = 0;
	register uint32_t power_pressed = 0;
	int repeat_power = 0;
	int shutdown = 0;
	uint32_t repeat = 0;
	while (read(input_fd, &ev, sizeof(ev)) == sizeof(ev)) {
		val = ev.value;
		if ((ev.type != EV_KEY) || (val > REPEAT)) continue;
		switch (ev.code) {
			case BUTTON_POWER:
				if (val == PRESSED) {
					power_pressed = val;
					power_pressed_duration = 0;
				} else if (val == RELEASED && power_pressed) {
					if (power_pressed_duration < 5) { // Short press
						if (sleep == 0) {
							setmute(1);
							sethibernate(1);
							//restorevolume(0);
							setcpu(1);
							keyinput_send(1, 1);
							keyinput_send(1, 2);
							display_setScreen(0); // Turn screen back off
							power_pressed = 0;
							repeat_power = 0;
							sleep = 1;
						} else if (sleep == 1) {
							setmute(0);
							sethibernate(0);
							//restorevolume(1);
							setcpu(0);
							keyinput_send(1, 1);
							keyinput_send(1, 2);
							usleep(1000);
							display_setScreen(1); // Turn screen back on
							keyinput_send(1, 1);
							power_pressed = 0;
            				repeat_power = 0;
							sleep = 0;
						}
					}
					// Long press is handled by the existing code
				} else if (val == REPEAT) {
					if (repeat_power >= 20) {
						shutdown = 1;
					}
					repeat_power++;
				}
				break;
			case BUTTON_MENU:
				if (sleep == 1) {
					if (val != REPEAT) menu_pressed = val;
				} else if (sleep == 0) { 
					if (val != REPEAT) menu_pressed = val;
				}
				break;
			case BUTTON_UP:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				} else if (sleep == 0) {
					if (val == REPEAT) {
						// Adjust repeat speed to 1/2
						val = repeat;
						repeat ^= PRESSED;
					} else {
						repeat = 0;
					}
					if (val == PRESSED && menu_pressed) {
						// Increase brightness
						modifyBrightness(1);
					}
				}
				break;
			case BUTTON_DOWN:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				} else if (sleep == 0) {
					if (val == REPEAT) {
						// Adjust repeat speed to 1/2
						val = repeat;
						repeat ^= PRESSED;
					} else {
						repeat = 0;
					}
					if (val == PRESSED && menu_pressed) {
						// Decrease brightness
						modifyBrightness(-1);
					}
				}
				break;
			case BUTTON_LEFT:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_RIGHT:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_A:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_B:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_Y:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_X:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_START:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_SELECT:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_L1:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_R1:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_L2:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
				break;
			case BUTTON_R2:
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				}
			case BUTTON_VOLUMEUP:
				// Increase volume
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				} else if (sleep == 0) {
					setVolume(volume, 1);
				}
				break;
			case BUTTON_VOLUMEDOWN:
				// Decrease volume
				if (sleep == 1) {
					if (val == PRESSED && menu_pressed) {}
				} else if (sleep == 0) {
					setVolume(volume, -1);
				}
				break;
			default:
				break;
		}
		
		if (shutdown) {
			power_pressed = 0;
			if (access("/customer/app/axp_test", F_OK) == 0)
				system("killall main; killall updater; killall audioserver; killall audioserver.plu; killall retroarch; killall simplemenu; killall batmon; killall keymon; /etc/init.d/K00_Sys; sync; sleep 5; poweroff");
			else
				system("killall main; killall updater; killall audioserver; killall audioserver.min; killall retroarch; killall simplemenu; killall batmon; killall keymon; /etc/init.d/K00_Sys; sync; sleep 5; reboot");
			while (1) pause();
		}
	}
	exit(EXIT_FAILURE);
}
