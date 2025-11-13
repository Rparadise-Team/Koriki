#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include <ctype.h>
#include <time.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <SDL/SDL.h>
#include "cJSON.h"
#include <json-c/json.h>
#include "osdFramebuffer.h"

#define BUTTON_MENU         KEY_ESC
#define BUTTON_POWER        KEY_POWER
#define BUTTON_SELECT       KEY_RIGHTCTRL
#define BUTTON_START        KEY_ENTER
#define BUTTON_L1           KEY_E
#define BUTTON_R1           KEY_T
#define BUTTON_L2           KEY_TAB
#define BUTTON_R2           KEY_BACKSPACE
#define BUTTON_A            KEY_SPACE
#define BUTTON_B            KEY_LEFTCTRL
#define BUTTON_X            KEY_LEFTSHIFT
#define BUTTON_Y            KEY_LEFTALT
#define BUTTON_UP           KEY_UP
#define BUTTON_DOWN         KEY_DOWN
#define BUTTON_LEFT         KEY_LEFT
#define BUTTON_RIGHT        KEY_RIGHT
#define BUTTON_VOLUMEUP     KEY_VOLUMEUP
#define BUTTON_VOLUMEDOWN   KEY_VOLUMEDOWN

#define CPUSAVE     "/mnt/SDCARD/.simplemenu/cpu.sav"
#define GOVSAVE     "/mnt/SDCARD/.simplemenu/governor.sav"
#define SPEEDSAVE   "/mnt/SDCARD/.simplemenu/speed.sav"
#define BRIMAX      10
#define BRIMIN      1

#define RELEASED    0
#define PRESSED     1
#define REPEAT      2

#define MI_AO_SETVOLUME 0x4008690b
#define MI_AO_GETVOLUME 0xc008690c
#define MI_AO_SETMUTE   0x4008690d

#define BASE_REG_RIU_PA   (0x1F000000)
#define BASE_REG_MPLL_PA  (BASE_REG_RIU_PA + 0x103000*2)
#define PLL_SIZE          (0x1000)

enum cpugov { PERFORMANCE = 0, POWERSAVE = 1, ONDEMAND = 2, USERSPACE = 3 };
static uint32_t current_cpu_freq = 1200000;
static enum cpugov current_governor = ONDEMAND;
static int cpu_config_saved = 0;

static struct input_event ev;
static int input_fd = 0;
static int mmModel = 0;
struct json_object *jval = NULL;
struct json_object *jfile = NULL;

extern int osd_volume;
extern int osd_headphones_connected;
extern int osd_brightness;

static void saveSpeakerVol(int vol);
static int loadSpeakerVol(void);

static int wifi_was_enabled_before_sleep = 0;

static int hp_fd = -1;

typedef enum {
    HP_STATE_UNKNOWN = -1,
    HP_STATE_DISCONNECTED = 0,
    HP_STATE_CONNECTED = 1
} HeadphoneState;

static HeadphoneState hp_state = HP_STATE_UNKNOWN;

static const char *cached_settings_file = NULL;

/* --------- prototipo para evitar el error de iconvol en updateHeadphoneState --------- */
static int iconvol(void);

/* ------------------------------------------------------------------------- */
/*                         CACHE DE system.json                              */
/* ------------------------------------------------------------------------- */
static cJSON *root_json = NULL;

typedef struct SettingsCache {
    int vol;
    int mute;
    int brightness;
    int hibernate;
    int wifi;
    int audiofix;
    int loaded;
    int dirty;
} SettingsCache;

static SettingsCache sc = {0};

/* ------------------------------------------------------------------------- */
/*                          CPU CONFIG / GOVERNOR                            */
/* ------------------------------------------------------------------------- */

static void read_current_cpu_config(uint32_t *freq, enum cpugov *gov) {
    FILE *fp;

    *freq = 1200000;
    *gov = ONDEMAND;

    fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "r");
    if (fp) {
        char gov_str[16];
        if (fgets(gov_str, sizeof(gov_str), fp)) {
            gov_str[strcspn(gov_str, "\n")] = 0;
            if (strcmp(gov_str, "performance") == 0) *gov = PERFORMANCE;
            else if (strcmp(gov_str, "powersave") == 0) *gov = POWERSAVE;
            else if (strcmp(gov_str, "ondemand") == 0) *gov = ONDEMAND;
            else if (strcmp(gov_str, "userspace") == 0) *gov = USERSPACE;
            else *gov = ONDEMAND;
        }
        fclose(fp);
    }

    fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
    if (!fp) {
        fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "r");
    }
    if (fp) {
        fscanf(fp, "%u", freq);
        fclose(fp);
    }
}

static void save_current_cpu_config(void) {
    uint32_t current_freq = 1200000;
    enum cpugov current_gov = ONDEMAND;

    read_current_cpu_config(&current_freq, &current_gov);

    FILE *cpu_file = fopen(CPUSAVE, "w");
    if (cpu_file) {
        fprintf(cpu_file, "%u", current_freq);
        fclose(cpu_file);
    }

    FILE *gov_file = fopen(GOVSAVE, "w");
    if (gov_file) {
        const char govstr[4][12] = { "performance", "powersave", "ondemand", "userspace" };
        fprintf(gov_file, "%s", govstr[current_gov]);
        fclose(gov_file);
    }

    if (current_gov == USERSPACE) {
        FILE *speed_file = fopen(SPEEDSAVE, "w");
        if (speed_file) {
            fprintf(speed_file, "%u", current_freq);
            fclose(speed_file);
        }
    } else {
        FILE *speed_file = fopen(SPEEDSAVE, "w");
        if (speed_file) {
            fprintf(speed_file, "unsupported");
            fclose(speed_file);
        }
    }

    cpu_config_saved = 1;
}

static void set_cpuclock(int clock) {
    sync();
    int fd_mem = open("/dev/mem", O_RDWR);
    if (fd_mem < 0) {
        return;
    }

    void* pll_map = mmap(0, PLL_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, BASE_REG_MPLL_PA);
    if (pll_map == MAP_FAILED) {
        close(fd_mem);
        return;
    }

    uint32_t post_div;
    if (clock >= 800000) post_div = 2;
    else if (clock >= 400000) post_div = 4;
    else if (clock >= 200000) post_div = 8;
    else post_div = 16;

    static const uint64_t divsrc = 432000000llu * 524288;
    uint32_t rate = (clock * 1000)/16 * post_div / 2;
    uint32_t lpf = (uint32_t)(divsrc / rate);
    volatile uint16_t* p16 = (uint16_t*)pll_map;

    uint32_t cur_post_div = (p16[0x232] & 0x0F) + 1;
    uint32_t tmp_post_div = cur_post_div;
    if (post_div > cur_post_div) {
        while (tmp_post_div != post_div) {
            tmp_post_div <<= 1;
            p16[0x232] = (p16[0x232] & 0xF0) | ((tmp_post_div-1) & 0x0F);
        }
    }

    p16[0x2A8] = 0x0000;
    p16[0x2AE] = 0x000F;
    p16[0x2A4] = lpf&0xFFFF;
    p16[0x2A6] = lpf>>16;
    p16[0x2B0] = 0x0001;
    p16[0x2B2] |= 0x1000;
    p16[0x2A8] = 0x0001;
    while( !(p16[0x2BA]&1) );
    p16[0x2A0] = lpf&0xFFFF;
    p16[0x2A2] = lpf>>16;

    if (post_div < cur_post_div) {
        while (tmp_post_div != post_div) {
            tmp_post_div >>= 1;
            p16[0x232] = (p16[0x232] & 0xF0) | ((tmp_post_div-1) & 0x0F);
        }
    }

    munmap(pll_map, PLL_SIZE);
    close(fd_mem);

    current_cpu_freq = clock;
}

static void set_cpugovernor_optimized(enum cpugov gov) {
    const char govstr[4][12] = { "performance", "powersave", "ondemand", "userspace" };
    const char fn_min_freq[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq";
    const char fn_max_freq[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
    const char fn_governor[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
    const char fn_setspeed[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed";
    static uint32_t minfreq = 0;
    FILE* fp;

    if (!minfreq) {
        fp = fopen(fn_min_freq, "r");
        if (fp) {
            fscanf(fp, "%d", &minfreq);
            fclose(fp);
        }
    }

    if (gov == ONDEMAND) {
        fp = fopen(fn_min_freq, "w");
        if (fp) {
            fprintf(fp, "%d", minfreq > 0 ? minfreq : 400000);
            fclose(fp);
        }
    } else {
        fp = fopen(fn_min_freq, "w");
        if (fp) {
            if (gov == POWERSAVE) {
                fprintf(fp, "%d", current_cpu_freq);
            } else {
                fprintf(fp, "%d", 400000);
            }
            fclose(fp);
        }
    }

    fp = fopen(fn_governor, "w");
    if (fp) {
        fwrite(govstr[gov], 1, strlen(govstr[gov]), fp);
        fclose(fp);
        current_governor = gov;
    }

    fp = fopen(fn_max_freq, "w");
    if (fp) {
        fprintf(fp, "%d", current_cpu_freq);
        fclose(fp);
    }

    if (gov == USERSPACE) {
        int fset = open(fn_setspeed, O_WRONLY);
        if (fset >= 0) {
            char str[16];
            sprintf(str, "%d", current_cpu_freq);
            write(fset, str, strlen(str));
            close(fset);
        }
    }
}

static void restore_cpu_config(void) {
    uint32_t saved_freq = 1200000;
    enum cpugov saved_gov = ONDEMAND;

    FILE *cpu_file = fopen(CPUSAVE, "r");
    if (cpu_file) {
        fscanf(cpu_file, "%d", &saved_freq);
        fclose(cpu_file);
    } else {
        cpu_file = fopen(CPUSAVE, "w");
        if (cpu_file) {
            fprintf(cpu_file, "%d", saved_freq);
            fclose(cpu_file);
        }
    }

    FILE *gov_file = fopen(GOVSAVE, "r");
    if (gov_file) {
        char gov_str[16];
        if (fgets(gov_str, sizeof(gov_str), gov_file)) {
            gov_str[strcspn(gov_str, "\n")] = 0;
            if (strcmp(gov_str, "performance") == 0) saved_gov = PERFORMANCE;
            else if (strcmp(gov_str, "powersave") == 0) saved_gov = POWERSAVE;
            else if (strcmp(gov_str, "ondemand") == 0) saved_gov = ONDEMAND;
            else if (strcmp(gov_str, "userspace") == 0) saved_gov = USERSPACE;
        }
        fclose(gov_file);
    } else {
        gov_file = fopen(GOVSAVE, "w");
        if (gov_file) {
            fprintf(gov_file, "ondemand");
            fclose(gov_file);
        }
    }

    current_cpu_freq = saved_freq;
    set_cpugovernor_optimized(saved_gov);
    set_cpuclock(saved_freq);
}

/* ------------------------------------------------------------------------- */
/*                          SETTINGS / JSON HELPERS                          */
/* ------------------------------------------------------------------------- */

void initializeSettingsFile(void) {
    if (cached_settings_file != NULL) return;

    const char *env_settings = getenv("SETTINGS_FILE");
    if (env_settings != NULL) {
        cached_settings_file = env_settings;
        return;
    }

    FILE* pipe = popen("dmesg | fgrep '[FSP] Flash is detected (0x1100, 0x68, 0x40, 0x18) ver1.1'", "r");
    if (!pipe) {
        FILE* configv4 = fopen("/appconfigs/system.json.old", "r");
        if (!configv4) {
            cached_settings_file = "/appconfigs/system.json";
        } else {
            cached_settings_file = "/mnt/SDCARD/system.json";
            fclose(configv4);
        }
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
            cached_settings_file = "/mnt/SDCARD/system.json";
        } else {
            cached_settings_file = "/appconfigs/system.json";
        }
    }
}

char* load_file(char const* path) {
    char* buffer = 0;
    long length = 0;
    FILE * f = fopen(path, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = (char*) malloc((length+1)*sizeof(char));
        if (buffer) {
            fread(buffer, sizeof(char), length, f);
        }
        fclose(f);
        buffer[length] = '\0';
    }
    return buffer;
}

int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

/* ----------------- CARGA Y GUARDADO DEL CACHE DE AJUSTES ----------------- */

void loadSettingsCache(void) {
    initializeSettingsFile();

    static time_t last_mtime = 0;

    struct stat st;
    if (stat(cached_settings_file, &st) != 0) {
        return;
    }

    /* Si está cargado y NO ha cambiado el fichero, no hacemos nada */
    if (sc.loaded && st.st_mtime == last_mtime) {
        return;
    }

    /* Si ha cambiado, actualizamos */
    last_mtime = st.st_mtime;

    /* limpiar JSON previo si existía */
    if (root_json) {
        cJSON_Delete(root_json);
        root_json = NULL;
    }

    /* recargar fichero */
    char *body = load_file(cached_settings_file);
    if (!body)
        return;

    root_json = cJSON_Parse(body);
    free(body);

    if (!root_json)
        return;

    /* cargar datos nuevos en la cache */
    #define LOAD_INT(key, target)                     \
        do {                                          \
            cJSON *obj = cJSON_GetObjectItem(root_json, key); \
            target = obj ? obj->valueint : target;    \
        } while(0)

    LOAD_INT("vol",        sc.vol);
    LOAD_INT("mute",       sc.mute);
    LOAD_INT("brightness", sc.brightness);
    LOAD_INT("hibernate",  sc.hibernate);
    LOAD_INT("wifi",       sc.wifi);
    LOAD_INT("audiofix",   sc.audiofix);

    sc.loaded = 1;
    sc.dirty = 0;
}

void saveSettingsCache(void) {
    if (!sc.loaded || !sc.dirty)
        return;

    sc.dirty = 0;

    if (!root_json)
        return;

    /* actualizar solo los campos modificados */
    cJSON_ReplaceItemInObject(root_json, "vol",        cJSON_CreateNumber(sc.vol));
    cJSON_ReplaceItemInObject(root_json, "mute",       cJSON_CreateNumber(sc.mute));
    cJSON_ReplaceItemInObject(root_json, "brightness", cJSON_CreateNumber(sc.brightness));
    cJSON_ReplaceItemInObject(root_json, "hibernate",  cJSON_CreateNumber(sc.hibernate));
    cJSON_ReplaceItemInObject(root_json, "wifi",       cJSON_CreateNumber(sc.wifi));
    cJSON_ReplaceItemInObject(root_json, "audiofix",   cJSON_CreateNumber(sc.audiofix));

    /* guardar el JSON completo con todos los demás campos intactos */
    char *text = cJSON_Print(root_json);
    if (text) {
        FILE *fp = fopen(cached_settings_file, "w");
        if (fp) {
            fputs(text, fp);
            fclose(fp);
        }
        free(text);
    }
}

/* ------------------------------------------------------------------------- */
/*                          HALL SENSOR / UTILS                              */
/* ------------------------------------------------------------------------- */

int read_hallvalue(const char* path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    int val = -1;
    if (fscanf(f, "%d", &val) != 1) val = -1;
    fclose(f);
    return val;
}

/* ------------------------------------------------------------------------- */
/*                          HEADPHONE VOL HELPERS                            */
/* ------------------------------------------------------------------------- */

static void saveHeadphoneVol(int vol) {
    FILE *f = fopen("/appconfigs/headvol", "w");
    if (f) {
        fprintf(f, "%d\n", vol);
        fclose(f);
    }
}

static int loadHeadphoneVol(void) {
    int vol = -1;
    FILE *f = fopen("/appconfigs/headvol", "r");
    if (f) {
        fscanf(f, "%d", &vol);
        fclose(f);
        return vol;
    }

    loadSettingsCache();
    return sc.vol;
}

static void saveSpeakerVol(int vol) {
    FILE *f = fopen("/appconfigs/spkvol", "w");
    if (f) {
        fprintf(f, "%d\n", vol);
        fclose(f);
    }
}

static int loadSpeakerVol(void) {
    int vol = -1;
    FILE *f = fopen("/appconfigs/spkvol", "r");
    if (f) {
        fscanf(f, "%d", &vol);
        fclose(f);
        return vol;
    }

    loadSettingsCache();
    return sc.vol;
}

static int loadVol(void) {
    loadSettingsCache();
    return sc.vol;
}

/* ------------------------------------------------------------------------- */
/*                            PROCESS CHECKERS                               */
/* ------------------------------------------------------------------------- */

int isProcessRunning(const char* processName) {
    FILE* fp;
    char cmd[64];
    char buffer[128];

    snprintf(cmd, sizeof(cmd), "pgrep %s", processName);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return 0;
    }

    int running = (fgets(buffer, sizeof(buffer), fp) != NULL) ? 1 : 0;
    pclose(fp);
    return running;
}

int isRetroarchRunning(void)  { return isProcessRunning("retroarch"); }
int isGMERunning(void)        { return isProcessRunning("gme_player"); }
int isGMURunning(void)        { return isProcessRunning("gmu.bin"); }
int isOpenborRunning(void)    { return isProcessRunning("OpenBOR"); }
int isDrasticRunning(void)    { return isProcessRunning("drastic"); }
int isFBNeoRunning(void)      { return isProcessRunning("fbneo"); }
int isPcsxRunning(void)       { return isProcessRunning("pcsx"); }
int isPico8Running(void)      { return isProcessRunning("pico8_dyn"); }
int isKeytesterRunning(void)  { return isProcessRunning("keytester_launcher"); }
int isSimpleMenuRunning(void) { return isProcessRunning("simplemenu"); }

int isDukemRunning(void) {
    const char *dukems[] = {"rednukem", "eduke32", "nblood", "voidsw"};
    for (unsigned int i = 0; i < sizeof(dukems) / sizeof(dukems[0]); i++) {
        if (isProcessRunning(dukems[i])) return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/*                            AUDIO / VOLUME                                 */
/* ------------------------------------------------------------------------- */

int getAudioFix(void) {
    loadSettingsCache();
    return sc.audiofix;
}

void setmute(int mute) {
    int fd = open("/dev/mi_ao", O_RDWR);
    if (fd >= 0) {
        int buf2[] = {0, mute};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
        ioctl(fd, MI_AO_SETMUTE, buf1);
        close(fd);
    }
    loadSettingsCache();
    if (sc.mute != mute) {
        sc.mute = mute;
        sc.dirty = 1;
        saveSettingsCache();
    }
}

static int saveVolume(int volume)
{
    loadSettingsCache();
    sc.vol = volume;
    sc.dirty = 1;
    saveSettingsCache();
    return volume;
}

int setVolumeRaw(int volume, int add, int tiny) {
    int recent_volume = 0;
    int fd = open("/dev/mi_ao", O_RDWR);

    loadSettingsCache();

    int vol  = sc.vol;
    int mute = sc.mute;
    int fix  = sc.audiofix;

    if (fd >= 0) {
        int buf2[] = {0, 0};
        uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
        ioctl(fd, MI_AO_GETVOLUME, buf1);
        recent_volume = buf2[1];

        if (add != 0) {
            buf2[1] += add;
            if (buf2[1] > 9) buf2[1] = 9;
            else if (buf2[1] < -60) buf2[1] = -60;
        } else {
            buf2[1] = volume;
        }

        if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
        close(fd);
    }

    if (fix == 0) {
        char command[100];
        int tinyvol;
        if (add != 0) {
            tinyvol = tiny + add;
            if (tinyvol >= 109) tinyvol = 109;
            else if (tinyvol <= 40) tinyvol = 40;
        } else {
            tinyvol = tiny;
        }
        sprintf(command, "tinymix set 6 %d", tinyvol);
        system(command);
    }
	
	if (add == 3 && vol < 23) vol++;
    if (add == -3 && vol > 0) vol--;

    if (add != 0) {
        int oldmute = mute;
        sc.vol = vol;
        if (vol == 0) {
            sc.mute = 1;
            mute = 1;
        }
        if (vol != 0) {
            sc.mute = 0;
            mute = 0;
        }
        sc.dirty = 1;
        saveSettingsCache();
        if (mute != oldmute) {
            setmute(mute);
        }
    }
	
	volume = sc.vol;

    return volume;
}

int setVolume(int volume, int add) {
    loadSettingsCache();
    int recent_volume = sc.vol;

    int rawVolumeValue;
    int rawAdd;
    int rawTiny;

    if (recent_volume != volume) {
        rawVolumeValue = (volume * 3) - 60;
        rawAdd = add * 3;
        rawTiny = (volume * 3) + 40;
    } else {
        rawVolumeValue = (recent_volume * 3) - 60;
        rawAdd = add * 3;
        rawTiny = (recent_volume * 3) + 40;
    }

    recent_volume = setVolumeRaw(rawVolumeValue, rawAdd, rawTiny);
    return recent_volume;
}

int getVolume() {
    int recent_volume = 0;
    int fd = open("/dev/mi_ao", O_RDWR);

    loadSettingsCache();

    int vol      = sc.vol;
    int mute     = sc.mute;
    int audiofix = sc.audiofix;

    if (mute == 0) {
        if (audiofix == 1) {
            if (fd >= 0) {
                int buf2[] = {0, 0};
                uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
                ioctl(fd, MI_AO_GETVOLUME, buf1);
                recent_volume = ((vol * 3) - 60);
                buf2[1] = recent_volume;
                ioctl(fd, MI_AO_SETVOLUME, buf1);
                close(fd);
            }
        } else if (audiofix == 0) {
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
            sprintf(command, "tinymix set 6 %d", tiny);
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

    return vol;
}

/* ------------------------------------------------------------------------- */
/*                           GPIO HELPERS (SPEAKER / HP)                     */
/* ------------------------------------------------------------------------- */

static void gpio_export_if_needed(int n) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", n);
    if (access(path, F_OK) != 0) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", n);
        int fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd >= 0) {
            write(fd, buf, strlen(buf));
            close(fd);
        }
        usleep(20000);
    }
}

static int detect_headphones(void) {
    gpio_export_if_needed(45);

    if (hp_fd < 0) {
        int fd = open("/sys/class/gpio/gpio45/direction", O_WRONLY);
        if (fd >= 0) {
            write(fd, "in", 2);
            close(fd);
        }

        hp_fd = open("/sys/class/gpio/gpio45/value", O_RDONLY);
        if (hp_fd < 0)
            return 0;
    }

    lseek(hp_fd, 0, SEEK_SET);

    char c = '1';
    read(hp_fd, &c, 1);

    return (c == '0');
}

static void speaker_set(int on) {
    gpio_export_if_needed(44);
    int fd = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
    if (fd >= 0) {
        write(fd, "out", 3);
        close(fd);
    }

    fd = open("/sys/class/gpio/gpio44/value", O_WRONLY);
    if (fd < 0) return;
    if (on)
        write(fd, "1", 1);
    else
        write(fd, "0", 1);
    close(fd);
}

/* Máquina de estados para auriculares: sólo actúa cuando cambia el estado */
static void updateHeadphoneState(HeadphoneState new_state)
{
    if (new_state == hp_state)
        return;  // No repetir acciones si no hay cambio real

    hp_state = new_state;

    if (new_state == HP_STATE_CONNECTED) {
        speaker_set(0);
		
		int spk = loadVol();
        saveSpeakerVol(spk);

        int hv = loadHeadphoneVol();
        if (hv > 23) hv = 23;

        saveVolume(hv);
        setVolume(hv, 0);
        osd_headphones_connected = 1;
        iconvol();
        osd_show(OSD_VOLUME);
    }
    else if (new_state == HP_STATE_DISCONNECTED) {
		int hv = loadHeadphoneVol();
        saveHeadphoneVol(hv);

        int spk = loadSpeakerVol();
        if (spk > 20) spk = 20;

        saveVolume(spk);
        speaker_set(1);
        setVolume(spk, 0);

        osd_headphones_connected = 0;
        iconvol();
        osd_show(OSD_VOLUME);
    }
}

/* ------------------------------------------------------------------------- */
/*                           OSD / BRIGHTNESS / HIBERNATE                    */
/* ------------------------------------------------------------------------- */

static int iconvol(void) {
    loadSettingsCache();
    int vol = sc.vol;
    osd_volume = (vol * 3);
    return osd_volume;
}

void modifyBrightness(int inc) {
    loadSettingsCache();
    int brightness = sc.brightness;

    if (inc == 1 && brightness < BRIMAX) brightness++;
    if (inc == -1 && brightness > BRIMIN) brightness--;

    if (inc != 0) {
        sc.brightness = brightness;
        sc.dirty = 1;
        saveSettingsCache();
    }

    int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
    if (fd >= 0) {
        dprintf(fd, "%d", brightness * 10);
        osd_brightness = brightness;
        close(fd);
    }
}

void sethibernate(int hibernate) {
    loadSettingsCache();
    sc.hibernate = hibernate;
    sc.dirty = 1;
    saveSettingsCache();
}

/* ------------------------------------------------------------------------- */
/*                         PROCESS STOP/CONT FOR SLEEP                       */
/* ------------------------------------------------------------------------- */

void stopOrContinueProcesses(int value) {
    mmModel = access("/customer/app/axp_test", F_OK);

    const char *exceptions_base[] = {
        "batmon", "keymon", "init", "gmu.bin", "gme_player", "sh",
        "retroarch", "OpenBOR", "drastic", "fbneo",
        "pico8_dyn", "simplemenu", "pcsx", "keytester_launcher",
        "shutdown", "_shutdown", "nohup", "killall",
        "pkill", "umount", "cpuclock", "hwclock", "tinymix",
        "swapoff", "sync", "reboot", "poweroff"
    };

    const char *exceptions_music[] = {
        "batmon", "keymon", "init", "gmu.bin", "sh",
        "simplemenu","shutdown", "_shutdown", "nohup", "killall",
        "pkill", "umount", "cpuclock", "hwclock", "tinymix",
        "swapoff", "sync", "reboot", "poweroff",
        "wpa_supplicant", "udhcpc"
    };

    const char **exceptions;
    int exceptions_count;

    if (!mmModel && isGMURunning()) {
        exceptions = exceptions_music;
        exceptions_count = sizeof(exceptions_music) / sizeof(exceptions_music[0]);
    } else {
        exceptions = exceptions_base;
        exceptions_count = sizeof(exceptions_base) / sizeof(exceptions_base[0]);
    }

    const char *cmdType = (value == 0) ? "STOP" : "CONT";

    DIR *dir;
    struct dirent *ent;
    dir = opendir("/proc");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (isdigit(ent->d_name[0])) {
                int pid = atoi(ent->d_name);
                char cmdline_path[256];
                snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);

                FILE *cmdline_file = fopen(cmdline_path, "r");
                if (cmdline_file != NULL) {
                    char cmdline[1024];
                    if (fgets(cmdline, sizeof(cmdline), cmdline_file) != NULL) {

                        int shouldSkip = 0;
                        for (int i = 0; i < exceptions_count; i++) {
                            if (strstr(cmdline, exceptions[i]) != NULL) {
                                shouldSkip = 1;
                                break;
                            }
                        }

                        if (!shouldSkip) {
                            char stopCmd[128];
                            snprintf(stopCmd, sizeof(stopCmd), "kill -%s %d", cmdType, pid);
                            system(stopCmd);
                        }
                    }
                    fclose(cmdline_file);
                }
            }
        }
        closedir(dir);
    }
}

/* ------------------------------------------------------------------------- */
/*                                WIFI                                       */
/* ------------------------------------------------------------------------- */

int isWifiEnabled() {
    loadSettingsCache();
    return sc.wifi;
}

void wifiOff() {
    system("ifconfig wlan0 down");
    usleep(200000);
    system("/customer/app/axp_test wifioff &");
}

void wifiOn() {
    system("/customer/app/axp_test wifion &");
    usleep(200000);
    system("ifconfig wlan0 up");
}

void wifiReconnectIfNeeded() {
    if (!wifi_was_enabled_before_sleep)
        return;

    if (!file_exists("/appconfigs/wpa_supplicant.conf"))
        return;

    system("wpa_supplicant -B -D nl80211 -i wlan0 -c /appconfigs/wpa_supplicant.conf");
    usleep(500000);
    system("udhcpc -i wlan0 -s /etc/init.d/udhcpc.script &");
    usleep(1000000);
}

/* ------------------------------------------------------------------------- */
/*                             DISPLAY SLEEP/WAKE                             */
/* ------------------------------------------------------------------------- */

void display_setScreen(int value) {

    if (value == 0) {
        if (isRetroarchRunning() == 1) {
            system("pkill -STOP retroarch");
        }

        if (isOpenborRunning() == 1) {
            system("pkill -STOP OpenBOR");
        }

        if (isDrasticRunning() == 1) {
            system("pkill -STOP drastic");
        }

        if (isFBNeoRunning() == 1) {
            system("pkill -STOP fbneo");
        }

        if (isPcsxRunning() == 1) {
            system("pkill -STOP pcsx");
        }

        if (isPico8Running() == 1) {
            system("pkill -STOP pico8_dyn");
        }

        if (isKeytesterRunning() == 1) {
            system("pkill -STOP keytester_launcher");
        }

        if (isSimpleMenuRunning() == 1) {
            system("pkill -STOP simplemenu");
        }

        stopOrContinueProcesses(0);

        FILE *file;
        if ((file = fopen("/sys/class/gpio/export", "w"))) {
            fprintf(file, "4\n");
            fclose(file);
        }

        if ((file = fopen("/sys/class/gpio/gpio4/direction", "w"))) {
            fprintf(file, "out\n");
            fclose(file);
        }

        if ((file = fopen("/sys/class/gpio/gpio4/value", "w"))) {
            fprintf(file, "0\n");
            fclose(file);
        }

        if ((file = fopen("/sys/class/pwm/pwmchip0/pwm0/enable", "w"))) {
            fprintf(file, "0\n");
            fclose(file);
        }

        if ((file = fopen("/sys/module/gpio_keys_polled/parameters/button_enable", "w"))) {
            fprintf(file, "0\n");
            fclose(file);
        }

        if ((file = fopen("/proc/mi_modules/fb/mi_fb0", "w"))) {
            fprintf(file, "GUI_SHOW 0 off\n");
            fclose(file);
        }

    } else if (value == 1) {
        FILE *file;
        if ((file = fopen("/proc/mi_modules/fb/mi_fb0", "w"))) {
            fprintf(file, "GUI_SHOW 0 on\n");
            fclose(file);
        }

        if ((file = fopen("/sys/module/gpio_keys_polled/parameters/button_enable", "w"))) {
            fprintf(file, "1\n");
            fclose(file);
        }

        if ((file = fopen("/sys/class/gpio/gpio4/value", "w"))) {
            fprintf(file, "1\n");
            fclose(file);
        }

        if ((file = fopen("/sys/class/gpio/unexport", "w"))) {
            fprintf(file, "4\n");
            fclose(file);
        }

        if ((file = fopen("/sys/class/pwm/pwmchip0/pwm0/enable", "w"))) {
            fprintf(file, "1\n");
            fclose(file);
        }

        usleep(20000);
        stopOrContinueProcesses(1);

        if (isRetroarchRunning() == 1) {
            system("pkill -CONT retroarch");
        }

        if (isOpenborRunning() == 1) {
            system("pkill -CONT OpenBOR");
        }

        if (isDrasticRunning() == 1) {
            system("pkill -CONT drastic");
        }

        if (isFBNeoRunning() == 1) {
            system("pkill -CONT fbneo");
        }

        if (isPcsxRunning() == 1) {
            system("pkill -CONT pcsx");
        }

        if (isPico8Running() == 1) {
            system("pkill -CONT pico8_dyn");
        }

        if (isKeytesterRunning() == 1) {
            system("pkill -CONT keytester_launcher");
        }

        if (isSimpleMenuRunning() == 1) {
            system("pkill -CONT simplemenu");
        }
    }
}

/* ------------------------------------------------------------------------- */
/*                              KILL RETROARCH                               */
/* ------------------------------------------------------------------------- */

void killRetroArch() {
    FILE *fp;
    char buffer[128];
    fp = popen("pgrep retroarch", "r");
    if (fp != NULL) {
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            int retroarch_pid = atoi(buffer);
            if (retroarch_pid > 0) {
                kill(retroarch_pid, SIGTERM);
                usleep(2000000);
                if (kill(retroarch_pid, 0) == 0) {
                    kill(retroarch_pid, SIGKILL);
                    usleep(2000000);
                    if (isProcessRunning("simplemenu")){
                        system("pkill -TERM simplemenu");
                    }
                }
            }
        }
        pclose(fp);
    }
}

/* ------------------------------------------------------------------------- */
/*                          CPU PROFILE SELECTION                            */
/* ------------------------------------------------------------------------- */

void setcpu_optimized(int cpu) {
    if (cpu == 0) {
        restore_cpu_config();

        if (isDrasticRunning() == 1) {
            int speed;
            const char* settings = "/mnt/SDCARD/App/drastic/resources/settings.json";
            const char* maxcpu = "maxcpu";

            jfile = json_object_from_file(settings);
            if (jfile && json_object_object_get_ex(jfile, maxcpu, &jval)) {
                speed = json_object_get_int(jval);
                if (speed >= 400 && speed <= 1600) {
                    current_cpu_freq = speed * 1000;
                    set_cpugovernor_optimized(USERSPACE);
                    set_cpuclock(current_cpu_freq);
                }
            }
            if (jfile) json_object_put(jfile);
        }

        if (isPcsxRunning() == 1) {
            current_cpu_freq = 1400000;
            set_cpugovernor_optimized(USERSPACE);
            set_cpuclock(1400000);
        }

        if (isFBNeoRunning() == 1) {
            current_cpu_freq = 1400000;
            set_cpugovernor_optimized(USERSPACE);
            set_cpuclock(1400000);
        }

        if (isPico8Running() == 1) {
            int speed;
            const char* settings = "/mnt/SDCARD/App/cfg/pico/korikicf.json";
            const char* maxcpu = "cpuclock";

            jfile = json_object_from_file(settings);
            if (jfile) {
                json_object* performanceObject = json_object_object_get(jfile, "performance");
                if (performanceObject && json_object_object_get_ex(performanceObject, maxcpu, &jval)) {
                    speed = json_object_get_int(jval);
                    if (speed >= 400 && speed <= 1600) {
                        current_cpu_freq = speed * 1000;
                        set_cpugovernor_optimized(USERSPACE);
                        set_cpuclock(current_cpu_freq);
                    }
                }
                json_object_put(jfile);
            }
        }

        cpu_config_saved = 0;

    } else {
        if (!cpu_config_saved) {
            save_current_cpu_config();
        }

        switch (cpu) {
            case 1:
                current_cpu_freq = 600000;
                set_cpugovernor_optimized(POWERSAVE);
                set_cpuclock(600000);
                break;

            case 2:
                current_cpu_freq = 1000000;
                set_cpugovernor_optimized(ONDEMAND);
                set_cpuclock(1000000);
                break;

            case 3:
                current_cpu_freq = 600000;
                set_cpugovernor_optimized(USERSPACE);
                set_cpuclock(600000);
                break;

            default:
                break;
        }
    }

    sync();
}

/* ------------------------------------------------------------------------- */
/*                                  MAIN                                     */
/* ------------------------------------------------------------------------- */

int main (int argc, char *argv[]) {
    input_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    mmModel = access("/customer/app/axp_test", F_OK);

    const char *hallvalue_path = "/sys/devices/soc0/soc/soc:hall-mh248/hallvalue";
    int last_hallvalue = -1;

    initializeSettingsFile();
    loadSettingsCache();

    /* Estado inicial de auriculares (sin disparar acciones extra) */
    int hp_init = detect_headphones();
    osd_headphones_connected = 0;
    hp_state = hp_init ? HP_STATE_CONNECTED : HP_STATE_DISCONNECTED;

    getVolume();
    modifyBrightness(0);
    setcpu_optimized(0);
    sethibernate(0);
    setmute(0);
    setVolume(0,0);

    int volume = sc.vol;
    int sleep = 0;

    register uint32_t val;
    register uint32_t menu_pressed = 0;
    register uint32_t l2_pressed = 0;
    register uint32_t r2_pressed = 0;
    register uint32_t Select_pressed = 0;
    register uint32_t Start_pressed = 0;
    register uint32_t power_pressed = 0;
    int repeat_power = 0;
    int shutdown = 0;
    int close = 0;
    uint32_t repeat = 0;
    ssize_t n;

    while (1) {
        n = read(input_fd, &ev, sizeof(ev));

        if (n == sizeof(ev)) {
            val = ev.value;

            if ((ev.type != EV_KEY) || (val > REPEAT)) continue;

            switch (ev.code) {
                case BUTTON_POWER:
                    if (val == PRESSED) {
                        power_pressed = val;
                    } else if (val == RELEASED && power_pressed) {
                        if (isKeytesterRunning() == 0) {
                            if (sleep == 0) {
                                display_setScreen(0);

                                wifi_was_enabled_before_sleep = isWifiEnabled();

                                if (isGMERunning() == 1 || isGMURunning() == 1) {
                                    setmute(0);
                                } else {
                                    setmute(1);
                                }

                                sethibernate(1);
                                if (!mmModel && wifi_was_enabled_before_sleep && !isGMURunning()) {
                                    wifiOff();
                                }

                                if (isGMERunning() == 1 || isGMURunning() == 1) {
                                    setcpu_optimized(2);
                                } else if (isRetroarchRunning() == 1) {
                                    setcpu_optimized(1);
                                } else if (isDrasticRunning() == 1) {
                                    setcpu_optimized(3);
                                } else if (isPcsxRunning() == 1) {
                                    setcpu_optimized(3);
                                } else if (isFBNeoRunning() == 1) {
                                    setcpu_optimized(3);
                                } else if (isPico8Running() == 1) {
                                    setcpu_optimized(3);
                                } else {
                                    setcpu_optimized(1);
                                }

                                power_pressed = 0;
                                repeat_power = 0;
                                sleep = 1;

                            } else if (sleep == 1 && close == 0) {
                                setmute(0);
                                sethibernate(0);
                                setcpu_optimized(0);
                                if (!mmModel && wifi_was_enabled_before_sleep && !isGMURunning()) {
                                    wifiOn();
                                    wifiReconnectIfNeeded();
                                }
                                if (!isGMERunning() || !isGMURunning()) {
                                    getVolume();
                                }

                                display_setScreen(1);
                                power_pressed = 0;
                                repeat_power = 0;
                                sleep = 0;
                            }
                        }
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
                        val = repeat;
                        repeat ^= PRESSED;
                    } else {
                        repeat = 0;
                    }

                    if (isSimpleMenuRunning() == 1 || isRetroarchRunning() == 1 || isDukemRunning() == 1 || isPico8Running() == 1) {
                        if (val == PRESSED && menu_pressed) {
                            modifyBrightness(1);
                            osd_show(OSD_BRIGHTNESS);
                        }
                    } else {
                        if (val == PRESSED && Select_pressed) {
                            modifyBrightness(1);
                            osd_show(OSD_BRIGHTNESS);
                        }
                    }
                    break;

                case BUTTON_DOWN:
                    if (val == REPEAT) {
                        val = repeat;
                        repeat ^= PRESSED;
                    } else {
                        repeat = 0;
                    }

                    if (isSimpleMenuRunning() == 1 || isRetroarchRunning() == 1 || isDukemRunning() == 1 || isPico8Running() == 1) {
                        if (val == PRESSED && menu_pressed) {
                            modifyBrightness(-1);
                            osd_show(OSD_BRIGHTNESS);
                        }
                    } else {
                        if (val == PRESSED && Select_pressed) {
                            modifyBrightness(-1);
                            osd_show(OSD_BRIGHTNESS);
                        }
                    }
                    break;

                case BUTTON_RIGHT:
                    if (mmModel) {
                        if (val == REPEAT) {
                            val = repeat;
                            repeat ^= PRESSED;
                        } else {
                            repeat = 0;
                        }

                        if (isSimpleMenuRunning() == 1 || isRetroarchRunning() == 1 || isDukemRunning() == 1 || isPico8Running() == 1) {
                            if (val == PRESSED && menu_pressed) {
                                loadSettingsCache();
                                volume = sc.vol;
								setVolume(volume, 1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        } else {
                            if (val == PRESSED && Select_pressed) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, 1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        }
                    }
                    break;

                case BUTTON_LEFT:
                    if (mmModel) {
                        if (val == REPEAT) {
                            val = repeat;
                            repeat ^= PRESSED;
                        } else {
                            repeat = 0;
                        }

                        if (isSimpleMenuRunning() == 1 || isRetroarchRunning() == 1 || isDukemRunning() == 1 || isPico8Running() == 1) {
                            if (val == PRESSED && menu_pressed) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, -1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        } else {
                            if (val == PRESSED && Select_pressed) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, -1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        }
                    }
                    break;

                case BUTTON_VOLUMEUP:
                    if (val == REPEAT) {
                        val = repeat;
                        repeat ^= PRESSED;
                    } else {
                        repeat = 0;
                    }

                    if (isKeytesterRunning() == 0) {
                        if (isSimpleMenuRunning() == 1 || isRetroarchRunning() == 1 || isDukemRunning() == 1 || isPico8Running() == 1) {
                            if (val == PRESSED && menu_pressed) {
                                modifyBrightness(1);
                                osd_show(OSD_BRIGHTNESS);
                            } else if (val == PRESSED) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, 1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        } else {
                            if (val == PRESSED && Select_pressed) {
                                modifyBrightness(1);
                                osd_show(OSD_BRIGHTNESS);
                            } else if (val == PRESSED) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, 1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        }
                    }
                    break;

                case BUTTON_VOLUMEDOWN:
                    if (val == REPEAT) {
                        val = repeat;
                        repeat ^= PRESSED;
                    } else {
                        repeat = 0;
                    }

                    if (isKeytesterRunning() == 0) {
                        if (isSimpleMenuRunning() == 1 || isRetroarchRunning() == 1 || isDukemRunning() == 1 || isPico8Running() == 1) {
                            if (val == PRESSED && menu_pressed) {
                                modifyBrightness(-1);
                                osd_show(OSD_BRIGHTNESS);
                            } else if (val == PRESSED) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, -1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        } else {
                            if (val == PRESSED && Select_pressed) {
                                modifyBrightness(-1);
                                osd_show(OSD_BRIGHTNESS);
                            } else if (val == PRESSED) {
                                loadSettingsCache();
                                volume = sc.vol;
                                setVolume(volume, -1);
                                iconvol();
                                osd_show(OSD_VOLUME);
                            }
                        }
                    }
                    break;

                case BUTTON_L2:
                    if (val != REPEAT) l2_pressed = val;
                    break;

                case BUTTON_R2:
                    if (val != REPEAT) r2_pressed = val;
                    break;

                case BUTTON_SELECT:
                    if (val != REPEAT) Select_pressed = val;
                    break;

                case BUTTON_START:
                    if (val != REPEAT) Start_pressed = val;
                    break;

                default:
                    break;
            }

            if (menu_pressed && l2_pressed && r2_pressed && Select_pressed && Start_pressed) {
                killRetroArch();
            }

            if (shutdown) {
                power_pressed = 0;
                unlink("/mnt/SDCARD/.simplemenu/NUL");
                unlink("/mnt/SDCARD/.simplemenu/apps/NUL");
                unlink("/mnt/SDCARD/.simplemenu/launchers/NUL");
                system("date -u +\"%Y-%m-%d %H:%M:%S\" > /mnt/SDCARD/App/Clock/time.txt");

                if (mmModel) {
                    if (isProcessRunning("retroarch")) {
                        system("echo MM_in_RA; touch /tmp/shutdowning; pkill -TERM retroarch; sleep 2; pkill -TERM simplemenu; sync; sleep 3; shutdown -r");
                    } else if (isProcessRunning("simplemenu") != 1) {
                        system("echo MM; touch /tmp/shutdowning; sync; sleep 3; shutdown -r");
                    } else {
                        system("echo MM_in_SM; touch /tmp/shutdowning; pkill -TERM simplemenu; sync; sleep 3; shutdown -r");
                    }
                } else {
                    if (isProcessRunning("retroarch")) {
                        system("echo MMP_in_RA; touch /tmp/shutdowning; pkill -TERM retroarch; sleep 2; pkill -TERM simplemenu; sync; sleep 3; shutdown");
                    } else if (isProcessRunning("simplemenu") != 1) {
                        system("echo MMP; sync; touch /tmp/shutdowning; sleep 3; shutdown");
                    } else {
                        system("echo MMP_in_SM; touch /tmp/shutdowning; pkill -TERM simplemenu; sync; sleep 3; shutdown");
                    }
                }
                while (1) pause();
            }
        }

        /* JACK DE AURICULARES: sólo actuamos cuando cambie el estado */
        {
            int hp = detect_headphones();
            if (hp == 1)
                updateHeadphoneState(HP_STATE_CONNECTED);
            else
                updateHeadphoneState(HP_STATE_DISCONNECTED);
        }

        if (!file_exists("/tmp/shutdowning")) {
            int hv = read_hallvalue(hallvalue_path);
            if (hv != -1 && hv != last_hallvalue) {
                last_hallvalue = hv;
                if (hv == 1 && sleep == 1) {
                    setmute(0);
                    sethibernate(0);
                    setcpu_optimized(0);
                    if (!mmModel && wifi_was_enabled_before_sleep && !isGMURunning()) {
                        wifiOn();
                        wifiReconnectIfNeeded();
                    }
                    if (!isGMERunning() || !isGMURunning()) {
                        getVolume();
                    }

                    display_setScreen(1);
                    power_pressed = 0;
                    repeat_power = 0;
                    sleep = 0;
                    close = 0;
                } else if (hv == 0 && sleep == 0) {
                    display_setScreen(0);

                    wifi_was_enabled_before_sleep = isWifiEnabled();

                    if (isGMERunning() == 1 || isGMURunning() == 1) {
                        setmute(0);
                    } else {
                        setmute(1);
                    }

                    sethibernate(1);
                    if (!mmModel && wifi_was_enabled_before_sleep && !isGMURunning()) {
                        wifiOff();
                    }
                    if (isGMERunning() == 1 || isGMURunning() == 1) {
                        setcpu_optimized(2);
                    } else if (isRetroarchRunning() == 1) {
                        setcpu_optimized(1);
                    } else if (isDrasticRunning() == 1) {
                        setcpu_optimized(3);
                    } else if (isPcsxRunning() == 1) {
                        setcpu_optimized(3);
                    } else if (isFBNeoRunning() == 1) {
                        setcpu_optimized(3);
                    } else if (isPico8Running() == 1) {
                        setcpu_optimized(3);
                    } else {
                        setcpu_optimized(1);
                    }

                    sleep = 1;
                    close = 1;
                }
            }
        }

        if (sleep == 1) {
            if (isGMERunning() || isGMURunning()) {
                usleep(50000);
            } else {
                usleep(1000000);
            }
        } else {
            usleep(50000);
        }
    }

    exit(EXIT_FAILURE);
}
