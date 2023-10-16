#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cJSON.h"

// Set Volume (Raw)
#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE 0x4008690d

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

int isAudiofixRunning()
{
	FILE *fp;
	char buffer[64];
	const char *cmd = "pgrep audioserver";
    
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Error al ejecutar el comando 'pgrep audioserver'\n");
		return 0;
	}
    
	if (fgets(buffer, sizeof(buffer), fp) != NULL) {
		pclose(fp);
		return 1;
	}
	
	pclose(fp);
	return 0;
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
			char buffer[64];
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
		if ( isAudiofixRunning() == 1) {
		if (fd >= 0) {
			int buf2[] = {0, 0};
			uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
			ioctl(fd, MI_AO_GETVOLUME, buf1);
			recent_volume = ((vol * 3) - 60);
			buf2[1] = recent_volume;
			ioctl(fd, MI_AO_SETVOLUME, buf1);
			close(fd);
			}
		} else if (isAudiofixRunning() == 0) {
			if (fd >= 0) {
			   int buf2[] = {0, 0};
			   uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
			   ioctl(fd, MI_AO_GETVOLUME, buf1);
			   recent_volume = ((vol * 3) - 60);
			   buf2[1] = recent_volume;
			   ioctl(fd, MI_AO_SETVOLUME, buf1);
			   close(fd);
			}
			char command[100];
			int tiny;
			tiny = (vol * 3) + 40;
			sprintf(command, "/customer/app/tinymix set 6 %d", tiny);
			system(command);
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
	
void getBrightness() {
	cJSON* request_json = NULL;
	cJSON* itemBrightness;
	
	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL) {
		FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
		if (!pipe) {
			settings_file = "/appconfigs/system.json";
		} else {
			char buffer[64];
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
	
	cJSON_Delete(request_json);
	free(request_body);
	
	int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%d", brightness * 10);
		close(fd);
	}
}

int main()
{
	getVolume();
	getBrightness();
	exit(0);
}