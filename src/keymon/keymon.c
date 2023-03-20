#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "cJSON.h"

//	Button Defines
#define	BUTTON_MENU         KEY_ESC
#define	BUTTON_POWER        KEY_POWER
#define	BUTTON_SELECT       KEY_RIGHTCTRL
#define	BUTTON_START        KEY_ENTER
#define	BUTTON_L1           KEY_E
#define	BUTTON_R1           KEY_T
#define	BUTTON_L2           KEY_TAB
#define	BUTTON_R2           KEY_BACKSPACE
#define BUTTON_UP           KEY_UP
#define BUTTON_DOWN         KEY_DOWN
#define BUTTON_LEFT         KEY_LEFT
#define BUTTON_RIGHT        KEY_RIGHT
#define BUTTON_VOLUMEUP     KEY_VOLUMEUP
#define BUTTON_VOLUMEDOWN   KEY_VOLUMEDOWN

#define BRIMAX		10
#define BRIMIN		1

// for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

// Set Volume (Raw)
#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c

int setVolumeRaw(int volume, int add)
{
    int recent_volume = 0;
    int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
        ioctl(fd, MI_AO_GETVOLUME, buf1);
        recent_volume = buf2[1];
        if (add) {
            buf2[1] += add;
            if (buf2[1] > 30)
                buf2[1] = 30;
            else if (buf2[1] < -30)
                buf2[1] = -30;
        }
        else
            buf2[1] = volume;
        if (buf2[1] != recent_volume)
            ioctl(fd, MI_AO_SETVOLUME, buf1);
        close(fd);
    }
    return recent_volume;
}


// Global Variables
static struct input_event	ev;
static int	input_fd = 0;


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

// Increase/Decrease Brightness
void modifyBrightness(int inc) {
  cJSON* request_json = NULL;
  cJSON* itemBrightness;

  char *settings_file = getenv("SETTINGS_FILE");
    if (settings_file == NULL)
        settings_file = "/appconfigs/system.json";

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

int main (int argc, char *argv[]) {
  input_fd = open("/dev/input/event0", O_RDONLY);

  modifyBrightness(0);

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
          repeat_power = 0;
        } else if (val == RELEASED && power_pressed) {
          power_pressed = val;
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
			val = PRESSED;
			setVolumeRaw(0, +3);
	    break;
	    case BUTTON_VOLUMEDOWN:
			val = PRESSED;
			setVolumeRaw(0, -3);
	    break;
      default:
      break;
    }

    if (shutdown) {
      power_pressed = 0;
      system("sync; reboot");
      while (1) pause();
    }
  }
  exit(EXIT_FAILURE);
}
