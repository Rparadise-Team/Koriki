#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>

#include "../headers/config.h"
#include "../headers/control.h"
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/graphics.h"
#include "../headers/input.h"
#include "../headers/logic.h"
#include "../headers/screen.h"
#include "../headers/string_utils.h"
#include "../headers/system_logic.h"
#include "../headers/utils.h"


void initializeGlobals() {
        running=1;
	currentSectionNumber=0;
	gamesInPage=0;
	CURRENT_SECTION.totalPages=0;
	MAX_GAMES_IN_SECTION=500000;
	favoritesSectionNumber=0;
	favoritesSize=0;
	favoritesChanged=0;
	FULLSCREEN_ITEMS_PER_PAGE=12;
	MENU_ITEMS_PER_PAGE=10;
	ITEMS_PER_PAGE=MENU_ITEMS_PER_PAGE;
	isPicModeMenuHidden=1;
	footerVisibleInFullscreenMode=1;
	menuVisibleInFullscreenMode=1;
	stripGames=1;
	srand(time(0));
	#if defined MIYOOMINI
	brightness = getCurrentBrightness();
	loadConfiguration1();
	loadConfiguration2();
	loadConfiguration3();
	if (!mmModel) {
		loadConfiguration4();
    }
    #endif
}

void resetFrameBuffer () {
	int ret = system("./scripts/reset_fb");
	if (ret==-1) {
		generateError("FATAL ERROR", 1);
	}
	logMessage("INFO","resetFrameBuffer","Reset Framebuffer");
}

void critical_error_handler()
{
	logMessage("ERROR","critical_error_handler","Nice, a critical error!!!");
	closeLogFile();
	char command[100];
	snprintf(command, sizeof(command), "rm %s/.simplemenu/last_state.sav && sync", getenv("HOME"));
	system(command);
	if (musicEnabled) {
		stopmusic();
	}
	freeResources();
	exit(0);
}

void sig_term_handler()
{
	logMessage("WARN","sig_term_handler","Received SIGTERM");
	running=0;
}

void initialSetup(int w, int h) {
	initializeGlobals();
	logMessage("INFO","initialSetup","Initialized Globals");
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = critical_error_handler;
	sa.sa_flags   = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	signal(SIGTERM, &sig_term_handler);
	#if defined(TARGET_NPG) || defined(TARGET_OD) || defined TARGET_OD_BETA
	resetFrameBuffer();
	#endif
	createConfigFilesInHomeIfTheyDontExist();
	loadConfig();
	#if defined MIYOOMINI
	#else
	OCValue = OC_OC_LOW;
	sharpnessValue=8;
    #endif
	initializeDisplay(w,h);
	freeFonts();
	initializeFonts();
	initializeSettingsFonts();
	createThemesInHomeIfTheyDontExist();
	checkThemes();
	loadLastState();
    #if defined MIYOOMINI
    #else
	char temp[100];
	sprintf(temp,"SDL_VIDEO_KMSDRM_SCALING_SHARPNESS=%i",sharpnessValue);
	SDL_putenv(temp);
    #endif
//	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_NOFRAME|SDL_SWSURFACE);
	HW_Init();
	#if defined MIYOOMINI
    #else
	currentCPU = OC_NO;
	#endif
#ifndef TARGET_OD_BETA
	logMessage("INFO","initialSetup","Setting CPU to base");
	setCPU(currentCPU);
#endif
	setupKeys();
	checkIfDefault();
}

void initialSetup2() {
	char temp[300];
	strcpy(temp,themes[activeTheme]);
	strcat(temp,"/theme.ini");
	logMessage("INFO","initialSetup2","Loading theme");
	loadTheme(temp);
	logMessage("INFO","initialSetup2","Loading section groups");
	loadSectionGroups();
	logMessage("INFO","initialSetup2","Loading sections");
	int sectionCount=loadSections(sectionGroups[activeGroup].groupPath);
	logMessage("INFO","initialSetup2","Loading favorites");
	loadFavorites();
	currentMode=3;
	MENU_ITEMS_PER_PAGE=itemsPerPage;
	FULLSCREEN_ITEMS_PER_PAGE=itemsPerPageFullscreen;
	if(fullscreenMode==0) {
		ITEMS_PER_PAGE=MENU_ITEMS_PER_PAGE;
	} else {
		ITEMS_PER_PAGE=FULLSCREEN_ITEMS_PER_PAGE;
	}
	#if defined(TARGET_BITTBOY) || defined(TARGET_RFW) || defined(TARGET_OD) || defined(TARGET_OD_BETA) || defined(TARGET_NPG) || defined(MIYOOMINI)
	initSuspendTimer();
	#endif
	determineStartingScreen(sectionCount);
	enableKeyRepeat();
	#if defined MIYOOMINI
	mmModel = access("/customer/app/axp_test", F_OK);
	lastWifiMode=getCurrentWifi();
	#endif
	lastChargeLevel=getBatteryLevel();
	beforeTryingToSwitchGroup = activeGroup;
	#if defined MIYOOMINI
	audioFix = getCurrentSystemValue("audiofix");
	luminationValue = getCurrentSystemValue("lumination");
	volValue = getCurrentVolume();
	hueValue = getCurrentSystemValue("hue");
	saturationValue = getCurrentSystemValue("saturation");
	contrastValue = getCurrentSystemValue("contrast");
	gammaValue = loadConfiguration3();
	#endif
        brightnessValue = getCurrentBrightness();
        maxBrightnessValue = getMaxBrightness();
}

int isSettingsState(int state) {
        switch (state) {
                case SETTINGS_SCREEN:
                case APPEARANCE_SETTINGS:
                case SYSTEM_SETTINGS:
                case HELP_SCREEN_1:
                case HELP_SCREEN_2:
#if defined MIYOOMINI
                case SCREEN_SETTINGS:
#endif
                        return 1;
                default:
                        return 0;
        }
}

static int canToggleSearchWindow() {
        return !isSettingsState(currentState) && currentState != SHUTTING_DOWN && currentState != AFTER_RUNNING_LAUNCH_AT_BOOT && currentState != LOADING;
}

void processEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                if(event.type==getKeyDown()){
                        if (!isSuspended) {
                                SDLKey keyPressed = event.key.keysym.sym;
				if (canToggleSearchWindow() && keyPressed==(SDLKey)BTN_L1) {
					if (currentState==SEARCHING_ROMS) {
						closeSearchWindow();
					} else {
						openSearchWindow();
					}
					resetScreenOffTimer();
					continue;
				}
                                if (currentState==SEARCHING_ROMS) {
                                        handleSearchInput(keyPressed);
                                        resetScreenOffTimer();
                                        continue;
                                }
                                switch (currentState) {
                                        case BROWSING_GAME_LIST:
                                                previousState=BROWSING_GAME_LIST;
						performAction(CURRENT_SECTION.currentGameNode);
						refreshName=0;
						break;
					case SELECTING_SECTION:
						previousState=SELECTING_SECTION;
						performAction(CURRENT_SECTION.currentGameNode);
						refreshName=0;
						break;
					case SELECTING_EMULATOR:
						performChoosingAction();
						break;
					case CHOOSING_GROUP:
						previousState=CHOOSING_GROUP;
						performGroupChoosingAction();
						break;
					case SETTINGS_SCREEN:
						performSettingsChoosingAction();
						break;
					case APPEARANCE_SETTINGS:
						performAppearanceSettingsChoosingAction();
						break;
					case SYSTEM_SETTINGS:
						performSystemSettingsChoosingAction();
						break;
					case HELP_SCREEN_1:
						performHelpAction();
						break;
					case HELP_SCREEN_2:
						performHelpAction();
						break;
					case AFTER_RUNNING_LAUNCH_AT_BOOT:
						performLaunchAtBootQuitScreenChoosingAction();
						break;
#if defined MIYOOMINI
                    case SCREEN_SETTINGS:
                        performScreenSettingsChoosingAction();
                        break;
#endif
				}
			}
			resetScreenOffTimer();
			if (currentState!=AFTER_RUNNING_LAUNCH_AT_BOOT) {
				refreshRequest=1;
			}
		} else if (event.type==getKeyUp()&&!alternateControls) {
			if (currentState==BROWSING_GAME_LIST && previousState != SELECTING_EMULATOR ) {
				if(((int)event.key.keysym.sym)==BTN_B) {
					if (!aKeyComboWasPressed&&previousState!=SETTINGS_SCREEN) {
						currentState=SELECTING_SECTION;
					}
					hotKeyPressed=0;
					if(fullscreenMode) {
						if (CURRENT_SECTION.alphabeticalPaging) {
							resetPicModeHideMenuTimer();
						}
					}
					CURRENT_SECTION.alphabeticalPaging=0;
					if (aKeyComboWasPressed) {
						currentState=BROWSING_GAME_LIST;
					}
					aKeyComboWasPressed=0;
					if (currentState!=AFTER_RUNNING_LAUNCH_AT_BOOT) {
						refreshRequest=1;
					}
				}
			} else if (currentState==SELECTING_SECTION) {
				if(((int)event.key.keysym.sym)==BTN_B) {
					if (aKeyComboWasPressed==0) {
						if (sectionGroupCounter>1&&previousState!=SETTINGS_SCREEN) {
							beforeTryingToSwitchGroup = activeGroup;
							currentState=CHOOSING_GROUP;
						}
					} else {
						hotKeyPressed=0;
						aKeyComboWasPressed=0;
						CURRENT_SECTION.alphabeticalPaging=0;
						currentState=BROWSING_GAME_LIST;
					}
					if (currentState!=AFTER_RUNNING_LAUNCH_AT_BOOT) {
						refreshRequest=1;
					}
				}
			}
		} else if (alternateControls&&event.type==getKeyUp()) {
			if(((int)event.key.keysym.sym)==BTN_B) {
				if ((currentState==BROWSING_GAME_LIST || currentState==SELECTING_SECTION)&& previousState != SELECTING_EMULATOR) {
					if (!aKeyComboWasPressed&&sectionGroupCounter>1&&previousState!=SETTINGS_SCREEN) {
						beforeTryingToSwitchGroup = activeGroup;
						currentState=CHOOSING_GROUP;
					}
				}
				hotKeyPressed=0;
				if(fullscreenMode) {
					if(alternateControls&&currentState==SELECTING_SECTION) {
						hideFullScreenModeMenu();
					} else if (CURRENT_SECTION.alphabeticalPaging) {
						resetPicModeHideMenuTimer();
					}
				}
				CURRENT_SECTION.alphabeticalPaging=0;
				if (aKeyComboWasPressed) {
					currentState=BROWSING_GAME_LIST;
				}
				aKeyComboWasPressed=0;
				if (currentState!=AFTER_RUNNING_LAUNCH_AT_BOOT) {
					refreshRequest=1;
				}
			}
		}
		if (currentState==BROWSING_GAME_LIST_AFTER_TIMER) {
			loadGameList(0);
			currentState=BROWSING_GAME_LIST;
		}
	}
}
#ifdef TARGET_PC
	int main(int argc, char* argv[]) {
		if(argc<3) {
			printf("Usage: simplemenu-x86 [width] [height] \n");
			exit(0);
		}
#else
	int main() {
#endif
	logMessage("INFO","main","Setup 1");
#ifdef TARGET_PC
	initialSetup(atoi(argv[1]), atoi(argv[2]));
#elif defined MIYOOMINI
	initialSetup(640,480);
#else
	initialSetup(320,240);
#endif
	logMessage("INFO","main","Setup 2");
	initialSetup2();
	logMessage("INFO","main","Checking launch at boot");
	struct AutostartRom *launchAtBootGame = getLaunchAtBoot();
	if (launchAtBootGame!=NULL) {
		if (wasRunningFlag()) {
			currentState=AFTER_RUNNING_LAUNCH_AT_BOOT;
			resetShutdownTimer();
		} else {
			logMessage("INFO","main","Launching at boot");
			launchAutoStartGame(launchAtBootGame->rom, launchAtBootGame->emulatorDir, launchAtBootGame->emulator);
		}
	} else {
		currentState=BROWSING_GAME_LIST;
		pushEvent();
	}
	const int GAME_FPS=60;
	const int FRAME_DURATION_IN_MILLISECONDS = 1000/GAME_FPS;
	Uint32 start_time;
updateScreen(CURRENT_SECTION.currentGameNode);
refreshScreen();
startBatteryTimer();
startWifiTimer();
while(running) {
                start_time=SDL_GetTicks();
                processEvents();
		if(refreshRequest) {
			updateScreen(CURRENT_SECTION.currentGameNode);
			refreshRequest=0;
			refreshScreen();
		}
		if(refreshName) {	// titles too long need refresh to scroll
			refreshCounter++;
			if(refreshCounter>10) {
				refreshRequest=1;
				refreshCounter=0;
			}
		}
		//Time spent on one loop
		int timeSpent = SDL_GetTicks()-start_time;
		//If it took less than a frame
		if(timeSpent < FRAME_DURATION_IN_MILLISECONDS) {
			//Wait the remaining time until one frame completes
			SDL_Delay(FRAME_DURATION_IN_MILLISECONDS-timeSpent);
		}
	}
	int notDefaultButTryingToRebootOrShutDown = (shutDownEnabled==0&&(selectedShutDownOption==1||selectedShutDownOption==2));
	if(shutDownEnabled||notDefaultButTryingToRebootOrShutDown) {
		currentState=SHUTTING_DOWN;
		updateScreen(CURRENT_SECTION.currentGameNode);
		refreshScreen();
		sleep(1);
	}
	quit();
}
