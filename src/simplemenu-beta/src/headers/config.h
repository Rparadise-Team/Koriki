#include "../headers/globals.h"
#ifndef CONFIG
#define CONFIG
void saveLastState();
void loadLastState();
void saveFavorites();
void loadFavorites();
int loadSections();
void loadConfig();
void loadAliasList(int sectionNumber);
void createConfigFilesInHomeIfTheyDontExist();
void checkIfDefault();
void loadTheme(char *theme);
void loadSectionGroups();
void checkThemes();
struct AutostartRom *getLaunchAtBoot();
void saveRomPreferences(struct Rom *rom);
void setLaunchAtBoot(struct Rom *rom);
void loadRomPreferences(struct Rom *rom);
int isLaunchAtBoot(char *romName);
void setRunningFlag();
void loadRomPreferences(struct Rom *rom);
int wasRunningFlag();
void createThemesInHomeIfTheyDontExist();
#endif
