#ifndef SYSTEM_LOGIC
#define SYSTEM_LOGIC
void setCPU(uint32_t mhz);
void HW_Init();
void cycleFrequencies();
void resetScreenOffTimer();
void clearTimer();
uint32_t suspend();
void initSuspendTimer();
void setBacklight(int level);
int getBacklight();
void resetScreenOffTimer();
int getBatteryLevel();
void rumble();
int getCurrentBrightness();
int getMaxBrightness();
int getCurrentWifi();
void setBrightness(int value);
int getCurrentVolume();
int setVolumeRaw(int volume, int add, int tiny);
int setVolume(int volume, int add);
void setMute(int mute);
int getCurrentSystemValue(char const *key);
void setSystemValue(char const *key, int value);
void startmusic();
void stopmusic();
void Luma(int dev, int value);
void Hue(int dev, int value);
void Saturation(int dev, int value);
void Contrast(int dev, int value);
#endif
