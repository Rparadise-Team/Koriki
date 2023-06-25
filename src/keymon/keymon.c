#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cJSON.h"

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
#define GOVSAVE "/mnt/SDCARD/.simplemenu/governor.sav"

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
	
	// Increase/Decrease Volume
	cJSON* request_json = NULL;
	cJSON* itemVol;
	cJSON* itemMute;
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	
	itemVol = cJSON_GetObjectItem(request_json, "vol");
	itemMute = cJSON_GetObjectItem(request_json, "mute");
	int vol = cJSON_GetNumberValue(itemVol);
	
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
	
	if (vol == 0) {
		cJSON_SetNumberValue(itemMute, 1);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
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
    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};

        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
	}
	}	
	
	if (add == 3 && vol < 20) vol++;
	if (add == -3 && vol > 0) vol--;
	if (add != 0) {
		cJSON_SetNumberValue(itemVol, vol);
		FILE *file = fopen(settings_file, "w");
		char *test = cJSON_Print(request_json);
		fputs(test, file);
		fclose(file);
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

int getVolume() {
	int recent_volume = 0;
	int fd = open("/dev/mi_ao", O_RDWR);
	
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
	
	// get Volume level
	cJSON* request_json = NULL;
	cJSON* itemVol;
	cJSON* itemMute;
	
	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	
	itemVol = cJSON_GetObjectItem(request_json, "vol");
	itemMute = cJSON_GetObjectItem(request_json, "mute");
	int vol = cJSON_GetNumberValue(itemVol);
	int mute = cJSON_GetNumberValue(itemMute);

	
	if (mute == 0) {
		if (fd >= 0) {
			int buf2[] = {0, 0};
			uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
			ioctl(fd, MI_AO_GETVOLUME, buf1);
			recent_volume = ((vol * 3) - 60);
			buf2[1] = recent_volume;
			ioctl(fd, MI_AO_SETVOLUME, buf1);
			close(fd);
			}
		
		if (vol > 0) {
	    	if (fd >= 0) {
	        	int buf2[] = {0, 0};
	        	uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
						
   		     	ioctl(fd, MI_AO_SETMUTE, buf1);
	        	close(fd);
			}
		}
	} else if (mute == 1) {
		if (fd >= 0) {
	        	int buf2[] = {0, 1};
	        	uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
						
   		     	ioctl(fd, MI_AO_SETMUTE, buf1);
	        	close(fd);
			}
		}
	
	return 0;
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
		FILE *file0;
		FILE *file1;
		char cpuValue[10];
		char govValue[20];
		
		file0 = fopen(CPUSAVE, "r");
		 if (file0 == NULL) {
			 file0 = fopen(CPUSAVE, "w");
			 fprintf(file0, "%d", 1200000);
			 fclose(file0);
			 file0 = fopen(CPUSAVE, "r");
		 }
		
		file1 = fopen(GOVSAVE, "r");
		 if (file1 == NULL) {
			 file1 = fopen(GOVSAVE, "w");
			 fprintf(file1, "ondemand");
			 fclose(file1);
			 file1 = fopen(GOVSAVE, "r");
        }
		
		fgets(cpuValue, sizeof(cpuValue), file0);
		fclose(file0);
		
		fgets(govValue, sizeof(govValue), file1);
		fclose(file1);
		
		FILE *cpuFile = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "w");
		fprintf(cpuFile, "%s", cpuValue);
		fclose(cpuFile);
			 
		FILE *govFile = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "w");
		fprintf(govFile, "%s", govValue);
		fclose(govFile);
	} else if (cpu == 1) {
		system("cp /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor /mnt/SDCARD/.simplemenu/governor.sav");
		system("echo powersave > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
		system("echo 400000 > /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq");
		system("sync");
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
	int volumesave = cJSON_GetNumberValue(itemVol);
	int recent_volume = 0;
	int add = 0;
	int fd = open("/dev/mi_ao", O_RDWR);
	
	if (valuevol == 0) {
		if (access("/tmp/volsav", F_OK) == 0) {
			if (fd >= 0) {
				int buf2[] = {0, -60};
				uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
				ioctl(fd, MI_AO_SETVOLUME, buf1);
				close(fd);
			}
		} else { 
			system("touch /tmp/volsav");
			char command[100];
			sprintf(command, "echo %d > /tmp/volsav", volumesave);
			system(command);

			if (fd >= 0) {
				int buf2[] = {0, -60};
				uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
				ioctl(fd, MI_AO_SETVOLUME, buf1);
				close(fd);
			}
		}
	} else if (valuevol == 1){
		if (access("/tmp/volsav", F_OK) == 0) {
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
		} else { 
			system("touch /tmp/volsav");
			char command[100];
			sprintf(command, "echo %d > /tmp/volsav", volumesave);
			system(command);
		}
	}
}

void keyinput_send(int code, int mode)
{
    struct input_event events[1];

    events[0].type = EV_KEY;
    events[0].code = code;
    events[0].value = mode;

    int input_fd = open("/dev/input/event0", O_WRONLY);
    if (input_fd == -1) {
        perror("Failed to open input device");
        return;
    }

    ssize_t bytes_written = write(input_fd, events, sizeof(events));
    if (bytes_written == -1) {
        perror("Failed to write to input device");
    }

    fsync(input_fd);
    close(input_fd);
}

void keymulti_send(int code1, int mode1, int code2, int mode2) 
{
    int num_events = 2;
    struct input_event *events = (struct input_event*)malloc(num_events * sizeof(struct input_event));

    events[0].type = EV_KEY;
    events[0].code = code1;
    events[0].value = mode1;

    events[1].type = EV_KEY;
    events[1].code = code2;
    events[1].value = mode2;

    int input_fd = open("/dev/input/event0", O_WRONLY);
    if (input_fd == -1) {
        perror("Failed to open input device");
        free(events);
        return;
    }

    for (int i = 0; i < num_events; i++) {
        ssize_t bytes_written = write(input_fd, &events[i], sizeof(events[i]));
        if (bytes_written == -1) {
            perror("Failed to write to input device");
            free(events);
            close(input_fd);
            return;
        }
    }

    fsync(input_fd);
    close(input_fd);
    free(events);
}

int isRetroarchRunning()
{
	FILE *fp;
	char buffer[128];
	const char *cmd = "pgrep retroarch";
    
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Error al ejecutar el comando 'pgrep retroarch'\n");
		return 0;
	}
    
	if (fgets(buffer, sizeof(buffer), fp) != NULL) {
		pclose(fp);
		return 1;
	}
	
	pclose(fp);
	return 0;
}

void display_setScreen(int value) {
	if (value == 0) {  // enter in savepower mode
		if (isRetroarchRunning() == 1) {
			system("pkill -STOP retroarch");
		}
		system("echo 4 > /sys/class/gpio/export");
		system("echo out > /sys/class/gpio/gpio4/direction");
		system("echo 0 > /sys/class/gpio/gpio4/value");
		system("echo 0 > /sys/class/pwm/pwmchip0/pwm0/enable");
		system("echo 0 > /sys/module/gpio_keys_polled/parameters/button_enable");
		system("echo GUI_SHOW 0 off > /proc/mi_modules/fb/mi_fb0");
	} else if (value == 1) {  // exit for savepower mode
		system("echo GUI_SHOW 0 on > /proc/mi_modules/fb/mi_fb0");
		system("echo 1 > /sys/module/gpio_keys_polled/parameters/button_enable");
		system("echo 1 > /sys/class/gpio/gpio4/value");
		system("echo 4 > /sys/class/gpio/unexport");
		system("echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable");
		usleep(20000);
		if (isRetroarchRunning() == 1) {
			system("pkill -CONT retroarch");
		}
	}
}

int main (int argc, char *argv[]) {
	input_fd = open("/dev/input/event0", O_RDONLY);
	
	getVolume();
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
							display_setScreen(0); // Turn screen back off
							setmute(1);
							sethibernate(1);
							setcpu(1);
							power_pressed = 0;
							repeat_power = 0;
							sleep = 1;
						} else if (sleep == 1) {
							setmute(0);
							sethibernate(0);
							setcpu(0);
							display_setScreen(1); // Turn screen back on
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
				if (val != REPEAT) menu_pressed = val;
				break;
			case BUTTON_UP:
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
				break;
			case BUTTON_DOWN:
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
				break;
			case BUTTON_VOLUMEUP:
				// Increase volume
				setVolume(volume, 1);
				break;
			case BUTTON_VOLUMEDOWN:
				// Decrease volume
				setVolume(volume, -1);
				break;
			default:
				break;
		}
		
		if (shutdown) {
			power_pressed = 0;
			if (access("/customer/app/axp_test", F_OK) == 0)
				system("killall main; killall updater; killall audioserver; killall audioserver.plu; killall retroarch; killall simplemenu; killall batmon; killall updater; killall keymon; /etc/init.d/K00_Sys; sync; sleep 5; umount -l /mnt/SDCARD; poweroff");
			else
				system("killall main; killall updater; killall audioserver; killall audioserver.min; killall retroarch; killall simplemenu; killall batmon; killall updater; killall keymon; /etc/init.d/K00_Sys; sync; sleep 5; umount -l /mnt/SDCARD; reboot");
			while (1) pause();
		}
	}
	exit(EXIT_FAILURE);
}
