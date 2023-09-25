#include <fcntl.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <mi_ao.h>
#include <mi_sys.h>
#include <mi_common.h>
#include <mi_disp.h>
#include <SDL/SDL.h>
#include <SDL/SDL_sound.h>
#include <SDL/SDL_mixer.h>
#include "../headers/logic.h"
#include "../headers/system_logic.h"
#include "../headers/globals.h"
#include "../headers/utils.h"
#include "../headers/cJSON.h"

// Set Volume (Raw)
#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE 0x4008690d


volatile uint32_t *memregs;
int32_t memdev = 0;
int oldCPU;

typedef struct {
    int channel_value;
    int adc_value;
} SAR_ADC_CONFIG_READ;
 
#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)

static SAR_ADC_CONFIG_READ  adcCfg = {0,0};
static int sar_fd = 0;

static void initADC(void) {
    sar_fd = open("/dev/sar", O_WRONLY);
    ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
}

static int is_charging = 0;
void checkCharging(void) {
  int charging = 0;
  if (mmModel == 0) {
    char *cmd = "cd /customer/app/ ; ./axp_test";
    int axp_response_size = 100;
    char buf[axp_response_size];
    int battery = 0;
    int voltage = 0;

    FILE *fp;
    fp = popen(cmd, "r");
      if (fgets(buf, axp_response_size, fp) != NULL)
        sscanf(buf,  "{\"battery\":%d, \"voltage\":%d, \"charging\":%d}", &battery, &voltage, &charging);
    pclose(fp);
    is_charging = charging;
  } else {
    FILE *file = fopen("/sys/devices/gpiochip0/gpio/gpio59/value", "r");
    if (file!=NULL) {
      fscanf(file, "%i", &charging);
      fclose(file);
    }
    is_charging = charging;
  }
}

int percBat = 0;
int firstLaunch = 1; 
int countChecks = 0;

static int checkADC(void) {  
    ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);

    int old_is_charging = is_charging;
    checkCharging();

        //char val[3];

            int percBatTemp = 0;
            if (is_charging == 0) {
                if (adcCfg.adc_value >= 528)
                    percBatTemp = adcCfg.adc_value-478;
                else if ((adcCfg.adc_value >= 512) && (adcCfg.adc_value < 528))
                    percBatTemp = (int)(adcCfg.adc_value*2.125-1068);
                else if ((adcCfg.adc_value >= 480) && (adcCfg.adc_value < 512))
                    percBatTemp = (int)(adcCfg.adc_value* 0.51613 - 243.742);

                if ((firstLaunch == 1) || (old_is_charging == 1)) {
                    // Calibration needed at first launch or when the
                    // user just unplugged his charger
                    firstLaunch = 0;
                    percBat = percBatTemp;
                } else {
                    if (percBat>percBatTemp) {
                        percBat--;
                    } else if (percBat < percBatTemp) {
                        percBat++;
                    }
                }
                if (percBat<0)
                    percBat=0;
                else if (percBat>100)
                    percBat=100;
            } else {
                // The handheld is currently charging
                percBat = 500;
            }
    return percBat;
}

void to_string(char str[], int num) {
    int i, rem, len = 0, n;

    n = num;
    while (n != 0) {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++) {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

void setCPU(uint32_t mhz) {
    currentCPU = mhz;
}

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

int getCurrentSystemValue(char const *key) {
    cJSON* request_json = NULL;
    cJSON* item = NULL;
    int result = 0;

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

    char* request_body = load_file(settings_file);
    request_json = cJSON_Parse(request_body);
    item = cJSON_GetObjectItem(request_json, key);
    result = cJSON_GetNumberValue(item);
    free(request_body);
    return result;
}

void setSystemValue(char const *key, int value) {
    cJSON* request_json = NULL;
    cJSON* item = NULL;

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
    char* request_body = load_file(settings_file);
    request_json = cJSON_Parse(request_body);
    item = cJSON_GetObjectItem(request_json, key);
    cJSON_SetNumberValue(item, value);

    FILE *file = fopen(settings_file, "w");
    char *test = cJSON_Print(request_json);
    fputs(test, file);
    fclose(file);
    free(request_body);
}

void turnScreenOnOrOff(int state) {
    const char *path1 = "/proc/mi_modules/fb/mi_fb0";
	const char *path2 = "/sys/class/pwm/pwmchip0/pwm0/enable";
    const char *blank = state ? "GUI_SHOW 0 off" : "GUI_SHOW 0 on";
	const char *power = state ? "0" : "1";
    int fd1 = open(path1, O_RDWR);
    int ret1 = write(fd1, blank, strlen(blank));
	int fd2 = open(path2, O_RDWR);
    int ret2 = write(fd2, power, strlen(power));
	
    if (ret1==-1) {
        generateError("FATAL ERROR", 1);
    }
    close(fd1);
	
    if (ret2==-1) {
        generateError("FATAL ERROR", 1);
    }
    close(fd2);
}

void clearTimer() {
    if (timeoutTimer != NULL) {
        SDL_RemoveTimer(timeoutTimer);
    }
    timeoutTimer = NULL;
}

uint32_t suspend() {
    if (timeoutValue!=0) {
        clearTimer();
        //oldCPU=currentCPU;
        turnScreenOnOrOff(1);
        isSuspended=1;
    }
    return 0;
};

void resetScreenOffTimer() {
    if (isSuspended) {
        turnScreenOnOrOff(0);
        //currentCPU=oldCPU;
        isSuspended=0;
    }
    clearTimer();
    timeoutTimer=SDL_AddTimer(timeoutValue * 1e3, suspend, NULL);
}

void initSuspendTimer() {
    timeoutTimer=SDL_AddTimer(timeoutValue * 1e3, suspend, NULL);
    isSuspended=0;
    logMessage("INFO","initSuspendTimer","Suspend timer initialized");
}

// Increments between -60 and 9 or 40 and 109
int setVolumeRaw(int volume, int add, int tiny) {
	int fix;
    int recent_volume = 0;
	int set = 0;
	
	fix = getCurrentSystemValue("audiofix");
	
	if (fix == 1) {
        int fd = open("/dev/mi_ao", O_RDWR);
        if (fd >= 0) {
            int buf2[] = {0, 0};
            uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
            ioctl(fd, MI_AO_GETVOLUME, buf1);
            recent_volume = buf2[1];
            if (add) {
                buf2[1] += add;
                if (buf2[1] > 9) buf2[1] = 9;
                else if (buf2[1] < -60) buf2[1] = -60;
            } else buf2[1] = volume;
            if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
            close(fd);
        }
	} else if (fix == 0) {
		set = tiny+add; //tinymix work in 100-40 // 0-(-60)
        if (set >= 109) set = 109;
        else if (set <= 40) set = 40;
		char command[100];
		sprintf(command, "tinymix set 6 %d", set);
		system(command);
		int fd = open("/dev/mi_ao", O_RDWR);
        if (fd >= 0) {
            int buf2[] = {0, 0};
            uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
            ioctl(fd, MI_AO_GETVOLUME, buf1);
            recent_volume = buf2[1];
            if (add) {
                buf2[1] += add;
                if (buf2[1] > 9) buf2[1] = 9;
                else if (buf2[1] < -60) buf2[1] = -60;
            } else buf2[1] = volume;
            if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
            close(fd);
        }
	}
  
  return recent_volume;
}

// Read volume from system config and set this automatic
int getCurrentVolume() {
	int sysvolume;
	int volume;
	int tiny;
    int fix;
	sysvolume = getCurrentSystemValue("vol");
    fix = getCurrentSystemValue("audiofix");
	volume = (sysvolume * 3) - 60;
	tiny = (sysvolume * 3) + 40;

    if (fix == 1) {
	int fv = open("/dev/mi_ao", O_RDWR);
	if (fv >= 0) {
		int buf2[] = {0, 0};
		uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
						
		ioctl(fv, MI_AO_GETVOLUME, buf1);
		buf2[1] = volume;
		ioctl(fv, MI_AO_SETVOLUME, buf1);
        close(fv); 
	    }
    }
	
    if (fix == 0) {
	char command[100];
	sprintf(command, "tinymix set 6 %d", tiny);
	system(command);
	int fv = open("/dev/mi_ao", O_RDWR);
	if (fv >= 0) {
		int buf2[] = {0, 0};
		uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
						
		ioctl(fv, MI_AO_GETVOLUME, buf1);
		buf2[1] = volume;
		ioctl(fv, MI_AO_SETVOLUME, buf1);
        close(fv); 
	    }
    }
	
	return sysvolume;
}

// Increments between 0 and 23 or 40 and 109
int setVolume(int volume, int add) {
    int recent_volume = 0;
	int tinyvol = 0;
    int rawVolumeValue = 0;
    int rawAdd = 0;
	
    rawVolumeValue = (volume * 3) - 60;
	tinyvol = (volume * 3) + 40;
    rawAdd = (add * 3);
    
    recent_volume = setVolumeRaw(rawVolumeValue, rawAdd, tinyvol);
    return recent_volume;
}

Mix_Music *music = NULL;

void startmusic() {
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "error init SDL: %s\n", SDL_GetError());
        return;
    }

    if(Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 4096) == -1) {
        fprintf(stderr, "error init SDL_mixer: %s\n", Mix_GetError());
        return;
    }

    Mix_Music *music = Mix_LoadMUS("/mnt/SDCARD/Media/music.wav");
    if(music == NULL) {
        fprintf(stderr, "error to load audio file: %s\n", Mix_GetError());
        return;
    }

    Mix_PlayMusic(music, -1);
}

void stopmusic() {
	Mix_FreeMusic(music);
    Mix_CloseAudio();
	music = NULL;
}

void HW_Init() {
	int brightness = 0;
	brightness = getCurrentBrightness();
    initADC();
    getCurrentVolume();
	startmusic();
	setBrightness(brightness);
    logMessage("INFO","HW_Init","HW Initialized");
}

void rumble() {

}

int getBatteryLevel() {
  //int max_voltage;
  //int voltage_now;
  //int total;
    int charge = 0;
    if (mmModel)
        charge = checkADC();
    else {
        checkCharging();
        if (is_charging)
            charge = 500;
        else {
            char *cmd = "cd /customer/app/ ; ./axp_test";
            int axp_response_size = 100;
            char buf[axp_response_size];

            FILE *fp;
            fp = popen(cmd, "r");
              if (fgets(buf, axp_response_size, fp) != NULL)
                sscanf(buf,  "{\"battery\":%d, \"voltage\":%*d, \"charging\":%*d}", &charge);
            pclose(fp);
        }
    }
    if (charge<=10)       return 1;
    else if (charge<=20)  return 2;
    else if (charge<=30)  return 3;
    else if (charge<=40)  return 4;
    else if (charge<=50)  return 5;
    else if (charge<=60)  return 6;
    else if (charge<=70)  return 7;
    else if (charge<=80)  return 8;
    else if (charge<=90)  return 9;
    else if (charge<=100) return 10;
    else                  return 11;

}

int getCurrentBrightness() {
    return getCurrentSystemValue("brightness");
}

int getMaxBrightness() {
    return 10;
}

int getCurrentWifi() {
	if (mmModel)
		return 2;
	else
		return getCurrentSystemValue("wifi");
}

void setBrightness(int value) {
    FILE *f = fopen("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", "w");
    if (f!=NULL) {
        fprintf(f, "%d", value * 10);
        fclose(f);
    }
}

void Luma(int dev, int value) {
    MI_DISP_PubAttr_t attrs;
    memset(&attrs, 0, sizeof(MI_DISP_PubAttr_t));
    MI_DISP_GetPubAttr(dev, &attrs);

    attrs.eIntfType = E_MI_DISP_INTF_LCD;
    attrs.eIntfSync = E_MI_DISP_OUTPUT_USER;
    MI_DISP_SetPubAttr(dev, &attrs);

    MI_DISP_Enable(dev);

    MI_DISP_LcdParam_t params;
    memset(&params, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(dev, &params);
    params.stCsc.u32Luma = value;
    MI_DISP_SetLcdParam(dev, &params);
}

void Hue(int dev, int value) {
    MI_DISP_PubAttr_t attrs;
    memset(&attrs, 0, sizeof(MI_DISP_PubAttr_t));
    MI_DISP_GetPubAttr(dev, &attrs);

    attrs.eIntfType = E_MI_DISP_INTF_LCD;
    attrs.eIntfSync = E_MI_DISP_OUTPUT_USER;
    MI_DISP_SetPubAttr(dev, &attrs);

    MI_DISP_Enable(dev);

    MI_DISP_LcdParam_t params;
    memset(&params, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(dev, &params);
    params.stCsc.u32Hue = value;
    MI_DISP_SetLcdParam(dev, &params);
}

void Saturation(int dev, int value) {
    MI_DISP_PubAttr_t attrs;
    memset(&attrs, 0, sizeof(MI_DISP_PubAttr_t));
    MI_DISP_GetPubAttr(dev, &attrs);

    attrs.eIntfType = E_MI_DISP_INTF_LCD;
    attrs.eIntfSync = E_MI_DISP_OUTPUT_USER;
    MI_DISP_SetPubAttr(dev, &attrs);

    MI_DISP_Enable(dev);

    MI_DISP_LcdParam_t params;
    memset(&params, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(dev, &params);
    params.stCsc.u32Saturation = value;
    MI_DISP_SetLcdParam(dev, &params);
}

void Contrast(int dev, int value) {
    MI_DISP_PubAttr_t attrs;
    memset(&attrs, 0, sizeof(MI_DISP_PubAttr_t));
    MI_DISP_GetPubAttr(dev, &attrs);

    attrs.eIntfType = E_MI_DISP_INTF_LCD;
    attrs.eIntfSync = E_MI_DISP_OUTPUT_USER;
    MI_DISP_SetPubAttr(dev, &attrs);

    MI_DISP_Enable(dev);

    MI_DISP_LcdParam_t params;
    memset(&params, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(dev, &params);
    params.stCsc.u32Contrast = value;
    MI_DISP_SetLcdParam(dev, &params);
}
