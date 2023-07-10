#include "../headers/globals.h"
#ifndef SCREEN_DEFINED
#define SCREEN_DEFINED
void updateScreen(struct Node *node);
void drawHeader();
void drawGameList();
void drawFooter(char *text);
void setupKeys();
void setupDecorations();
void freeResources();
void displayGamePicture();
void displayBackgroundPicture();
void showConsole();
void drawShutDownScreen();
uint32_t hideFullScreenModeMenu();
void resetPicModeHideMenuTimer();
void resetPicModeHideLogoTimer();
void clearPicModeHideMenuTimer();
void clearPicModeHideLogoTimer();
void clearHideHeartTimer();
void resetHideHeartTimer();
void drawSpecialScreen();
void drawHelpScreen(int page);
void clearShutdownTimer();
void resetShutdownTimer();
void startBatteryTimer();
void clearBatteryTimer();
void drawLoadingText();
void drawCopyingText();
void resizeSectionBackground(struct MenuSection *section);
void initializeFonts();
void freeResources();
void freeFonts();
void freeSettingsFonts();
void initializeSettingsFonts();

#endif
