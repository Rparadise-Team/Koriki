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

// for suspend / hibernate
#define CHECK_SEC 15   // Interval sec to check hibernate/adc
#define SHUTDOWN_MIN 1 // Minutes to power off after hibernate
#define REPEAT_SEC(val) ((val * 1000 - 250) / 50)
#define PIDMAX 32

uint32_t suspendpid[PIDMAX];

// Global Variables
static struct input_event	ev;
static int	input_fd = 0;

//
//    Suspend / Kill processes
//        mode: 0:STOP 1:TERM 2:KILL
//
int suspend(uint32_t mode)
{
    DIR *procdp;
    struct dirent *dir;
    char fname[32];
    pid_t suspend_pid = getpid();
    pid_t pid;
    pid_t ppid;
    char state;
    uint32_t flags;
    char comm[128];
    int ret = 0;

    // terminate retroarch before kill
    if (mode == 2)
        ret = terminate_retroarch();

    sync();
    procdp = opendir("/proc");

    // Pick active processes to suspend and send SIGSTOP
    // Cond:1. PID is greater than 2(kthreadd) and not myself
    //    2. PPID is greater than 2(kthreadd)
    //    3. state is "R" or "S" or "D"
    //    4. comm is not "(sh)" / "(MainUI)" when AudioFix:OFF
    //    5. flags does not contain PF_KTHREAD (0x200000) (just in case)
    //    6. comm is not "(updater)" "(MainUI)" "(tee)" "(audioserver*" when
    //    kill mode
    if (!mode)
        suspendpid[0] = 0;
    while ((dir = readdir(procdp))) {
        if (dir->d_type == DT_DIR) {
            pid = atoi(dir->d_name);
            if ((pid > 2) && (pid != suspend_pid)) {
                sprintf(fname, "/proc/%d/stat", pid);
                FILE *fp = fopen(fname, "r");
                if (fp) {
                    fscanf(fp, "%*d %127s %c %d %*d %*d %*d %*d %u",
                           (char *)&comm, &state, &ppid, &flags);
                    fclose(fp);
                }
                if ((ppid > 2) &&
                    ((state == 'R') || (state == 'S') || (state == 'D')) &&
                    (strcmp(comm, "(sh)")) && (!(flags & PF_KTHREAD))) {
                    if (mode) {
                        if ((strcmp(comm, "(updater)")) &&
                            (strcmp(comm, "(MainUI)")) &&
                            (strcmp(comm, "(tee)")) &&
                            (strncmp(comm, "(audioserver", 12)) &&
                            (strcmp(comm, "(charging)"))) {
                            kill(pid, (mode == 1) ? SIGTERM : SIGKILL);
                            ret++;
                        }
                    }
                    else {
                        if (suspendpid[0] < PIDMAX) {
                            suspendpid[++suspendpid[0]] = pid;
                            kill(pid, SIGSTOP);
                            ret++;
                        }
                    }
                }
            }
        }
    }
    closedir(procdp);

    // reset display when anything killed
    if (mode == 2 && ret)
        display_reset();

    return ret;
}

//
//    Resume
//
void resume(void)
{
    // Send SIGCONT to suspended processes
    if (suspendpid[0]) {
        for (uint32_t i = 1; i <= suspendpid[0]; i++)
            kill(suspendpid[i], SIGCONT);
        suspendpid[0] = 0;
    }
}

//
//    Quit
//
void quit(int exitcode)
{
    display_free();
    if (input_fd > 0)
        close(input_fd);
    system_clock_get();
    system_rtc_set();
    system_clock_save();
    exit(exitcode);
}

//
//    Suspend interface
//
void suspend_exec(int timeout)
{
    keyinput_disable();

    // suspend
    system_clock_pause(true);
    suspend(0);
    rumble(0);
    setVolume(0);
    display_setBrightnessRaw(0);
    display_off();
    system_powersave_on();

    uint32_t repeat_power = 0;
    uint32_t killexit = 0;

    while (1) {
        int ready = poll(fds, 1, timeout);

        if (ready > 0) {
            read(input_fd, &ev, sizeof(ev));
            if ((ev.type != EV_KEY) || (ev.value > REPEAT))
                continue;
            if (ev.code == HW_BTN_POWER) {
                if (ev.value == RELEASED)
                    break;
                else if (ev.value == PRESSED)
                    repeat_power = 0;
                else if (ev.value == REPEAT) {
                    if (++repeat_power >= REPEAT_SEC(5)) {
                        short_pulse();
                        killexit = 1;
                        break;
                    }
                }
            }
            else if (ev.value == RELEASED) {
                if (ev.code == HW_BTN_MENU) {
                    // screenshot
                    system_powersave_off();
                    display_on();
                    takeScreenshot();
                    break;
                }
            }
        }
        else if (!ready && !battery_isCharging()) {
            // shutdown
            system_powersave_off();
            resume();
            usleep(100000);
            shutdown();
        }
    }

    // resume
    system_powersave_off();
    if (killexit) {
        resume();
        usleep(100000);
        suspend(2);
        usleep(400000);
    }
    display_on();
    display_setBrightness(settings.brightness);
    setVolume(settings.mute ? 0 : settings.volume);
    if (!killexit) {
        resume();
        system_clock_pause(false);
    }

    keyinput_enable();
}

//
//    [onion] deepsleep if MainUI/gameSwitcher/retroarch is running
//
void deepsleep(void)
{
    system_state_update();
    if (system_state == MODE_MAIN_UI) {
        short_pulse();
        system_shutdown();
        kill_mainUI();
    }
    else if (system_state == MODE_GAME) {
        if (check_autosave()) {
            short_pulse();
            system_shutdown();
            terminate_retroarch();
        }
    }
    else if (system_state == MODE_APPS) {
        short_pulse();
        remove(CMD_TO_RUN_PATH);
        system_shutdown();
        suspend(1);
    }
}

void turnOffScreen(void)
{
    int timeout = (settings.sleep_timer + SHUTDOWN_MIN) * 60000;
    bool stay_awake = settings.sleep_timer == 0 || temp_flag_get("stay_awake");
    suspend_exec(stay_awake ? -1 : timeout);
}

//
//    Terminate retroarch before kill/shotdown processes to save progress
//
bool terminate_retroarch(void)
{
    char fname[16];
    pid_t pid = process_searchpid("retroarch");
    if (!pid)
        pid = process_searchpid("ra32");

    if (pid) {

        // send signal
        kill(pid, SIGCONT);
        usleep(100000);
        kill(pid, SIGTERM);
        // wait for terminate
        sprintf(fname, "/proc/%d", pid);

        uint32_t count = 20; // 4s
        while (--count && exists(fname))
            usleep(200000); // 0.2s

        return true;
    }

    return false;
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

// Increments between -63 and -3
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
            else if (buf2[1] < -63) buf2[1] = -63;
        } else buf2[1] = volume;
        if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
        close(fd);
    }

  // Increase/Decrease Volume
	cJSON* request_json = NULL;
	cJSON* itemVol;

	const char *settings_file = getenv("SETTINGS_FILE");
	if (settings_file == NULL){
        settings_file = "/appconfigs/system.json";
	}

	// Store in system.json
	char *request_body = load_file(settings_file);
	request_json = cJSON_Parse(request_body);
	itemVol = cJSON_GetObjectItem(request_json, "vol");
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

  cJSON_Delete(request_json);
  free(request_body);
  
  return recent_volume;
}

// Increments between 0 and 20
int setVolume(int volume, int add) {
    int recent_volume = 0;
    int rawVolumeValue = 0;
    int rawAdd = 0;
    
    rawVolumeValue = (volume * 3) - 63;
    rawAdd = (add * 3);
    
    recent_volume = setVolumeRaw(rawVolumeValue, rawAdd);
    return (int)((recent_volume/3)+20);
}

// Increase/Decrease Brightness
void modifyBrightness(int inc) {
  cJSON* request_json = NULL;
  cJSON* itemBrightness;

  const char *settings_file = getenv("SETTINGS_FILE");
    if (settings_file == NULL){
        settings_file = "/appconfigs/system.json";
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

int main (int argc, char *argv[]) {
  input_fd = open("/dev/input/event0", O_RDONLY);

  modifyBrightness(0);
  setVolume(0,0);
  int volume = 0;

 //READ Volume valor from system
  cJSON* request_json = NULL;
  cJSON* itemVol;
  const char *settings_file = getenv("SETTINGS_FILE");
  if (settings_file == NULL){
        settings_file = "/appconfigs/system.json";
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
  int ticks = getMilliseconds();
  int hibernate_start = ticks;
  int hibernate_time;
  int elapsed_sec = 0;

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
          if (repeat_power > 7)
			turnOffScreen();
		} else if (repeat_power == 7)
            deepsleep(); // 0.5sec deepsleepif
		} else if (repeat_power >= 20) {
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
/*	  case BUTTON_LEFT:
        if (val == REPEAT) {
          // Adjust repeat speed to 1/2
          val = repeat;
          repeat ^= PRESSED;
        } else {
          repeat = 0;
        }
        if (val == PRESSED && menu_pressed) {
          // Increase volume
          setVolumeRaw(recent_volume, +3);
        }
      break;
      case BUTTON_RIGHT:
        if (val == REPEAT) {
          // Adjust repeat speed to 1/2
          val = repeat;
          repeat ^= PRESSED;
        } else {
          repeat = 0;
        }
        if (val == PRESSED && menu_pressed) {
          // Decrease volume
          setVolumeRaw(recent_volume, -3);
        }
      break;*/ //test miyoo mini
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

		ticks = getMilliseconds();

        // Check Hibernate
        if (battery_isCharging())
            hibernate_time = 0;
		else
         hibernate_time = settings.sleep_timer;

     if (hibernate_time && !temp_flag_get("stay_awake")) {
         if (ticks - hibernate_start > hibernate_time * 60 * 1000)
			suspend_exec(SHUTDOWN_MIN * 60000);
            hibernate_start = ticks;
         }
    }

    if (shutdown) {
      power_pressed = 0;
      if (access("/customer/app/axp_test", F_OK) == 0)
		system("killall main; killall updater; killall audioserver; killall audioserver.plu; killall retroarch; killall simplemenu; killall keymon; /etc/init.d/K00_Sys; sync; sleep 5; poweroff");
      else
        system("killall main; killall updater; killall audioserver; killall audioserver.min; killall retroarch; killall simplemenu; killall keymon; /etc/init.d/K00_Sys; sync; sleep 5; reboot");
      while (1) pause();
    }
  }
  exit(EXIT_FAILURE);
}
