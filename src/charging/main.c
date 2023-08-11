#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <poll.h>

#define ANIMATION_DELAY 200000
#define ANIMATION_LOOPS 10
#define ANIMATION_IMAGES 6

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

//  Button Defines
#define BUTTON_MENU   KEY_ESC
#define BUTTON_POWER  KEY_POWER
#define BUTTON_SELECT KEY_RIGHTCTRL
#define BUTTON_START  KEY_ENTER
#define BUTTON_L1     KEY_E
#define BUTTON_R1     KEY_T
#define BUTTON_L2     KEY_TAB
#define BUTTON_R2     KEY_BACKSPACE
#define BUTTON_A	  KEY_SPACE
#define BUTTON_B	  KEY_LEFTCTRL
#define BUTTON_X	  KEY_LEFTSHIFT
#define BUTTON_Y	  KEY_LEFTALT
#define BUTTON_UP     KEY_UP
#define BUTTON_DOWN   KEY_DOWN
#define BUTTON_LEFT   KEY_LEFT
#define BUTTON_RIGHT  KEY_RIGHT
#define BUTTON_VOLUMEUP		KEY_VOLUMEUP
#define BUTTON_VOLUMEDOWN	KEY_VOLUMEDOWN

//  for ev.value
#define RELEASED  0
#define PRESSED   1
#define REPEAT    2


//  Global Variables
static struct input_event ev;
static int  input_fd = 0;
static struct pollfd fds[1];
static int is_charging = 0;
static bool running = true;
static bool screen_on = true;
static int animation_image = 0;
static int animation_loop = 0;
static int mmp = 0;
static time_t last_activity_time;

void checkCharging(void) {
  int charging = 0;
  if (access("/customer/app/axp_test", F_OK) == 0) {
    mmp = 1;
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

void logMessage(char* Message) {
  if (access("/mnt/SDCARD/.tmp_update/log_charging_Message.txt", F_OK) == 0) {
  FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_charging_Message.txt", "a");
  char valLog[200];
  sprintf(valLog, "%s %s", Message, "\n");
  fputs(valLog, file);
  fclose(file);
  } else {
  system("touch /mnt/SDCARD/.tmp_update/log_charging_Message.txt");
  FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_charging_Message.txt", "a");
  char valLog[200];
  sprintf(valLog, "%s %s", Message, "\n");
  fputs(valLog, file);
  fclose(file);
  }
}

void SetBrightness(int value) {  // value = 0-10
  int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
  if (fd>=0) {
    dprintf(fd,"%d",value*10);
    close(fd);
  }
}

static void sigHandler(int sig) {
  switch (sig) {
    case SIGINT:
    case SIGTERM:
      running = false;
      break;
    default: break;
  }
}

int main(void) {
  signal(SIGINT, sigHandler);
  signal(SIGTERM, sigHandler);

  checkCharging();
  if (is_charging == 0){
    return EXIT_SUCCESS;
  }

  // Prepare for Poll button input
  input_fd = open("/dev/input/event0", O_RDONLY);
  memset(&fds, 0, sizeof(fds));
  fds[0].fd = input_fd;
  fds[0].events = POLLIN;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(SDL_DISABLE);

  SDL_Surface* video = SDL_SetVideoMode(640,480, 32, SDL_HWSURFACE);
  SDL_Surface* screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640,480, 32, 0,0,0,0);
  SDL_Surface* black_image = IMG_Load("/mnt/SDCARD/Koriki/images/black.png");

  int image_index = 0;
  SDL_Surface *image;
  SDL_Surface *images[6];
  char image_path[100];
  for (int i = 0; i < ANIMATION_IMAGES; i++) {
    snprintf(image_path, 99, "/mnt/SDCARD/Koriki/images/chargingState%d.png", i);
    if ((image = IMG_Load(image_path)))
      images[image_index++] = image;
  }

  SetBrightness(8);

  bool power_pressed = false;
  int repeat_power = 0;
  bool screen_on = true;
  last_activity_time = time(NULL);

  while (running) {
    if (animation_loop < ANIMATION_LOOPS) {
      SDL_BlitSurface(images[animation_image++], NULL, screen, NULL);
      SDL_BlitSurface(screen, NULL, video, NULL);
      SDL_Flip(video);
      if (animation_image == ANIMATION_IMAGES) {
        animation_image = 0;
        animation_loop++;
      }
    } else {
      if (screen_on) SetBrightness(0);
	  SDL_BlitSurface(black_image, NULL, screen, NULL);
	  SDL_BlitSurface(screen, NULL, video, NULL);
	  SDL_Flip(video);
	  system("echo powersave > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	  system("echo 400000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
      screen_on = false;
    }

    checkCharging();
    if (is_charging == 0){
      if (mmp)
        system("/etc/init.d/K00_Sys; sync; umount -l /mnt/SDCARD; poweroff; sleep 10");
      else
        system("/etc/init.d/K00_Sys; sync; umount -l /mnt/SDCARD; reboot; sleep 10");
    }

    while (poll(fds, 1, 0)) {
      read(input_fd, &ev, sizeof(ev));
      if (ev.type != EV_KEY || ev.value > REPEAT) continue;

      if (ev.code == BUTTON_POWER) {
        if (ev.value == PRESSED) {
          power_pressed = true;
          repeat_power = 0;
        } else if (ev.value == RELEASED && power_pressed) {
          power_pressed = false;
		  screen_on = true;
		  SetBrightness(8);
		  animation_loop = 0;
        } else if (ev.value == REPEAT) {
          if (repeat_power >= 5) {
			system("echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
			system("echo 1200000 > /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq");
            running = false; // power on
          }
          repeat_power++;
        }
      }
    }
    
    if (mmp) { // autopoweroff MMP in 60s
            time_t current_time = time(NULL);
            if (current_time - last_activity_time >= 60) {
                system("/sbin/poweroff");
                running = false;
            }
        }

    usleep(ANIMATION_DELAY);
  }

  SDL_FreeSurface(screen);
  SDL_FreeSurface(video);
  SDL_FreeSurface(black_image);
  SDL_Quit();

  return EXIT_SUCCESS;
}
