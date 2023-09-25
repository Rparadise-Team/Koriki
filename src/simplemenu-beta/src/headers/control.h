#ifndef CONTROL_DEFINED
#define CONTROL_DEFINED
#include <SDL/SDL_events.h>
#include "../headers/globals.h"
int advanceSection();
int rewindSection();
void showPicture();
void launchGame();
void launchEmulator();
void scrollUp();
void scrollDown();
void scrollToGame(int gameNumber);
void advancePage();
void rewindPage();
void showOrHideFavorites();
void removeFavorite();
void markAsFavorite();
int isSelectPressed();
void performChoosingAction();
void performGroupChoosingAction();
void performSettingsChoosingAction();
void performScreenSettingsChoosingAction();
void performAppearanceSettingsChoosingAction();
void performSystemSettingsChoosingAction();
void performHelpAction();
int performAction();
void callDeleteGame(struct Rom * rom);
void performLaunchAtBootQuitScreenChoosingAction();
void launchAutoStartGame(struct Rom *rom, char *emuDir, char *emuExec);
#endif