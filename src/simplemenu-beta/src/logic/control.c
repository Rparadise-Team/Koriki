#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined TARGET_OD || defined TARGET_OD_BETA
#include <shake.h>
#endif

#include "../headers/config.h"
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/screen.h"
#include "../headers/logic.h"
#include "../headers/screen.h"
#include "../headers/graphics.h"
#include "../headers/string_utils.h"
#include "../headers/system_logic.h"
#include "../headers/doubly_linked_rom_list.h"
#include "../headers/utils.h"
#include "../headers/control.h"

int switchToGroup;

void scrollUp() {
	CURRENT_SECTION.currentGameNode=CURRENT_SECTION.currentGameNode->prev;
	//if at the first item of the first page
	if (CURRENT_SECTION.currentGameNode==NULL) {
		//go to the last page
		CURRENT_SECTION.currentPage=CURRENT_SECTION.totalPages;
		//go to the last game whether on the same page or not
		if (CURRENT_SECTION.totalPages==0) {
			CURRENT_SECTION.currentGameInPage=CURRENT_SECTION.gameCount-1;
		} else {
			if (CURRENT_SECTION.gameCount%(ITEMS_PER_PAGE)!=0) {
				CURRENT_SECTION.currentGameInPage=(CURRENT_SECTION.gameCount%ITEMS_PER_PAGE)-1;
			} else {
				CURRENT_SECTION.currentGameInPage=ITEMS_PER_PAGE-1;
			}
		}
		//point to the last node
		CURRENT_SECTION.currentGameNode=CURRENT_SECTION.tail;
	} else {
		//it's a full page and not the first item
		if (CURRENT_SECTION.currentGameInPage > 0) {
			//decrease selected game
			CURRENT_SECTION.currentGameInPage--;
		} else {
			//decrease page
			CURRENT_SECTION.currentPage--;
			//go to the last ekement of the page
			CURRENT_SECTION.currentGameInPage=ITEMS_PER_PAGE-1;
		}
	}
	//establish real game number
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void scrollDown() {
	CURRENT_SECTION.currentGameNode=CURRENT_SECTION.currentGameNode->next;
	// if at the end end of the list
	if (CURRENT_SECTION.currentGameNode==NULL) {
		//restart pages
		CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
		CURRENT_SECTION.currentPage=0;
		CURRENT_SECTION.currentGameInPage=0;
	} else {
		// it's a filled page and not the last item
		if (CURRENT_SECTION.currentGameInPage < ITEMS_PER_PAGE-1) {
			// increase selected game
			CURRENT_SECTION.currentGameInPage++;
		} else {
			CURRENT_SECTION.currentPage++;
			CURRENT_SECTION.currentGameInPage=0;
		}
	}
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void scrollToGame(int gameNumber) {
	if (CURRENT_SECTION.gameCount < gameNumber) {
		CURRENT_SECTION.currentGameInPage=0;
		CURRENT_SECTION.currentPage=0;
		CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
		return;
	}
	int pages = CURRENT_SECTION.gameCount / ITEMS_PER_PAGE;
	if (CURRENT_SECTION.gameCount%ITEMS_PER_PAGE==0) {
		pages--;
	}
	CURRENT_SECTION.totalPages=pages;
	CURRENT_SECTION.currentGameInPage=0;
	CURRENT_SECTION.currentPage=0;
	CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
	while (CURRENT_GAME_NUMBER<gameNumber) {
		scrollDown();
	}
}

int advanceSection() {
	int tempCurrentSection = currentSectionNumber;
//	if(currentSectionNumber==favoritesSectionNumber) {
//		return 0;
//	}
	int returnValue = 0;
	do {
		returnValue = 0;
		currentSectionNumber++;
		if (currentSectionNumber==menuSectionCounter) {
			currentSectionNumber=0;
		}
		if(!CURRENT_SECTION.counted&&!CURRENT_SECTION.initialized) {
			drawLoadingText();
			CURRENT_SECTION.gameCount=theSectionHasGames(&CURRENT_SECTION);
		}
		if (tempCurrentSection==currentSectionNumber) {
			returnValue = 0;
			break;
		} else if (CURRENT_SECTION.gameCount>0) {
			returnValue = 1;
			break;
		}
	} while(1);
	if (CURRENT_SECTION.systemLogoSurface == NULL) {
		CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
	}
	return returnValue;
}

int rewindSection() {
	int returnValue;
	int tempCurrentSection = currentSectionNumber;
//	if(currentSectionNumber==favoritesSectionNumber) {
//		return 0;
//	}
	logMessage("INFO","rewindSection","Rewinding section");
	do {
		currentSectionNumber--;
		if (currentSectionNumber==-1) {
			currentSectionNumber=menuSectionCounter-1;
		}
		if(!CURRENT_SECTION.counted&&!CURRENT_SECTION.initialized) {
			drawLoadingText();
			CURRENT_SECTION.gameCount=theSectionHasGames(&CURRENT_SECTION);
		}
		if (tempCurrentSection==currentSectionNumber) {
			returnValue = 0;
			break;
		} else if (CURRENT_SECTION.gameCount>0) {
			returnValue = 1;
			break;
		}
	} while(1);
	if (CURRENT_SECTION.systemLogoSurface == NULL) {
		CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
	}
	return returnValue;
}

void launchAutoStartGame(struct Rom *rom, char *emuDir, char *emuExec) {
	logMessage("INFO","launchAutoStartGame","BEGIN");
	FILE *file=NULL;
	char *error=malloc(3000);
	char tempExec[3000];
	if (emuDir[strlen(emuDir)-1]=='\n') {
		emuDir[strlen(emuDir)-1]='\0';
	}
	if (rom->name[strlen(rom->name)-1]=='\n') {
		rom->name[strlen(rom->name)-1]='\0';
	}
	char tempExecDirPlusFileName[3000];
	char tempExecFile[3000];
	logMessage("INFO","launchAutoStartGame","Loading rom preferences");
	loadRomPreferences(rom);
	logMessage("INFO","launchAutoStartGame","Setting running flag");
	setRunningFlag();
	strcpy(tempExec,emuDir);
	strcpy(tempExecFile,emuExec);
	char *ptr = strtok(tempExec, " ");
	strcpy(tempExecDirPlusFileName,emuDir);
	strcat(tempExecDirPlusFileName,tempExecFile);
	file = fopen(ptr, "r");
	strcat(tempExec,emuExec);
	if (!file&&strstr(tempExec,"#")==NULL) {
		strcpy(error,tempExecDirPlusFileName);
		strcat(error,"-NOT FOUND");
		generateError(error,0);
		return;
	}
	logMessage("INFO","launchAutoStartGame","Saving last state");
	saveLastState();
	#if defined MIYOOMINI
	if (CURRENT_SECTION.onlyFileNamesNoExtension) {
		logMessage("INFO","launchAutoStartGame","Executing");
		executeCommand(emuDir, emuExec, getGameName(rom->name), rom->isConsoleApp);
	} else {
		logMessage("INFO","launchAutoStartGame","Executing 2");
		executeCommand(emuDir, emuExec, rom->name, rom->isConsoleApp);
	}
	#else
	int freq = rom->preferences.frequency;
	//if it's not the base clock freq
	if (freq!=OC_NO) {
		//then it's overclocked
		//if it's not the configured level of OC
		if (freq!=OCValue) {
			//Change the freq
			//Save the rom prefs
			rom->preferences.frequency = OCValue;
			saveRomPreferences(rom);
		}
	}
	if (CURRENT_SECTION.onlyFileNamesNoExtension) {
		logMessage("INFO","launchAutoStartGame","Executing");
		executeCommand(emuDir, emuExec, getGameName(rom->name), rom->isConsoleApp, rom->preferences.frequency);
	} else {
		logMessage("INFO","launchAutoStartGame","Executing 2");
		executeCommand(emuDir, emuExec, rom->name, rom->isConsoleApp, rom->preferences.frequency);
	}
	#endif
}

void launchGame(struct Rom *rom) {
	FILE *file=NULL;
	char *error=malloc(3000);
	char tempExec[3000];

	char tempExecDirPlusFileName[3000];
	char tempExecFile[3000];
	if (isFavoritesSectionSelected() && favoritesSize > 0) {
		struct Favorite favorite = favorites[CURRENT_GAME_NUMBER];
		strcpy(tempExec,favorite.emulatorFolder);
		strcat(tempExec,favorite.executable);
		file = fopen(tempExec, "r");
		if (!file&&strstr(tempExec,"#")==NULL) {
			strcpy(error,favorite.executable);
			strcat(error,"-NOT FOUND");
			generateError(error,0);
			return;
		}
		#if defined MIYOOMINI
		executeCommand(favorite.emulatorFolder,favorite.executable,favorite.name, favorite.isConsoleApp);
    	#else
		int freq = favorite.frequency;
		//if it's not the base clock freq
		if (freq!=OC_NO) {
			//then it's overclocked
			//if it's not the configured level of OC
			if (freq!=OCValue) {
				//Change the freq to curent OC
				favorite.frequency = OCValue;
			}
		}
		executeCommand(favorite.emulatorFolder,favorite.executable,favorite.name, favorite.isConsoleApp, favorite.frequency);
		#endif
	} else if (rom->name!=NULL) {
		loadRomPreferences(rom);
		#if defined MIYOOMINI
		#else
		int freq = rom->preferences.frequency;
		//if it's not the base clock freq
		if (freq!=OC_NO) {
			//then it's overclocked
			//if it's not the configured level of OC
			if (freq!=OCValue) {
				//Change the freq
				//Save the rom prefs
				rom->preferences.frequency = OCValue;
				saveRomPreferences(rom);
			}
		}
		#endif
		if (isLaunchAtBoot(rom->name)) {
			setRunningFlag();
		}
		strcpy(tempExec,CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir]);
		strcpy(tempExecFile,CURRENT_SECTION.executables[rom->preferences.emulator]);
		char *ptr = strtok(tempExec, " ");
		strcpy(tempExecDirPlusFileName,CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir]);
		strcat(tempExecDirPlusFileName,tempExecFile);
		file = fopen(ptr, "r");
		strcat(tempExec,CURRENT_SECTION.executables[rom->preferences.emulator]);
		if (!file&&strstr(tempExec,"#")==NULL) {
			strcpy(error,tempExecDirPlusFileName);
			strcat(error,"-NOT FOUND");
			generateError(error,0);
			return;
		}
		#if defined MIYOOMINI
		if (CURRENT_SECTION.onlyFileNamesNoExtension) {
			executeCommand(CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir], CURRENT_SECTION.executables[rom->preferences.emulator],getGameName(rom->name), rom->isConsoleApp);
		} else {
			executeCommand(CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir], CURRENT_SECTION.executables[rom->preferences.emulator],rom->name, rom->isConsoleApp);
		}
		#else
		if (CURRENT_SECTION.onlyFileNamesNoExtension) {
			executeCommand(CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir], CURRENT_SECTION.executables[rom->preferences.emulator],getGameName(rom->name), rom->isConsoleApp, rom->preferences.frequency);
		} else {
			executeCommand(CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir], CURRENT_SECTION.executables[rom->preferences.emulator],rom->name, rom->isConsoleApp, rom->preferences.frequency);
		}
		#endif
	}
}

void launchEmulator(struct Rom *rom) {
	if (isFavoritesSectionSelected() && favoritesSize > 0) {
		struct Favorite favorite = favorites[CURRENT_GAME_NUMBER];
		#if defined MIYOOMINI
		executeCommand(favorite.emulatorFolder,favorite.executable,"*", favorite.isConsoleApp);
		#else
		int freq = favorite.frequency;
		//if it's not the base clock freq
		if (freq!=OC_NO) {
			//then it's overclocked
			//if it's not the configured level of OC
			if (freq!=OCValue) {
				//Change the freq to curent OC
				favorite.frequency = OCValue;
			}
		}
		executeCommand(favorite.emulatorFolder,favorite.executable,"*", favorite.isConsoleApp, favorite.frequency);
		#endif
	} else if (rom->name!=NULL) {
		loadRomPreferences(rom);
		#if defined MIYOOMINI
		executeCommand(CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir], CURRENT_SECTION.executables[CURRENT_SECTION.currentGameNode->data->preferences.emulator],"*", 0);
		#else
		int freq = rom->preferences.frequency;
		//if it's not the base clock freq
		if (freq!=OC_NO) {
			//then it's overclocked
			//if it's not the configured level of OC
			if (freq!=OCValue) {
				//Change the freq
				rom->preferences.frequency = OCValue;
				//Save the rom prefs
				saveRomPreferences(rom);
			}
		}
		executeCommand(CURRENT_SECTION.emulatorDirectories[CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir], CURRENT_SECTION.executables[CURRENT_SECTION.currentGameNode->data->preferences.emulator],"*", 0, rom->preferences.frequency);
		#endif
	}
}

void advancePage(struct Rom *rom) {
	logMessage("INFO","advancePage","Entering function");
	if (rom==NULL||rom->name==NULL) {
		logMessage("INFO","advancePage","Loading system logo");
		return;
	}
	if(CURRENT_SECTION.currentPage<=CURRENT_SECTION.totalPages) {
		logMessage("INFO","advancePage","Less or equal than total pages");
		if (CURRENT_SECTION.alphabeticalPaging) {
			logMessage("INFO","advancePage","Alpha paging");
			char *currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
			char currentLetter=tolower(currentGame[0]);
			while((tolower(currentGame[0])==currentLetter||(isdigit(currentLetter)&&isdigit(currentGame[0])))) {
				scrollDown();
				free(currentGame);
				currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
				if(CURRENT_SECTION.currentGameNode==CURRENT_SECTION.head) {
					break;
				}
			}
			free(currentGame);
		} else {
			logMessage("INFO","advancePage","Normal paging");
			if(CURRENT_SECTION.currentPage!=CURRENT_SECTION.totalPages) {
				logMessage("INFO","advancePage","Not at the last page");
				CURRENT_SECTION.currentPage++;
				for (int i=CURRENT_SECTION.currentGameInPage;i<ITEMS_PER_PAGE;i++) {
					CURRENT_SECTION.currentGameNode=CURRENT_SECTION.currentGameNode->next;
				}
			} else {
				logMessage("INFO","advancePage","Loop back to the first page");
				CURRENT_SECTION.currentPage=0;
				CURRENT_SECTION.currentGameNode=CURRENT_SECTION.head;
			}
			CURRENT_SECTION.currentGameInPage=0;
		}
	}
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void rewindPage(struct Rom *rom) {
	if (rom==NULL||rom->name==NULL) {
		return;
	}
	if (CURRENT_SECTION.alphabeticalPaging) {
		char *currentGame = getFileNameOrAlias(rom);
		char *previousGame;
		int hitStart = 0;
		while(!(CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0)) {
			previousGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->prev->data);
			if(tolower(currentGame[0])==tolower(previousGame[0])) {
				if (CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0) {
					hitStart = 1;
					break;
				} else {
					scrollUp();
				}
				free(currentGame);
				free(previousGame);
				currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
			} else {
				break;
			}

		}
		if (!hitStart) {
			scrollUp();
		}
		hitStart=0;
		free(currentGame);
		currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
		while(!(CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0)) {
			previousGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->prev->data);
			if ( (tolower(currentGame[0])==tolower(previousGame[0])) ||
					(isdigit(currentGame[0])&&isdigit(previousGame[0]))) {
				if (CURRENT_SECTION.currentPage==0&&CURRENT_SECTION.currentGameInPage==0) {
					hitStart = 1;
					break;
				} else {
					scrollUp();
				}
				free(currentGame);
				free(previousGame);
				currentGame = getFileNameOrAlias(CURRENT_SECTION.currentGameNode->data);
			} else {
				break;
			}
		}
		free(currentGame);
	} else if (CURRENT_SECTION.currentPage > 0) {
		int tempCurrentPage = CURRENT_SECTION.currentPage;
		while (!(CURRENT_SECTION.currentPage==(tempCurrentPage-1)&&CURRENT_SECTION.currentGameInPage==0)) {
			scrollUp();
		}
	} else {
		while (!(CURRENT_SECTION.currentPage==CURRENT_SECTION.totalPages&&CURRENT_SECTION.currentGameInPage==0)) {
			scrollUp();
		}
	}
	CURRENT_SECTION.realCurrentGameNumber=CURRENT_GAME_NUMBER;
}

void showOrHideFavorites() {
//	if (favoritesSectionSelected()) {
//		favoritesSectionSelected()=0;
//		currentSectionNumber=returnTo;
//		if (CURRENT_SECTION.systemLogoSurface == NULL) {
//			CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
//			logMessage("INFO","showOrHideFavorites","Loading system logo");
//		}
//		if (CURRENT_SECTION.backgroundSurface == NULL) {
//			logMessage("INFO","showOrHideFavorites","Loading system background");
//			CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
//			CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
//		}
//		if (returnTo==0) {
//			if(!alternateControls) {
//				currentState=SELECTING_SECTION;
//			} else {
//				currentState=BROWSING_GAME_LIST;
//			}
//			logMessage("INFO","showOrHideFavorites","Determining starting screen");
//			determineStartingScreen(menuSectionCounter);
//		} else {
//				if(!alternateControls) {
//					currentState=SELECTING_SECTION;
//				} else {
//					currentState=BROWSING_GAME_LIST;
//				}
//				logMessage("INFO","showOrHideFavorites","No return, loading game list");
//				loadGameList(0);
//		}
//		logMessage("INFO","showOrHideFavorites","hideFullScreenModeMenu()");
//		hideFullScreenModeMenu();
//		scrollToGame(CURRENT_SECTION.realCurrentGameNumber);
//		logMessage("INFO","showOrHideFavorites","scrolled");
//		return;
//	}
//	if(strlen(favorites[0].name)<1) {
//		return;
//	}
//	favoritesSectionSelected()=1;
	returnTo=currentSectionNumber;
	currentSectionNumber=favoritesSectionNumber;
	if (CURRENT_SECTION.systemLogoSurface == NULL) {
		CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
		logMessage("WARN","showOrHideFavorites","Loading system logo");
	}
	if (CURRENT_SECTION.backgroundSurface == NULL) {
		logMessage("WARN","showOrHideFavorites","Loading system background");
		CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
		CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
	}
	if(!alternateControls) {
		currentState=SELECTING_SECTION;
	} else {
		currentState=BROWSING_GAME_LIST;
	}
	logMessage("WARN","showOrHideFavorites","Displaying system logo 3");
//	loadFavoritesSectionGameList();
}

void removeFavorite() {
	favoritesChanged=1;
	if (favoritesSize>0) {
		#if defined TARGET_OD || defined TARGET_OD_BETA
		Shake_Play(device, effect_id1);
		#endif	
		for (int i=CURRENT_GAME_NUMBER;i<favoritesSize;i++) {
			strcpy(favorites[i].emulatorFolder,favorites[i+1].emulatorFolder);
			strcpy(favorites[i].section,favorites[i+1].section);
			strcpy(favorites[i].executable,favorites[i+1].executable);
			strcpy(favorites[i].filesDirectory,favorites[i+1].filesDirectory);
			strcpy(favorites[i].name,favorites[i+1].name);
			strcpy(favorites[i].alias,favorites[i+1].alias);
			strcpy(favorites[i].sectionAlias,favorites[i+1].sectionAlias);
			favorites[i].isConsoleApp = favorites[i+1].isConsoleApp;
			#if defined MIYOOMINI
			#else
			favorites[i].frequency = favorites[i+1].frequency;
			#endif
		}
		strcpy(favorites[favoritesSize-1].section,"\0");
		strcpy(favorites[favoritesSize-1].emulatorFolder,"\0");
		strcpy(favorites[favoritesSize-1].executable,"\0");
		strcpy(favorites[favoritesSize-1].filesDirectory,"\0");
		strcpy(favorites[favoritesSize-1].name,"\0");
		strcpy(favorites[favoritesSize-1].alias,"\0");
		strcpy(favorites[favoritesSize-1].sectionAlias,"\0");
		#if defined MIYOOMINI
		#else
		favorites[favoritesSize-1].frequency = OC_NO;
		#endif
		favorites[favoritesSize-1].isConsoleApp = 0;
		favoritesSize--;
		if (CURRENT_GAME_NUMBER==favoritesSize) {
			FAVORITES_SECTION.realCurrentGameNumber--;
		}
		loadFavoritesSectionGameList();
		if(!isPicModeMenuHidden) {
			resetPicModeHideMenuTimer();
		}
	}
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void markAsFavorite(struct Rom *rom) {
	favoritesChanged=1;
	if (favoritesSize<FAVORITES_SIZE) {
		if (!doesFavoriteExist(rom->name)) {
			resetHideHeartTimer();
			#if defined TARGET_OD || defined TARGET_OD_BETA
			Shake_Play(device, effect_id);
			msleep(200);
			Shake_Play(device, effect_id);
			#endif		
			if (CURRENT_SECTION.onlyFileNamesNoExtension) {
				strcpy(favorites[favoritesSize].name, getGameName(rom->name));
			} else {
				strcpy(favorites[favoritesSize].name, rom->name);
			}
			if(rom->alias!=NULL&&strlen(rom->alias)>2) {
				strcpy(favorites[favoritesSize].alias, rom->alias);
			} else {
				favorites[favoritesSize].alias[0]=' ';
			}
			if (strlen(CURRENT_SECTION.fantasyName)>1) {
				strcpy(favorites[favoritesSize].sectionAlias,CURRENT_SECTION.fantasyName);
			} else {
				favorites[favoritesSize].sectionAlias[0]=' ';
			}
			strcpy(favorites[favoritesSize].section,CURRENT_SECTION.sectionName);
			loadRomPreferences(rom);
			strcpy(favorites[favoritesSize].emulatorFolder,CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir]);
			strcpy(favorites[favoritesSize].executable,CURRENT_SECTION.executables[rom->preferences.emulator]);
			#if defined MIYOOMINI
			#else
			favorites[favoritesSize].frequency = rom->preferences.frequency;
			#endif
			strcpy(favorites[favoritesSize].filesDirectory,rom->directory);
			favorites[favoritesSize].isConsoleApp = rom->isConsoleApp;
			favoritesSize++;
			FAVORITES_SECTION.gameCount++;
			qsort(favorites, favoritesSize, sizeof(struct Favorite), compareFavorites);
		}
	}
}

int isSelectPressed() {
	return keys[BTN_SELECT];
}

void performGroupChoosingAction() {
	int existed = 0;
	if (keys[BTN_START]) {
		chosenSetting=SHUTDOWN_OPTION;
		themeChanged=activeTheme;
		previousState=CHOOSING_GROUP;
		currentState=SETTINGS_SCREEN;
		return;
	}
	if ((!alternateControls&&keys[BTN_UP])||(alternateControls&&keys[BTN_L1])) {
		if(activeGroup>0) {
			activeGroup--;
		} else {
			activeGroup=sectionGroupCounter-1;
		}
		return;
	}
	if ((!alternateControls&&keys[BTN_DOWN])||(alternateControls&&keys[BTN_R1])) {
		if(activeGroup<sectionGroupCounter-1) {
			activeGroup++;
		} else {
			activeGroup=0;
		}
		return;
	}
	if (keys[BTN_A]) {
//		int preFavs = favoritesSectionNumber;
		if (beforeTryingToSwitchGroup!=activeGroup) {
			for (int sectionCount=0;sectionCount<menuSectionCounter;sectionCount++) {
				if((!isFavoritesSectionSelected()&&sectionCount==currentSectionNumber)||
					(isFavoritesSectionSelected()&&sectionCount==returnTo)) {
					sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][0]=1;
				} else {
					sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][0]=0;
				}
				sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][1]=menuSections[sectionCount].currentPage;
				sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][2]=menuSections[sectionCount].currentGameInPage;
				sectionGroupStates[beforeTryingToSwitchGroup][sectionCount][3]=menuSections[sectionCount].realCurrentGameNumber;

				cleanListForSection(&menuSections[sectionCount]);
				if (menuSections[sectionCount].backgroundSurface!=NULL) {
					SDL_FreeSurface(menuSections[sectionCount].backgroundSurface);
					menuSections[sectionCount].backgroundSurface = NULL;
				}
				if (menuSections[sectionCount].systemLogoSurface!=NULL) {
					SDL_FreeSurface(menuSections[sectionCount].systemLogoSurface);
					menuSections[sectionCount].systemLogoSurface = NULL;
				}
				if (menuSections[sectionCount].systemPictureSurface!=NULL) {
					SDL_FreeSurface(menuSections[sectionCount].systemPictureSurface);
					menuSections[sectionCount].systemPictureSurface = NULL;
				}

				int execsNum = sizeof(menuSections[sectionCount].executables) / sizeof(menuSections[sectionCount].executables[0]);
				for (int j=0;j<execsNum-1;j++) {
					if (sectionCount!=favoritesSectionNumber) {
						if (menuSections[sectionCount].executables[j]!=NULL&&strlen(menuSections[sectionCount].executables[j])>0) {
							free(menuSections[sectionCount].executables[j]);
						}
						if (menuSections[sectionCount].emulatorDirectories[j]!=NULL&&strlen(menuSections[sectionCount].emulatorDirectories[j])>0) {
							free(menuSections[sectionCount].emulatorDirectories[j]);
						}
					}
				}
			}
			if (currentState!=BROWSING_GAME_LIST) {
				loadSections(sectionGroups[activeGroup].groupPath);
				if(!alternateControls) {
					currentState=SELECTING_SECTION;
				} else {
					currentState=BROWSING_GAME_LIST;
				}
				currentSectionNumber=0;
				for(int i=0;i<menuSectionCounter;i++) {
					menuSections[i].initialized=0;
					menuSections[i].counted=0;
					menuSections[i].currentPage=0;
					menuSections[i].realCurrentGameNumber=0;
					menuSections[i].currentGameInPage=0;
					menuSections[i].currentGameNode=CURRENT_SECTION.head;
				}
				for (int sectionCount=0;sectionCount<menuSectionCounter;sectionCount++) {
//					if(sectionCount!=favoritesSectionNumber) {
						if (sectionGroupStates[activeGroup][sectionCount][0]==1) {
							currentSectionNumber=sectionCount;
							logMessage("INFO","performGroupChoosingAction","Loading system logo");
							CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
							drawLoadingText();
							logMessage("INFO","performGroupChoosingAction","Loading system background");
							CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
							CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
							existed = 1;
						}
						menuSections[sectionCount].currentPage=sectionGroupStates[activeGroup][sectionCount][1];
						menuSections[sectionCount].currentGameInPage=sectionGroupStates[activeGroup][sectionCount][2];
						menuSections[sectionCount].realCurrentGameNumber=sectionGroupStates[activeGroup][sectionCount][3];
//					} else {
//						menuSections[sectionCount].currentPage=sectionGroupStates[beforeTryingToSwitchGroup][preFavs][1];
//						menuSections[sectionCount].currentGameInPage=sectionGroupStates[beforeTryingToSwitchGroup][preFavs][2];
//						menuSections[sectionCount].realCurrentGameNumber=sectionGroupStates[beforeTryingToSwitchGroup][preFavs][3];
//					}
				}
				loadGameList(0);
				loadFavoritesSectionGameList();
			}
			if (!existed) {
				drawLoadingText();
				logMessage("INFO","performGroupChoosingAction","performGroupChoosingAction !existed - Loading system logo");
				CURRENT_SECTION.systemLogoSurface = IMG_Load(CURRENT_SECTION.systemLogo);
				logMessage("INFO","performGroupChoosingAction","Loading system background");
				CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
				CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
			}
			if (CURRENT_SECTION.gameCount==0) {
				advanceSection();
				loadGameList(0);
				loadFavoritesSectionGameList();
			}
			beforeTryingToSwitchGroup=activeGroup;
		} else {
			activeGroup = beforeTryingToSwitchGroup;
			if(!alternateControls) {
				currentState=SELECTING_SECTION;
			} else {
				currentState=BROWSING_GAME_LIST;
			}
		}
	}
}

void performLaunchAtBootQuitScreenChoosingAction() {
	if (keys[BTN_X]) {
		setLaunchAtBoot(NULL);
		clearShutdownTimer();
		currentState=BROWSING_GAME_LIST;
		running=1;
	}
}

void performHelpAction() {
	if (keys[BTN_B]) {
		chosenSetting=previouslyChosenSetting;
		currentState=SETTINGS_SCREEN;
	} else 	if (keys[BTN_DOWN]) {
		currentState=HELP_SCREEN_2;
	} else 	if (keys[BTN_UP]) {
		currentState=HELP_SCREEN_1;
	}
}

void performAppearanceSettingsChoosingAction() {
	TIDY_ROMS_OPTION=0;
	FULL_SCREEN_FOOTER_OPTION=1;
	FULL_SCREEN_MENU_OPTION=2;
	if (keys[BTN_UP]) {
		if(chosenSetting>0) {
			chosenSetting--;
		} else {
			chosenSetting=2;
		}
	} else if (keys[BTN_DOWN]) {
		if(chosenSetting<2) {
			chosenSetting++;
		} else {
			chosenSetting=0;
		}
	} else if (keys[BTN_LEFT]||keys[BTN_RIGHT]) {
		if (chosenSetting==TIDY_ROMS_OPTION) {
			stripGames=1+stripGames*-1;
		} else if (chosenSetting==FULL_SCREEN_FOOTER_OPTION) {
			footerVisibleInFullscreenMode=1+footerVisibleInFullscreenMode*-1;
		} else if (chosenSetting==FULL_SCREEN_MENU_OPTION) {
			menuVisibleInFullscreenMode=1+menuVisibleInFullscreenMode*-1;
		}
	} else if (keys[BTN_LEFT]||keys[BTN_B]) {
		chosenSetting=previouslyChosenSetting;
		currentState=SETTINGS_SCREEN;
	}
}

#if defined MIYOOMINI
void performScreenSettingsChoosingAction() {
    LUMINATION_OPTION = 0;
    HUE_OPTION = 1;
    SATURATION_OPTION = 2;
    CONTRAST_OPTION = 3;
	GAMMA_OPTION = 4;
    NUM_SCREEN_OPTIONS = 5;
    COLOR_MAX_VALUE = 20;
	GAMMA_MAX_VALUE = 5;

	if (keys[BTN_UP]) {
		if(chosenSetting>0) {
			chosenSetting--;
		} else {
			chosenSetting=NUM_SCREEN_OPTIONS-1;
		}
	} else if (keys[BTN_DOWN]) {
		if(chosenSetting<NUM_SCREEN_OPTIONS-1) {
			chosenSetting++;
		} else {
			chosenSetting=0;
		}
	} else if (keys[BTN_LEFT]||keys[BTN_RIGHT]) {
		if (chosenSetting==LUMINATION_OPTION) {
			if (keys[BTN_LEFT]) {
				if (luminationValue>0) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("lumination");
					luminationValue-=1;
					value = (get+35)-1;
					Luma(0, value);
				}
			} else {
				if (luminationValue<COLOR_MAX_VALUE) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("lumination");
					luminationValue+=1;
					value = (get+35)+1;
					Luma(0, value);
				}
			}
			setSystemValue("lumination", luminationValue);
		} else if (chosenSetting==HUE_OPTION) {
			if (keys[BTN_LEFT]) {
				if (hueValue>0) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("hue");
					hueValue-=1;
					value = (get*5)-5;
					Hue(0, value);
				}
			} else {
				if (hueValue<COLOR_MAX_VALUE) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("hue");
					hueValue+=1;
					value = (get*5)+5;
					Hue(0, value);
				}
			}
			setSystemValue("hue", hueValue);
		} else if (chosenSetting==SATURATION_OPTION) {
			if (keys[BTN_LEFT]) {
				if (saturationValue>0) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("saturation");
					saturationValue-=1;
					value = (get*5)-5;
					Saturation(0, value);
				}
			} else {
				if (saturationValue<COLOR_MAX_VALUE) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("saturation");
					saturationValue+=1;
					value = (get*5)+5;
					Saturation(0, value);
				}
			}
			setSystemValue("saturation", saturationValue);
		} else if (chosenSetting==CONTRAST_OPTION) {
			if (keys[BTN_LEFT]) {
				if (contrastValue>0) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("contrast");
					contrastValue-=1;
					value = ((get*2)+30)-2;
					Contrast(0, value);
				}
			} else {
				if (contrastValue<COLOR_MAX_VALUE) {
					int value = 0;
					int get = 0;
					get = getCurrentSystemValue("contrast");
					contrastValue+=1;
					value = ((get*2)+30)+2;
					Contrast(0, value); 
				}
			}
			setSystemValue("contrast", contrastValue);
		} else if (chosenSetting==GAMMA_OPTION) {
			loadConfiguration3();
			if (keys[BTN_LEFT]) {
				if (gammaValue>-5) {
					gammaValue-=1;
				}
			} else {
				if (gammaValue<GAMMA_MAX_VALUE) {
					gammaValue+=1;
				}
			}
			saveConfiguration3(gammaValue);
		}
	} else if (keys[BTN_B]) {
		chosenSetting=SCREEN_OPTION;
		currentState=SYSTEM_SETTINGS;
	}
}
#endif

#if defined MIYOOMINI
void performSystemSettingsChoosingAction() {
	VOLUME_OPTION=0;
	BRIGHTNESS_OPTION=1;
	OC_OPTION=2;
    AUDIOFIX_OPTION=3;
	MUSIC_OPTION=4;
    SCREEN_OPTION=5;
	LOADING_OPTION=6;
	SCREEN_TIMEOUT_OPTION=7;
    NUM_SYSTEM_OPTIONS=8;
	if (keys[BTN_UP]) {
		if(chosenSetting>0) {
			chosenSetting--;
		} else {
			chosenSetting=NUM_SYSTEM_OPTIONS-1;
		}
	} else if (keys[BTN_DOWN]) {
		if(chosenSetting<NUM_SYSTEM_OPTIONS-1) {
			chosenSetting++;
		} else {
			chosenSetting=0;
		}
	} else if (keys[BTN_LEFT]||keys[BTN_RIGHT]) {
		if (chosenSetting==BRIGHTNESS_OPTION) {
			if (keys[BTN_LEFT]) {
				if (brightnessValue>1) {
					brightnessValue-=1;
				}
			} else {
				if (brightnessValue<maxBrightnessValue) {
					brightnessValue+=1;
				}
			}
			setSystemValue("brightness", brightnessValue);
			setBrightness(brightnessValue);
		} else if (chosenSetting==SCREEN_TIMEOUT_OPTION) {
				if (keys[BTN_LEFT]) {
					if (timeoutValue>0) {
						timeoutValue-=5;
					}
				} else {
					if (timeoutValue<60) {
						timeoutValue+=5;
					}
				}
		} else if (chosenSetting==OC_OPTION) {
			FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
			fscanf(fp, "%d", &CPUMIYOO);
			fclose(fp);
			if (keys[BTN_LEFT]) {
				if (CPUMIYOO>400000) {
					CPUMIYOO-=200000;
					char cpuclock0[100];
					snprintf(cpuclock0, sizeof(cpuclock0), "echo %d > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", CPUMIYOO);
					system(cpuclock0);
					system("cp /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor /mnt/SDCARD/.simplemenu/governor.sav");
					system("cp /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq /mnt/SDCARD/.simplemenu/cpu.sav");
					system("sync");
				}
			} else {
				if (CPUMIYOO<1200000) {
					CPUMIYOO+=200000;
					char cpuclock0[100];
					snprintf(cpuclock0, sizeof(cpuclock0), "echo %d > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", CPUMIYOO);
					system(cpuclock0);
					system("cp /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor /mnt/SDCARD/.simplemenu/governor.sav");
					system("cp /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq /mnt/SDCARD/.simplemenu/cpu.sav");
					system("sync");
				}
			}
		} else if (chosenSetting==AUDIOFIX_OPTION) {
			getCurrentVolume();
            audioFix = 1 - audioFix;
            setSystemValue("audiofix", audioFix);
		 	Fix = getCurrentSystemValue("audiofix");
			volume = getCurrentSystemValue("vol");
		 	if (Fix == 1) {
	        	brightness = getCurrentBrightness();
				if (musicEnabled) {
					stopmusic();
				}
				if (mmModel) {
					char command [100];
					char command2 [100];
					snprintf(command, sizeof(command), "/mnt/SDCARD/Koriki/bin/audioserver &");
					snprintf(command2, sizeof(command2), "touch /tmp/audioserver_on && sync");
					setenv("LD_PRELOAD", "/mnt/SDCARD/Koriki/lib/libpadsp.so", 1);
					system(command);
					system(command2);
				} else {
					char command [100];
					char command2 [100];
					snprintf(command, sizeof(command), "/mnt/SDCARD/Koriki/bin/audioserver &");
					snprintf(command2, sizeof(command2), "touch /tmp/audioserver_on && sync");
					setenv("LD_PRELOAD", "/mnt/SDCARD/Koriki/lib/libpadsp.so", 1);
					system(command);
					system(command2);
				}
				if (musicEnabled) {
					startmusic();
				}
				setBrightness(brightness);
			} else if (Fix == 0) {
	        	brightness = getCurrentBrightness();
				if (musicEnabled) {
					stopmusic();
				}
				if (mmModel) {
					char command [128];
					snprintf(command, sizeof(command), "killall audioserver && killall audioserver.min && rm /tmp/audioserver_on && sync");
					system(command);
					unsetenv("LD_PRELOAD");
				} else {
					char command [128];
					snprintf(command, sizeof(command), "killall audioserver && killall audioserver.plu && rm /tmp/audioserver_on && sync");
					system(command);
					unsetenv("LD_PRELOAD");
				}
				if (musicEnabled) {
					startmusic();
				}
				setBrightness(brightness);
			}
			getCurrentVolume();
			setVolume(volume, 0);
		} else if (chosenSetting==LOADING_OPTION) {
			loadConfiguration1();
			if (loadingScreenEnabled == 1) {
				loadingScreenEnabled = 0;
			} else if (loadingScreenEnabled == 0) {
				loadingScreenEnabled = 1;
				}
			saveConfiguration1();
		} else if (chosenSetting==MUSIC_OPTION) {
			loadConfiguration2();
			if (musicEnabled == 1) {
				musicEnabled = 0;
				stopmusic();
			} else if (musicEnabled == 0) {
				musicEnabled = 1;
				if (Fix == 0) {
					brightness = getCurrentBrightness();
					volume = getCurrentSystemValue("vol");
				}
				startmusic();
				if (Fix == 0) {
					setBrightness(brightness);
					setVolume(volume, 0);
				}
			}
			saveConfiguration2();
		} else if (chosenSetting==VOLUME_OPTION) {
			if (keys[BTN_LEFT]) {
				if (volValue>0) {
					volume = getCurrentSystemValue("vol");
					volValue-=1;
					setVolume(volume, -1);
					setSystemValue("vol", volValue);
					if (volValue == 0) {
						setSystemValue("mute", 1);
						setMute(1);
					} else if (volValue > 0) {
						setSystemValue("mute", 0);
						setMute(0);
					}
				}
			} else {
				if (volValue<23) {
					volume = getCurrentSystemValue("vol");
					volValue+=1;
					setVolume(volume, 1);
					setSystemValue("vol", volValue);
					if (volValue == 0) {
						setSystemValue("mute", 1);
						setMute(1);
					} else if (volValue > 0) {
						setSystemValue("mute", 0);
						setMute(0);
					}
				}
			}
		}
	} else if (chosenSetting==SCREEN_OPTION&&keys[BTN_A]) {
		chosenSetting=0;
		currentState=SCREEN_SETTINGS;
	} else if (keys[BTN_B]) {
		chosenSetting=previouslyChosenSetting;
		currentState=SETTINGS_SCREEN;
	}
}
#else
void performSystemSettingsChoosingAction() {
	VOLUME_OPTION=0;
	BRIGHTNESS_OPTION=1;
	SHARPNESS_OPTION=2;
	SCREEN_TIMEOUT_OPTION=3;
	OC_OPTION=4;
	USB_OPTION=5;
    NUM_SYSTEM_OPTIONS = 6;
	if (keys[BTN_UP]) {
		if(chosenSetting>0) {
			chosenSetting--;
		} else {
			chosenSetting=NUM_SYSTEM_OPTIONS-1;
		}
	} else if (keys[BTN_DOWN]) {
		if(chosenSetting<NUM_SYSTEM_OPTIONS-1) {
			chosenSetting++;
		} else {
			chosenSetting=0;
		}
	} else if (keys[BTN_LEFT]||keys[BTN_RIGHT]) {
		if (chosenSetting==USB_OPTION) {
#if defined TARGET_OD || defined TARGET_PC
			hdmiChanged=1+hdmiChanged*-1;
#endif
		} else if (chosenSetting==SCREEN_TIMEOUT_OPTION) {
			if(!hdmiEnabled) {
				if (keys[BTN_LEFT]) {
					if (timeoutValue>0) {
						timeoutValue-=5;
					}
				} else {
					if (timeoutValue<60) {
						timeoutValue+=5;
					}
				}
			}
		} else if (chosenSetting==BRIGHTNESS_OPTION) {
			if (keys[BTN_LEFT]) {
				if (brightnessValue>1) {
					brightnessValue-=1;
				}
			} else {
				if (brightnessValue<maxBrightnessValue) {
					brightnessValue+=1;
				}
			}
			setBrightness(brightnessValue);
		} else if (chosenSetting==SHARPNESS_OPTION) {
			if (keys[BTN_LEFT]) {
				if (sharpnessValue>0) {
					sharpnessValue-=1;
				}
			} else {
				if (sharpnessValue<32) {
					sharpnessValue+=1;
				}
			}
			char temp[100];
			sprintf(temp,"SDL_VIDEO_KMSDRM_SCALING_SHARPNESS=%i",sharpnessValue);
			SDL_putenv(temp);
//			screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_NOFRAME|SDL_SWSURFACE);
		} else if (chosenSetting==OC_OPTION) {
#if defined TARGET_OD_BETA || defined TARGET_PC
			if (OCValue==OC_OC_LOW) {
				OCValue=OC_OC_HIGH;
			} else {
				OCValue=OC_OC_LOW;
			}
		}
#else
			OCValue=OC_NO;
		}
#endif
	} else if (chosenSetting==VOLUME_OPTION&&keys[BTN_A]) {
		if (keys[BTN_A]) {
			executeCommand ("/usr/bin", "alsamixer", "#", 1, OC_NO);
		}
	} else if (chosenSetting==USB_OPTION&&keys[BTN_A]) {
#if defined TARGET_RFW
		executeCommand ("./scripts/", "usb_mode_on.sh", "#", 0, OC_NO);
		hotKeyPressed=0;
#elif defined TARGET_OD_BETA
		selectedShutDownOption=1;
		running=0;
#endif
	} else if (keys[BTN_B]) {
		chosenSetting=previouslyChosenSetting;
		currentState=SETTINGS_SCREEN;
	}
}
#endif
	
void performSettingsChoosingAction() {
	#if defined MIYOOMINI
	if (mmModel) {
	SHUTDOWN_OPTION=0;
	THEME_OPTION=1;
	APPEARANCE_OPTION=2;
	SYSTEM_OPTION=3;
	RETROARCH_OPTION=4;
	HELP_OPTION=5;
	} else {
	SHUTDOWN_OPTION=0;
	THEME_OPTION=1;
	APPEARANCE_OPTION=2;
	SYSTEM_OPTION=3;
	WIFI_OPTION=4;
	WIFIAPP_OPTION=5;
	SCRAPER_OPTION=6;
	RETROARCH_OPTION=7;
	HELP_OPTION=8;
	}
	#else
	SHUTDOWN_OPTION=0;
	THEME_OPTION=1;
	DEFAULT_OPTION=2;
	APPEARANCE_OPTION=3;
	SYSTEM_OPTION=4;
	HELP_OPTION=5;
	#endif

	if (keys[BTN_UP]) {
		if(chosenSetting>0) {
			chosenSetting--;
		} else {
			#if defined MIYOOMINI
			if (mmModel) {
			chosenSetting=5;
			} else {
			chosenSetting=8;
			}
			#else
			chosenSetting=5;
			#endif
		}
	} else if (keys[BTN_DOWN]) {
		#if defined MIYOOMINI
		if (mmModel) {
			if(chosenSetting<5) {
	        	chosenSetting++;
			} else {
				chosenSetting=0;
			}
		} else {
			if(chosenSetting<8) {
	        	chosenSetting++;
			} else {
				chosenSetting=0;
			}
		}
		#else
		if(chosenSetting<5) {
			chosenSetting++;
		} else {
			chosenSetting=0;
		}
		#endif
	} else if (keys[BTN_LEFT]||keys[BTN_RIGHT]) {
		if (chosenSetting==SHUTDOWN_OPTION) {
			if (shutDownEnabled) {
				selectedShutDownOption=1+selectedShutDownOption*-1;
			} else {
				switch (selectedShutDownOption) {
				case 0:
					if(keys[BTN_RIGHT]) {
						selectedShutDownOption = 1;
					} else {
						selectedShutDownOption = 2;
					}
					break;
				case 1:
					if(keys[BTN_RIGHT]) {
						selectedShutDownOption = 2;
					} else {
						selectedShutDownOption = 0;
					}
					break;
				default:
					if(keys[BTN_RIGHT]) {
						selectedShutDownOption = 0;
					} else {
						selectedShutDownOption = 1;
					}
				}
			}
		} else if (chosenSetting==THEME_OPTION) {
			if (keys[BTN_LEFT]) {
				if (activeTheme>0) {
					activeTheme--;
				} else {
					activeTheme=themeCounter-1;
				}
			} else {
				if (activeTheme<themeCounter-1) {
					activeTheme++;
				} else {
					activeTheme=0;
				}
			}
		} else if (chosenSetting==WIFI_OPTION) {
			#if defined MIYOOMINI
			if (!mmModel) {
			loadConfiguration4();
			if (wifiEnabled == 1) {
				wifiEnabled = 0;
			} else if (wifiEnabled == 0) {
				wifiEnabled = 1;
			}
			saveConfiguration4();
			}
			#else
			#endif
		} else if (chosenSetting==DEFAULT_OPTION) {
			#ifndef MIYOOMINI
			char command [300];
			if (shutDownEnabled) {
				#ifdef TARGET_BITTBOY
				snprintf(command,sizeof(command),"rm /mnt/autoexec.sh;mv /mnt/autoexec.sh.bck /mnt/autoexec.sh");
				#endif
				#ifdef TARGET_RFW
				snprintf(command,sizeof(command),"rm /home/retrofw/autoexec.sh;mv /home/retrofw/autoexec.sh.bck /home/retrofw/autoexec.sh");
				#endif
				#if defined TARGET_OD || defined TARGET_NPG
				snprintf(command,sizeof(command),"rm /usr/local/sbin/frontend_start;mv /usr/local/sbin/frontend_start.bck /usr/local/sbin/frontend_start");
				#endif
				#if defined TARGET_OD_BETA
				snprintf(command,sizeof(command),"rm /media/data/local/home/.autostart");
				#endif
			} else {
				#ifdef TARGET_BITTBOY
				snprintf(command,sizeof(command),"mv /mnt/autoexec.sh /mnt/autoexec.sh.bck;cp scripts/autoexec.sh /mnt");
				#endif
				#ifdef TARGET_RFW
				snprintf(command,sizeof(command),"mv /home/retrofw/autoexec.sh /home/retrofw/autoexec.sh.bck;cp scripts/autoexec.sh /home/retrofw");
				#endif
				#if defined TARGET_OD || defined TARGET_NPG
				snprintf(command,sizeof(command),"mv /usr/local/sbin/frontend_start /usr/local/sbin/frontend_start.bck;cp scripts/frontend_start /usr/local/sbin/");
				#endif
				#if defined TARGET_OD_BETA
				snprintf(command,sizeof(command),"cp ./scripts/frontend_start /media/data/local/home/.autostart");
				#endif
			}
			int ret = system(command);
			if (ret==-1) {
				logMessage("ERROR","performSettingsChoosingAction","Error setting default");
			}
			if (selectedShutDownOption==2) {
				selectedShutDownOption = 0;
			}
			shutDownEnabled=1+shutDownEnabled*-1;
			#endif
		}
	} else if (chosenSetting==SHUTDOWN_OPTION&&keys[BTN_A]) {
		running=0;
	} else if (chosenSetting==HELP_OPTION&&keys[BTN_A]) {
		previouslyChosenSetting=chosenSetting;
		chosenSetting=0;
		currentState=HELP_SCREEN_1;
	} else if (chosenSetting==APPEARANCE_OPTION&&keys[BTN_A]) {
		previouslyChosenSetting=chosenSetting;
		chosenSetting=0;
		currentState=APPEARANCE_SETTINGS;
	} else if (chosenSetting==SYSTEM_OPTION&&keys[BTN_A]) {
		previouslyChosenSetting=chosenSetting;
		chosenSetting=0;
		currentState=SYSTEM_SETTINGS;
		brightnessValue = getCurrentBrightness();
		volValue = getCurrentSystemValue("vol");
	} else if (chosenSetting==RETROARCH_OPTION&&keys[BTN_A]) {
		#if defined MIYOOMINI
		chosenSetting=0;
		executeCommand("/mnt/SDCARD/.simplemenu/hide", "RetroArch.sh", "#", 0);
		#else
		#endif
	} else if (chosenSetting==WIFIAPP_OPTION&&keys[BTN_A]) {
		#if defined MIYOOMINI
		if (!mmModel) {
		chosenSetting=0;
		executeCommand("/mnt/SDCARD/.simplemenu/hide", "Wifi.sh", "#", 0);
		}
		#else
		#endif
	} else if (chosenSetting==SCRAPER_OPTION&&keys[BTN_A]) {
		#if defined MIYOOMINI
		if (!mmModel) {
		chosenSetting=0;
		executeCommand("/mnt/SDCARD/.simplemenu/hide", "Scraper.sh", "#", 0);
		}
		#else
		#endif
	} else if (keys[BTN_B]) {
		#if defined TARGET_OD
		if (hdmiChanged!=hdmiEnabled) {
			FILE *fp = fopen("/sys/class/hdmi/hdmi","w");
			if (fp!=NULL) {
				fprintf(fp, "%d", hdmiChanged);
				fclose(fp);
			}
			saveLastState();
			saveFavorites();
			clearTimer();
			clearPicModeHideLogoTimer();
			clearPicModeHideMenuTimer();
			freeResources();
			execlp("./simplemenu","invoker",NULL);
		}
		#endif
		currentState=previousState;
		previousState=SETTINGS_SCREEN;
		if(themeChanged!=activeTheme){
			int headerAndFooterBackground[3]={37,50,56};
			drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(22), 0, SCREEN_HEIGHT-calculateProportionalSizeOrDistance1(22), headerAndFooterBackground);
			drawLoadingText();
			char *temp=malloc(8000);
			strcpy(temp,themes[activeTheme]);
			strcat(temp,"/theme.ini");
			loadTheme(temp);
			loadSectionGroups();
			free(temp);
			MENU_ITEMS_PER_PAGE=itemsPerPage;
			FULLSCREEN_ITEMS_PER_PAGE=itemsPerPageFullscreen;
		}
		if (CURRENT_SECTION.backgroundSurface==NULL) {
			logMessage("INFO","performSettingsChoosingAction","Loading system background");
			CURRENT_SECTION.backgroundSurface = IMG_Load(CURRENT_SECTION.background);
			logMessage("INFO","performSettingsChoosingAction","Loading system picture");
			CURRENT_SECTION.systemPictureSurface = IMG_Load(CURRENT_SECTION.systemPicture);
		}
		if(fullscreenMode==0) {
			ITEMS_PER_PAGE=MENU_ITEMS_PER_PAGE;
		} else {
			ITEMS_PER_PAGE=FULLSCREEN_ITEMS_PER_PAGE;
		}
		if(themeChanged!=activeTheme || CURRENT_SECTION.initialized==0) {
			if (currentSectionNumber!=favoritesSectionNumber) {
				if (CURRENT_SECTION.initialized==0) {
					loadGameList(0);
				} else {
					loadGameList(2);
				}
			} else {
				loadFavoritesSectionGameList();
			}
		}
	}
}

void performChoosingAction() {
	struct Rom *rom = CURRENT_SECTION.currentGameNode->data;
	if (keys[BTN_UP]) {
		chosenChoosingOption--;
		if (chosenChoosingOption<0) {
			chosenChoosingOption=2;
		}
	} else if (keys[BTN_DOWN]) {
		chosenChoosingOption++;
		if (chosenChoosingOption>2) {
			chosenChoosingOption=0;
		}
	} else if (keys[BTN_LEFT]) {
		if(chosenChoosingOption==0) {
#if defined TARGET_OD_BETA || defined TARGET_RFW || defined TARGET_BITTBOY || defined TARGET_PC
			if (rom->preferences.frequency==OC_NO) {
				rom->preferences.frequency=OCValue;
			} else {
				rom->preferences.frequency=OC_NO;
			}
#endif
		} else if (chosenChoosingOption == 1){
			launchAtBoot = 1+-1*launchAtBoot;
			if (launchAtBoot==1) {
				setLaunchAtBoot(rom);
			} else {
				setLaunchAtBoot(NULL);
			}
		} else {
			if(rom->preferences.emulator>0) {
				rom->preferences.emulator--;
				rom->preferences.emulatorDir--;
			} else {
				rom->preferences.emulator=sizeof(CURRENT_SECTION.executables)/sizeof(CURRENT_SECTION.executables[0])-1;
				rom->preferences.emulatorDir=sizeof(CURRENT_SECTION.executables)/sizeof(CURRENT_SECTION.executables[0])-1;
				while (rom->preferences.emulator>0&&CURRENT_SECTION.executables[rom->preferences.emulator]==NULL) {
					rom->preferences.emulator--;
					rom->preferences.emulatorDir--;
				}
			}
		}
	} else 	if (keys[BTN_RIGHT]) {
		if(chosenChoosingOption==0) {
#if defined TARGET_OD_BETA || defined TARGET_RFW || defined TARGET_BITTBOY || defined TARGET_PC
			if (rom->preferences.frequency==OC_NO) {
				rom->preferences.frequency=OCValue;
			} else {
				rom->preferences.frequency=OC_NO;
			}
#endif
		}
		else if (chosenChoosingOption == 1){
			launchAtBoot = 1+-1*launchAtBoot;
			if (launchAtBoot==1) {
				setLaunchAtBoot(rom);
			} else {
				setLaunchAtBoot(NULL);
			}
		} else {
			if(CURRENT_SECTION.executables[rom->preferences.emulator+1]!=NULL) {
				rom->preferences.emulator++;
				rom->preferences.emulatorDir++;
			} else {
				rom->preferences.emulator=0;
				rom->preferences.emulatorDir=0;
			}
		}
	} else	if (keys[BTN_B]) {
		#if defined MIYOOMINI
		if (currentState!=BROWSING_GAME_LIST) {
			int emu = CURRENT_SECTION.currentGameNode->data->preferences.emulator;
			int emuDir = CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir;
			loadRomPreferences(CURRENT_SECTION.currentGameNode->data);
			if (CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir!=emuDir  || CURRENT_SECTION.currentGameNode->data->preferences.emulator!=emu) {
				CURRENT_SECTION.currentGameNode->data->preferences.emulator=emu;
				CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir=emu;
				saveRomPreferences(CURRENT_SECTION.currentGameNode->data);
			}
			if (getLaunchAtBoot()!=NULL) {
				launchGame(CURRENT_SECTION.currentGameNode->data);
			}
			previousState=SELECTING_EMULATOR;
			currentState=BROWSING_GAME_LIST;
		}
		#else
		if (currentState!=BROWSING_GAME_LIST) {
			int emu = CURRENT_SECTION.currentGameNode->data->preferences.emulator;
			int emuDir = CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir;
			int hz = CURRENT_SECTION.currentGameNode->data->preferences.frequency;
			loadRomPreferences(CURRENT_SECTION.currentGameNode->data);
			if (CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir!=emuDir  || CURRENT_SECTION.currentGameNode->data->preferences.emulator!=emu || CURRENT_SECTION.currentGameNode->data->preferences.frequency!=hz) {
				CURRENT_SECTION.currentGameNode->data->preferences.emulator=emu;
				CURRENT_SECTION.currentGameNode->data->preferences.emulatorDir=emu;
				CURRENT_SECTION.currentGameNode->data->preferences.frequency=hz;
				saveRomPreferences(CURRENT_SECTION.currentGameNode->data);
			}
			if (getLaunchAtBoot()!=NULL) {
				launchGame(CURRENT_SECTION.currentGameNode->data);
			}
			previousState=SELECTING_EMULATOR;
			currentState=BROWSING_GAME_LIST;
		}
		#endif
	}
}

void callDeleteGame(struct Rom *rom) {
	if (!isFavoritesSectionSelected()) {
		deleteGame(rom);
		loadGameList(1);
		while(CURRENT_SECTION.hidden) {
			rewindSection();
			loadGameList(0);
			hideFullScreenModeMenu();
		}
		if(CURRENT_SECTION.currentGameInPage==countGamesInPage()) {
			scrollUp();
		}
		setupDecorations();
	} else {
		generateError("YOU CAN'T DELETE GAMES-WHILE IN FAVORITES",0);
	}
}
