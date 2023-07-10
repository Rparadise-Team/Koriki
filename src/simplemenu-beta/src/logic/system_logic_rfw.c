#include <fcntl.h> //for battery
#include <linux/soundcard.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include "../headers/system_logic.h"
#include "../headers/globals.h"

volatile uint32_t *memregs;
int32_t memdev = 0;
int oldCPU;

void setCPU(uint32_t mhz) {
    if (memdev > 0) {
        uint32_t m = mhz / 6;
        memregs[0x10 >> 2] = (m << 24) | 0x090520;
    }
}

int getBacklight() {
	char buf[32] = "-1";
	FILE *f = fopen("/proc/jz/backlight", "r");
	if (f) {
		fgets(buf, sizeof(buf), f);
	}
	fclose(f);
	return atoi(buf);
}

void setBacklight(int level) {
	char buf[200] = {0};
	sprintf(buf, "echo %d > /proc/jz/backlight", level);
	system(buf);
}

void clearTimer() {
	if (timeoutTimer != NULL) {
		SDL_RemoveTimer(timeoutTimer);
	}
	timeoutTimer = NULL;
}

uint32_t suspend() {
	if(timeoutValue!=0) {
		clearTimer();
		backlightValue = getBacklight();
		oldCPU=currentCPU;
		setBacklight(0);
		setCPU(OC_SLEEP);
		isSuspended=1;
	}
	return 0;
};

void resetScreenOffTimer() {
#ifndef TARGET_PC
	if(isSuspended) {
		setCPU(OC_NO);
		setBacklight(backlightValue);
		currentCPU=oldCPU;
		isSuspended=0;
	}
	clearTimer();
	timeoutTimer=SDL_AddTimer(timeoutValue * 1e3, suspend, NULL);
#endif
}

void initSuspendTimer() {
	timeoutTimer=SDL_AddTimer(timeoutValue * 1e3, suspend, NULL);
	isSuspended=0;
	logMessage("INFO","Suspend timer initialized");
}

void HW_Init() {
    uint32_t soundDev = open("/dev/mixer", O_RDWR);
    int32_t vol = (100 << 8) | 100;

    /* Init memory registers, pretty much required for anthing RS-97 specific */
	memdev = open("/dev/mem", O_RDWR);

	if (memdev > 0) {
	    memregs = (uint32_t*)mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED, memdev, 0x10000000);
	    if (memregs == MAP_FAILED) {
	        close(memdev);
	    }
	}

//    /* Setting Volume to max, that will avoid issues, i think */
    ioctl(soundDev, SOUND_MIXER_WRITE_VOLUME, &vol);
    close(soundDev);
	logMessage("INFO","HW Initialized");
}

int getBatteryLevel() {
	int val = -1;
	FILE *f = fopen("/proc/jz/battery", "r");
	fscanf(f, "%i", &val);
	fclose(f);
	char temp[30];
	sprintf( temp, "%d", val );
	logMessage("INFO", "getBatteryLevel", val);
	if ((val > 10000) || (val < 0)) return 6;
	if (val > 4000) return 5; // 100%
	if (val > 3900) return 4; // 80%
	if (val > 3800) return 3; // 60%
	if (val > 3700) return 2; // 40%
	if (val > 3520) return 1; // 20%
	return 0;
}
