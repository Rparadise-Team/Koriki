#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/logic.h"
#include "../headers/screen.h"
#include "../headers/string_utils.h"
#include "../headers/ini2.h"
#include "../headers/graphics.h"
#include "../headers/utils.h"
#include "../headers/config.h"

char home[5000];
char pathToThemeConfigFile[1000];
char pathToThemeConfigFilePlusFileName[1000];

int atoifgl(const char* value) {
	if (value!=NULL) {
		return atoi(value);
	} else {
		return 0;
	}

}

void loadAliasList(int sectionNumber) {
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *aliasFile=getCurrentSectionAliasFile();
	if (aliasFile==NULL) {
		strcpy(menuSections[sectionNumber].aliasFileName," ");
		return;
	}
	menuSections[sectionNumber].aliasHashTable = ht_create(MAX_GAMES_IN_SECTION);
	while ((read = getline(&line, &len, aliasFile)) != -1) {
		char *romName = strtok(line, "=");
		char *alias = strtok(NULL, "=");
		int lastChar = strlen(alias)-1;
		alias[lastChar]='\0';
		ht_set(menuSections[sectionNumber].aliasHashTable, romName, alias);
	}
	if (line) {
		free(line);
	}
	fclose(aliasFile);
}

void checkIfDefault() {
	#ifdef MIYOOMINI
	#else
	FILE *fp=NULL;
	FILE *fpScripts=NULL;
	#endif
	#ifdef TARGET_BITTBOY
	logMessage("INFO","checkIfDefault","Checking if default bittboy");
	fp = fopen("/mnt/autoexec.sh", "r");
	fpScripts = fopen("scripts/autoexec.sh", "r");
	#endif
	#ifdef TARGET_RFW
	fp = fopen("/home/retrofw/autoexec.sh", "r");
	fpScripts = fopen("scripts/autoexec.sh", "r");
	#endif
	#ifdef TARGET_OD
	fp = fopen("/media/data/local/sbin/frontend_start", "r");
	fpScripts = fopen("scripts/frontend_start", "r");
	#endif
	#ifdef TARGET_OD_BETA
	fp = fopen("/media/data/local/home/.autostart", "r");
	fpScripts = fopen("scripts/frontend_start", "r");
	#endif
	#ifdef TARGET_NPG
	fp = fopen("/media/data/local/sbin/frontend_start", "r");
	fpScripts = fopen("scripts/frontend_start", "r");
	#endif
	#ifdef MIYOOMINI
	//In Miyoo Mini there is no optional startup script.
	//The closest is /mnt/SDCARD/.tmp_update/updater but we can't count on it because
	//it is used by most distributions, so we control the quit menu manually by means
	//of the define MM_NOQUIT.
	#ifdef MM_NOQUIT
	shutDownEnabled=1;
	#else
	shutDownEnabled=0;
	#endif
	#else
	#ifndef MIYOOMINI
	shutDownEnabled=1;
	int sameFile=1;
	int c1, c2;
	if (fp==NULL) {
		shutDownEnabled=0;
		return;
	}
	c1 = getc(fp);
	c2 = getc(fpScripts);
	logMessage("INFO", "checkIfDefault", "Starting comparison");
	while (sameFile && c1 != EOF && c2 != EOF) {
		if (c1 != c2) {
			sameFile=0;
			shutDownEnabled=0;
			break;
		}
		c1 = getc(fp);
		c2 = getc(fpScripts);
	}
	logMessage("INFO","checkIfDefault","Done comparing");
	if (fp!=NULL) {
		fclose(fp);
	}
	if (fpScripts!=NULL) {
		fclose(fpScripts);
	}
	#endif
	#endif
	logMessage("INFO","checkIfDefault","Default state checked");
}

int isLaunchAtBoot(char *romName) {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	char pathToAutostartFilePlusFileName[300];
	snprintf(pathToAutostartFilePlusFileName,sizeof(pathToAutostartFilePlusFileName),"%s/.simplemenu/rom_preferences/autostart.rom",home);

	fp = fopen(pathToAutostartFilePlusFileName, "r");
	if (fp==NULL) {
		return 0;
	}
	int i = getline(&line, &len, fp);
	if (i==-1) {
		logMessage("ERROR", "isLaunchAtBoot", "Error reading line");
	}
	line[strlen(line)-1] = '\0';
	if (!strcmp(line,romName)) {
		fclose(fp);
		return 1;
	}
	fclose(fp);
	return 0;
}

int wasRunningFlag() {
	FILE * fp;
	char pathToRunningFlagFilePlusFileName[300];
	snprintf(pathToRunningFlagFilePlusFileName,sizeof(pathToRunningFlagFilePlusFileName),"%s/.simplemenu/rom_preferences/is_running.flg",home);
	fp = fopen(pathToRunningFlagFilePlusFileName, "rw");
	if (fp==NULL) {
		return 0;
	}
	fclose(fp);
	remove(pathToRunningFlagFilePlusFileName);
	return 1;
}

void setRunningFlag() {
	FILE * fp;
	char pathToRunningFlagFilePlusFileName[300];
	snprintf(pathToRunningFlagFilePlusFileName,sizeof(pathToRunningFlagFilePlusFileName),"%s/.simplemenu/rom_preferences/is_running.flg",home);
	fp = fopen(pathToRunningFlagFilePlusFileName, "w");
	fprintf(fp,"%d", 1);
	fclose(fp);
}

void setLaunchAtBoot(struct Rom *rom) {
	FILE * fp;
	char pathToAutostartFilePlusFileName[300];
	snprintf(pathToAutostartFilePlusFileName,sizeof(pathToAutostartFilePlusFileName),"%s/.simplemenu/rom_preferences/autostart.rom",home);
	if (rom==NULL) {
		remove(pathToAutostartFilePlusFileName);
	} else {
		fp = fopen(pathToAutostartFilePlusFileName, "w");
		fprintf(fp,"%s\n", rom->name);
		fprintf(fp,"%s\n", rom->directory);
		if (rom->alias!=NULL) {
			fprintf(fp,"%s\n", rom->alias);
		} else {
			fprintf(fp,"\n");
		}
		fprintf(fp,"%d\n", rom->isConsoleApp);
		fprintf(fp,"%s\n", CURRENT_SECTION.emulatorDirectories[rom->preferences.emulatorDir]);
		fprintf(fp,"%s", CURRENT_SECTION.executables[rom->preferences.emulator]);
		fclose(fp);
	}
}

struct AutostartRom *getLaunchAtBoot() {
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	char pathToAutostartFilePlusFileName[300];
	snprintf(pathToAutostartFilePlusFileName,sizeof(pathToAutostartFilePlusFileName),"%s/.simplemenu/rom_preferences/autostart.rom",home);
	fp = fopen(pathToAutostartFilePlusFileName, "r");
	if (fp==NULL) {
		return NULL;
	}
	struct Rom *rom = malloc(sizeof(struct Rom));
	int i = getline(&line, &len, fp);
	rom->name=strdup(line);

	i = getline(&line, &len, fp);
	rom->directory=strdup(line);

	i = getline(&line, &len, fp);
	rom->alias=strdup(line);

	i = getline(&line, &len, fp);
	rom->isConsoleApp=atoifgl(line);

	loadRomPreferences(rom);
	struct AutostartRom *autostartRom = malloc(sizeof(struct AutostartRom));
	autostartRom->rom = rom;

	i = getline(&line, &len, fp);
	autostartRom->emulatorDir = strdup(line);

	i = getline(&line, &len, fp);
	if (i==-1) {
		logMessage("ERROR", "getLaunchAtBoot", "Error reading line");
	}
	autostartRom->emulator = strdup(line);
	fclose(fp);
	return autostartRom;
}

uint32_t hex2int(char *hex) {
	char *hexCopy = malloc(strlen(hex)+1);
	strcpy(hexCopy,hex);
	uint32_t val = 0;
	int i=0;
	while (*hexCopy) {
		i++;
		uint8_t byte = *hexCopy++;
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
		val = (val << 4) | (byte & 0xF);
	}
	hexCopy-=i;
	free(hexCopy);
	return val;
}

void setRGBFromHex (int *rgbColor, const char *hexColor) {
	char r[3];
	char g[3];
	char b[3];
	r[0]=hexColor[0];
	r[1]=hexColor[1];
	r[2]='\0';
	g[0]=hexColor[2];
	g[1]=hexColor[3];
	g[2]='\0';
	b[0]=hexColor[4];
	b[1]=hexColor[5];
	b[2]='\0';
	rgbColor[0]=hex2int(r);
	rgbColor[1]=hex2int(g);
	rgbColor[2]=hex2int(b);
}


void setStringValueInSection(ini_t *config, char *sectionName, char *valueName, char *sectionValueToBeSet,char *fallback) {
	const char *value;
	value = ini_get(config, sectionName, valueName);
	if(value==NULL) {
		strcpy(sectionValueToBeSet,fallback);
		return;
	}
	strcpy(sectionValueToBeSet,value);
}

void setRGBColorInSection(ini_t *config, char *sectionName, char *valueName, int *sectionValueToBeSet) {
	const char *value;
	value = ini_get(config, sectionName, valueName);
	if (value==NULL) {
		value = ini_get(config, "DEFAULT", valueName);
	}
	setRGBFromHex(sectionValueToBeSet, value);
}

void setThemeResourceValueInSection(ini_t *config, char *sectionName, char *valueName, char *sectionValueToBeSet) {
	const char *value;
	value = ini_get(config, sectionName, valueName);
	if(value==NULL) {
		value = ini_get(config, "DEFAULT", valueName);
		if (value==NULL) {
			value = ini_get(config, "GENERAL", valueName);
		}
		if (value==NULL) {
			strcpy(sectionValueToBeSet, "");
			return;
		}
		strcpy(sectionValueToBeSet,pathToThemeConfigFile);
		strcat(sectionValueToBeSet,value);
		return;
	}
	strcpy(sectionValueToBeSet,pathToThemeConfigFile);
	strcat(sectionValueToBeSet,value);
}


void loadTheme(char *theme) {
	strcpy(pathToThemeConfigFilePlusFileName,theme);
	char *romPath = getRomPath(theme);
	char *temp = malloc(3000);
	strcpy(temp,romPath);
	strcat(temp,"/");
	strcpy(pathToThemeConfigFile,temp);
	free(temp);
	free(romPath);
	ini_t *themeConfig = ini_load(theme);
	const char *value;
	for (int i=0;i<menuSectionCounter;i++) {
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "fullscreen_menu_background_color", menuSections[i].fullScreenMenuBackgroundColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "fullscreen_menu_font_color", menuSections[i].fullscreenMenuItemsColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "items_font_color", menuSections[i].menuItemsFontColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "selected_item_background_color", menuSections[i].bodySelectedTextBackgroundColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "selected_item_font_color", menuSections[i].bodySelectedTextTextColor);

		value = ini_get(themeConfig, "DEFAULT", "art_font_color");
		if (value!=NULL) {
			setRGBColorInSection(themeConfig, menuSections[i].sectionName, "art_font_color", menuSections[i].pictureTextColor);
		} else {
			setRGBColorInSection(themeConfig, menuSections[i].sectionName, "items_font_color", menuSections[i].pictureTextColor);
		}

		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "system", menuSections[i].systemPicture);
		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "logo", menuSections[i].systemLogo);
		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "no_art_picture", menuSections[i].noArtPicture);
		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "background", menuSections[i].background);

		value = ini_get(themeConfig, "GENERAL", "system_w");
		systemWidth = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "system_h");
		systemHeight = atoifgl(value);

		if (menuSections[i].systemLogoSurface!=NULL) {
			logMessage("INFO","loadTheme","Freeing system logo");
			SDL_FreeSurface(menuSections[i].systemLogoSurface);
			menuSections[i].systemLogoSurface = NULL;
		}
		if (menuSections[i].systemPictureSurface!=NULL) {
			logMessage("INFO","loadTheme","Freeing system picture");
			SDL_FreeSurface(menuSections[i].systemPictureSurface);
			menuSections[i].systemPictureSurface = NULL;
		}
		if (menuSections[i].backgroundSurface!=NULL) {
			logMessage("INFO","loadTheme","Freeing system background");
			SDL_FreeSurface(menuSections[i].backgroundSurface);
			menuSections[i].backgroundSurface = NULL;
		}

		if(i==currentSectionNumber) {
			logMessage("INFO","loadTheme","Loading system logo");
			menuSections[i].systemLogoSurface = IMG_Load(menuSections[i].systemLogo);
			logMessage("INFO","loadTheme","Loading system background");
			menuSections[i].backgroundSurface = IMG_Load(menuSections[i].background);
			logMessage("INFO","loadTheme","Loading system picture");
			menuSections[i].systemPictureSurface = IMG_Load(menuSections[i].systemPicture);
		}

		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "system", menuSections[i].systemPicture);

		setThemeResourceValueInSection (themeConfig, "GENERAL", "favorite_indicator", favoriteIndicator);

		setThemeResourceValueInSection (themeConfig, "GENERAL", "font", menuFont);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "section_groups_folder", sectionGroupsFolder);

		value = ini_get(themeConfig, "GENERAL", "colorful_fullscreen_menu");
		if (value == NULL) {
			colorfulFullscreenMenu = 0;
		} else {
			colorfulFullscreenMenu = atoifgl(value);
		}

		value = ini_get(themeConfig, "GENERAL", "font_outline");
		if (value == NULL) {
			fontOutline = 0;
		} else {
			fontOutline = atoifgl(value);
		}

		value = ini_get(themeConfig, "GENERAL", "display_section_group_name");
		if (value == NULL) {
			displaySectionGroupName = 0;
		} else {
			displaySectionGroupName = atoifgl(value);
		}

		value = ini_get(themeConfig, "GENERAL", "show_art");
		if (value == NULL) {
			showArt = 1;
		} else {
			showArt = atoifgl(value);
		}

		value = ini_get(themeConfig, "GENERAL", "items");
		itemsPerPage = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "items_separation");
		itemsSeparation = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "items_in_fullscreen_mode");
		itemsPerPageFullscreen = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "game_list_alignment");
		gameListAlignment = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "game_list_x");
		gameListX = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "game_list_y");
		gameListY = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "game_list_w");
		gameListWidth = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "game_list_position_in_full");
		gameListPositionFullScreen = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_max_w");
		artWidth = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_max_h");
		artHeight = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_x");
		artX = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_y");
		artY = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "system_x");
		systemX = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "system_y");
		systemY = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "font_size");
		fontSize = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "text1_font_size");
		text1FontSize = atoi (value);

		value = ini_get(themeConfig, "GENERAL", "text1_x");
		text1X = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "text1_y");
		text1Y = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "text1_alignment");
		text1Alignment = atoifgl(value);

		setThemeResourceValueInSection (themeConfig, "GENERAL", "textX_font", textXFont);

		battX = -1;
		value = ini_get(themeConfig, "GENERAL", "batt_x");
		if(value!=NULL) {
			battX = atoifgl(value);
			value = ini_get(themeConfig, "GENERAL", "batt_y");
			battY = atoifgl(value);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_1", batt1);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_2", batt2);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_3", batt3);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_4", batt4);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_5", batt5);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_6", batt6);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_7", batt7);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_8", batt8);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_9", batt9);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_10", batt10);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_11", batt11);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_12", batt12);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_13", batt13);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_14", batt14);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_15", batt15);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_16", batt16);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_17", batt17);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_18", batt18);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_19", batt19);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_20", batt20);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_charging", battCharging);
			surfaceBatt1 = IMG_Load(batt1);
			surfaceBatt2 = IMG_Load(batt2);
			surfaceBatt3 = IMG_Load(batt3);
			surfaceBatt4 = IMG_Load(batt4);
			surfaceBatt5 = IMG_Load(batt5);
			surfaceBatt6 = IMG_Load(batt6);
			surfaceBatt7 = IMG_Load(batt7);
			surfaceBatt8 = IMG_Load(batt8);
			surfaceBatt9 = IMG_Load(batt9);
			surfaceBatt10 = IMG_Load(batt10);
			surfaceBatt11 = IMG_Load(batt11);
			surfaceBatt12 = IMG_Load(batt12);
			surfaceBatt13 = IMG_Load(batt13);
			surfaceBatt14 = IMG_Load(batt14);
			surfaceBatt15 = IMG_Load(batt15);
			surfaceBatt16 = IMG_Load(batt16);
			surfaceBatt17 = IMG_Load(batt17);
			surfaceBatt18 = IMG_Load(batt18);
			surfaceBatt19 = IMG_Load(batt19);
			surfaceBatt20 = IMG_Load(batt20);
			surfaceBattCharging = IMG_Load(battCharging);
		}
		
		wifiX = -1;
		value = ini_get(themeConfig, "GENERAL", "wifi_x");
		if(value!=NULL) {
			wifiX = atoifgl(value);
			value = ini_get(themeConfig, "GENERAL", "wifi_y");
			wifiY = atoifgl(value);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "wifioff", wifioff);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "wifion", wifion);
			setThemeResourceValueInSection (themeConfig, "GENERAL", "nowifi", nowifi);
			surfaceWifiOff = IMG_Load(wifioff);
			surfaceWifiOn = IMG_Load(wifion);
			surfaceNoWifi = IMG_Load(nowifi);
		}

		setThemeResourceValueInSection (themeConfig, "GENERAL", "game_count_font", gameCountFont);

		value = ini_get(themeConfig, "GENERAL", "display_game_count");
		displayGameCount=0;
		if (value!=NULL && atoi(value)==1) {
			displayGameCount=1;
			setThemeResourceValueInSection (themeConfig, "GENERAL", "game_count_font", gameCountFont);
			strcpy (gameCountText, "# Games Available");
			value = ini_get(themeConfig, "GENERAL", "game_count_text");
			if(value!=NULL) {
				strcpy (gameCountText, value);
			}
			char *temp = replaceWord(gameCountText, "#", "%d");
			strcpy (gameCountText, temp);
			free(temp);
			value = ini_get(themeConfig, "GENERAL", "game_count_font_size");
			gameCountFontSize = atoifgl(value);
			value = ini_get(themeConfig, "GENERAL", "game_count_x");
			gameCountX = atoifgl(value);
			value = ini_get(themeConfig, "GENERAL", "game_count_y");
			gameCountY = atoifgl(value);
			value = ini_get(themeConfig, "GENERAL", "game_count_alignment");
			gameCountAlignment = atoifgl(value);
			value = ini_get(themeConfig, "GENERAL", "game_count_font_color");
			setRGBFromHex(gameCountFontColor, value);
		}

		value = ini_get(themeConfig, "GENERAL", "newspaper_mode");
		if (value==NULL) {
			newspaperMode = 0;
		} else {
			newspaperMode = atoi (value);
		}

		value = ini_get(themeConfig, "GENERAL", "text2_font_size");
		text2FontSize = atoi (value);

		value = ini_get(themeConfig, "GENERAL", "text2_x");
		text2X = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "text2_y");
		text2Y = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "text2_alignment");
		text2Alignment = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_text_distance_from_picture");
		artTextDistanceFromPicture = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_text_line_separation");
		artTextLineSeparation = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "art_text_font_size");
		artTextFontSize = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "font_size");
		baseFont = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "transparent_shading");
		transparentShading  = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "fullscreen_footer_on_top");
		footerOnTop = atoifgl(value);

		value = ini_get(themeConfig, menuSections[i].sectionName, "name");
		if (value!=NULL) {
			strcpy(menuSections[i].fantasyName, value);
		} else {
			strcpy(menuSections[i].fantasyName, "\0");
		}

    	currentMode=3;
    	MENU_ITEMS_PER_PAGE=itemsPerPage;
    	FULLSCREEN_ITEMS_PER_PAGE=itemsPerPageFullscreen;

		freeFonts();
		freeSettingsFonts();
		initializeSettingsFonts();
		initializeFonts();
	}
	ini_free(themeConfig);
}

void checkThemes() {
	char *files[1000];
	char tempString[1000];
	snprintf(tempString,sizeof(tempString),"%s/.simplemenu/themes/%dx%d/",getenv("HOME"),SCREEN_WIDTH,SCREEN_HEIGHT);
	int n = findDirectoriesInDirectory(tempString, files, 0);
	qsort(files, n, sizeof(const char*), sortStringArray);
	for(int i=0;i<n;i++) {
		themes[i]=malloc(strlen(files[i])+1);
		strcpy(themes[i],files[i]);
		free(files[i]);
		themeCounter++;
	}
	logMessage("INFO","checkThemes","Themes checked");
}

void createConfigFilesInHomeIfTheyDontExist() {
	snprintf(home,sizeof(home),"%s",getenv("HOME"));
	char pathToConfigFiles[5000];
	char pathToAppFiles[5000];
	char pathToGameFiles[5000];
	char pathToTempFiles[5000];
	char pathToRomPreferencesFiles[5000];
	char pathToSectionGroupsFiles[5000];
	snprintf(pathToConfigFiles,sizeof(pathToConfigFiles),"%s/.simplemenu",home);
	snprintf(pathToAppFiles,sizeof(pathToAppFiles),"%s/.simplemenu/apps",home);
	snprintf(pathToGameFiles,sizeof(pathToGameFiles),"%s/.simplemenu/games",home);
	snprintf(pathToSectionGroupsFiles,sizeof(pathToSectionGroupsFiles),"%s/.simplemenu/section_groups",home);
	snprintf(pathToTempFiles,sizeof(pathToTempFiles),"%s/.simplemenu/tmp",home);
	snprintf(pathToRomPreferencesFiles,sizeof(pathToRomPreferencesFiles),"%s/.simplemenu/rom_preferences",home);
	int directoryExists=mkdir(pathToConfigFiles,0700);
	if (!directoryExists) {
		char copyCommand[5000];
		snprintf(copyCommand,sizeof(copyCommand),"cp config/* %s/.simplemenu",home);
		int ret = system(copyCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copyAppsCommand[5000];
		mkdir(pathToAppFiles,0700);
		snprintf(copyAppsCommand,sizeof(copyAppsCommand),"cp apps %s/.simplemenu",home);
		ret = system(copyAppsCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copyGamesCommand[5000];
		mkdir(pathToGameFiles,0700);
		snprintf(copyGamesCommand,sizeof(copyGamesCommand),"cp games %s/.simplemenu",home);
		ret = system(copyGamesCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copySectionGroupsCommand[5000];
		mkdir(pathToSectionGroupsFiles,0700);
		snprintf(copySectionGroupsCommand,sizeof(copySectionGroupsCommand),"cp -r section_groups %s/.simplemenu",home);
		ret = system(copySectionGroupsCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		ret = system("scripts/consoles.sh");
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		ret = system("scripts/handhelds.sh");
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		ret = system("scripts/arcades.sh");
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		ret = system("scripts/home computers.sh");
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		mkdir(pathToTempFiles,0700);
		mkdir(pathToRomPreferencesFiles,0700);
	}
	logMessage("INFO","createConfigFilesInHomeIfTheyDontExist","Validated configuration existence");
}

void createThemesInHomeIfTheyDontExist() {
	char pathToThemeFiles[5000];
	snprintf(pathToThemeFiles,sizeof(pathToThemeFiles),"%s/.simplemenu/themes",home);
	int directoryExists=mkdir(pathToThemeFiles,0700);
	if (!directoryExists) {
		drawCopyingText();
		refreshScreen();
		int ret=0;
		char copyThemesCommand[5000];
		mkdir(pathToThemeFiles,0700);
		snprintf(copyThemesCommand,sizeof(copyThemesCommand),"cp -r themes %s/.simplemenu",home);
		ret = system(copyThemesCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
	}
}


void saveRomPreferences(struct Rom *rom) {
	FILE * fp;
	char pathToPreferencesFilePlusFileName[300];
	snprintf(pathToPreferencesFilePlusFileName,sizeof(pathToPreferencesFilePlusFileName),"%s/.simplemenu/rom_preferences/%s/%s", home, CURRENT_SECTION.sectionName, getNameWithoutPath(rom->name));
	fp = fopen(pathToPreferencesFilePlusFileName, "w");
	fprintf(fp,"%d;", rom->preferences.emulatorDir);
	fprintf(fp,"%d;", rom->preferences.emulator);
	#if defined MIYOOMINI
	#else
	fprintf(fp,"%d", rom->preferences.frequency);
	#endif
	fclose(fp);
}

void loadRomPreferences(struct Rom *rom) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	char pathToPreferencesFilePlusFileName[800];
	char pathToPreferencesFiles[800];
	if (currentSectionNumber!=favoritesSectionNumber) {
		snprintf(pathToPreferencesFilePlusFileName,sizeof(pathToPreferencesFilePlusFileName),"%s/.simplemenu/rom_preferences/%s/%s",home, CURRENT_SECTION.sectionName, getNameWithoutPath(rom->name));
		snprintf(pathToPreferencesFiles,sizeof(pathToPreferencesFiles),"%s/.simplemenu/rom_preferences/%s", home, CURRENT_SECTION.sectionName);
		mkdir(pathToPreferencesFiles,0700);
	} else {
		snprintf(pathToPreferencesFilePlusFileName,sizeof(pathToPreferencesFilePlusFileName),"%s/.simplemenu/rom_preferences/%s/%s",home, favorites[CURRENT_GAME_NUMBER].section, getNameWithoutPath(rom->name));
		snprintf(pathToPreferencesFiles,sizeof(pathToPreferencesFiles),"%s/.simplemenu/rom_preferences/%s", home, favorites[CURRENT_GAME_NUMBER].section);
	}

	rom->preferences.emulatorDir=0;
	rom->preferences.emulator=0;
	#if defined MIYOOMINI
	#else
	rom->preferences.frequency=OC_NO;
	#endif

	fp = fopen(pathToPreferencesFilePlusFileName, "r");

	if (fp==NULL) {
		return;
	}
	char *configurations[4];
	char *ptr;
	int getLineResult = getline(&line, &len, fp);
	if (getLineResult==-1) {
		logMessage("ERROR", "loadRomPreferences", "Error reading line");
	}
	ptr = strtok(line, ";");
	int i=0;
	while(ptr != NULL) {
		configurations[i]=ptr;
		ptr = strtok(NULL, ";");
		i++;
	}
	rom->preferences.emulatorDir=atoifgl(configurations[0]);
	rom->preferences.emulator=atoifgl(configurations[1]);
	#if defined MIYOOMINI
	#else
	rom->preferences.frequency = atoifgl(configurations[2]);
	#endif
    fclose(fp);
}

void saveFavorites() {
	if (favoritesChanged) {
		FILE * fp;
		char pathToFavoritesFilePlusFileName[300];
		snprintf(pathToFavoritesFilePlusFileName,sizeof(pathToFavoritesFilePlusFileName),"%s/.simplemenu/favorites.sav",home);
		fp = fopen(pathToFavoritesFilePlusFileName, "w");
		int linesWritten=0;
		for (int j=0;j<favoritesSize;j++) {
			struct Favorite favorite = favorites[j];
			if (strlen(favorite.name)==1) {
				break;
			}
			if(linesWritten>0) {
				fprintf(fp,"\n");
			}
			fprintf(fp,"%s;",favorite.section);
			if(favorite.sectionAlias[0]=='\0') {
				fprintf(fp," ;");
			} else {
				fprintf(fp,"%s;",favorite.sectionAlias);
			}
			fprintf(fp,"%s;",favorite.name);
			if(favorite.alias[0]=='\0') {
				fprintf(fp," ;");
			} else {
				fprintf(fp,"%s;",favorite.alias);
			}
			fprintf(fp,"%s;",favorite.emulatorFolder);
			fprintf(fp,"%s;",favorite.executable);
			fprintf(fp,"%d;",favorite.isConsoleApp);
			fprintf(fp,"%s;",favorite.filesDirectory);
			#if defined MIYOOMINI
			#else
			fprintf(fp,"%d",favorite.frequency);
			#endif
			linesWritten++;
		}
		fclose(fp);
		favoritesChanged=0;
	}
}

void loadFavorites() {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char pathToFavoritesFilePlusFileName[300];
	snprintf(pathToFavoritesFilePlusFileName,sizeof(pathToFavoritesFilePlusFileName),"%s/.simplemenu/favorites.sav",home);
	fp = fopen(pathToFavoritesFilePlusFileName, "r");
	if (fp==NULL) {
		generateError("FAVORITES FILE NOT FOUND",1);
		return;
	}
	char *configurations[10];
	char *ptr;
	favoritesSize=0;
	while ((read = getline(&line, &len, fp)) != -1) {
		ptr = strtok(line, ";");
		int i=0;
		while(ptr != NULL) {
			configurations[i]=ptr;
			ptr = strtok(NULL, ";");
			i++;
		}
		strcpy(favorites[favoritesSize].section,configurations[0]);
		strcpy(favorites[favoritesSize].sectionAlias,configurations[1]);
		strcpy(favorites[favoritesSize].name,configurations[2]);
		strcpy(favorites[favoritesSize].alias,configurations[3]);
		strcpy(favorites[favoritesSize].emulatorFolder,configurations[4]);
		strcpy(favorites[favoritesSize].executable,configurations[5]);
		favorites[favoritesSize].isConsoleApp = atoi(configurations[6]);
		strcpy(favorites[favoritesSize].filesDirectory,configurations[7]);
		#if defined MIYOOMINI
		#else
		favorites[favoritesSize].frequency = atoi(configurations[8]);
		#endif
		int len = strlen(favorites[favoritesSize].filesDirectory);
		if (favorites[favoritesSize].filesDirectory[len-1]=='\n') {
			favorites[favoritesSize].filesDirectory[len-1]='\0';
		}
		favoritesSize++;
	}
	fclose(fp);
	if (line) {
		free(line);
	}
	qsort(favorites, favoritesSize, sizeof(struct Favorite), compareFavorites);
	logMessage("INFO","loadFavorites","Loaded favorites");
}

int cmpfnc(const void *f1, const void *f2)
{
	struct SectionGroup *e1 = (struct SectionGroup *)f1;
	struct SectionGroup *e2 = (struct SectionGroup *)f2;
	char temp1[300]="";
	char temp2[300]="";
	strcpy(temp1,e1->groupName);
	strcpy(temp2,e2->groupName);
	for(int i=0;temp1[i]; i++) {
		temp1[i] = tolower(temp1[i]);
	}
	for(int i=0;temp2[i]; i++) {
		temp2[i] = tolower(temp2[i]);
	}
	return strcmp(temp1, temp2);
}


void loadConfig() {
	char pathToConfigFilePlusFileName[1000];
	const char *value;
	snprintf(pathToConfigFilePlusFileName,sizeof(pathToConfigFilePlusFileName),"%s/.simplemenu/config.ini",home);
	ini_t *config = ini_load(pathToConfigFilePlusFileName);

	value = ini_get(config, "GENERAL", "media_folder");
	strcpy(mediaFolder,value);

	value = ini_get(config, "GENERAL", "logging_enabled");

	if (atoifgl(value)==1) {
		enableLogging();
	}

	value = ini_get(config, "GENERAL", "cache");
	useCache = atoifgl(value);

	value = ini_get(config, "GENERAL", "original_controls");
	if(value!=NULL) {
		alternateControls=atoifgl(value);
	} else {
		alternateControls=0;
	}

	value = ini_get(config, "CPU", "overclocked_speed_low");
	OC_OC_LOW=atoifgl(value);

	value = ini_get(config, "CPU", "normal_speed");
	OC_NO=atoifgl(value);

	value = ini_get(config, "CPU", "overclocked_speed_high");
	OC_OC_HIGH=atoifgl(value);

	value = ini_get(config, "CPU", "sleep_speed");
	OC_SLEEP=atoifgl(value);

	value = ini_get(config, "SCREEN", "hdmi_width");
	HDMI_WIDTH=atoifgl(value);

	value = ini_get(config, "SCREEN", "hdmi_height");
	HDMI_HEIGHT=atoifgl(value);

	value = ini_get(config, "CONTROLS", "A");
	if (value) {
		BTN_A = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "B");
	if (value) {
		BTN_B = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "X");
	if (value) {
		BTN_X = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "Y");
	if (value) {
		BTN_Y = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "L1");
	if (value) {
		BTN_L1 = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "L2");
	if (value) {
		BTN_L2 = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "R1");
	if (value) {
		BTN_R1 = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "R2");
	if (value) {
		BTN_R2 = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "UP");
	if (value) {
		BTN_UP = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "DOWN");
	if (value) {
		BTN_DOWN = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "LEFT");
	if (value) {
		BTN_LEFT = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "RIGHT");
	if (value) {
		BTN_RIGHT = atoifgl(value);
	}

	value = ini_get(config, "CONTROLS", "START");
	if (value) {
		BTN_START = atoifgl(value);
	}

        value = ini_get(config, "CONTROLS", "SELECT");
        if (value) {
                BTN_SELECT = atoifgl(value);
        }

        value = ini_get(config, "CONTROLS", "R");
        if (value) {
                BTN_R = atoifgl(value);
        }

        value = ini_get(config, "CONTROLS", "MENU");
        if (value) {
                BTN_MENU = atoifgl(value);
        } else {
                BTN_MENU = BTN_SELECT;
        }

	ini_free(config);
	logMessage("INFO","loadConfig","Config loaded");
}

void loadSectionGroups() {
	ini_t *themeConfig = ini_load(pathToThemeConfigFilePlusFileName);
	sectionGroupCounter=0;
	char *files[1000];
	char tempString[1000];

	setThemeResourceValueInSection (themeConfig, "GENERAL", "section_groups_folder", sectionGroupsFolder);

	snprintf(tempString,sizeof(tempString),"%s/.simplemenu/section_groups/",getenv("HOME"));

	int n = recursivelyScanDirectory(tempString, files, 0);

	for(int i=0;i<n;i++) {
		if(strstr(files[i],".png")!=NULL) {
			continue;
		}
		strcpy(sectionGroups[sectionGroupCounter].groupPath, files[i]);

		char *temp3 = malloc(3000);
		char *temp = getNameWithoutPath((files[i]));
		char *temp1 = getNameWithoutExtension(temp);
		char *sectionGroupPath = getRomPath(files[i]);

		if (strlen(sectionGroupsFolder)>1) {
			strcpy(temp3,sectionGroupsFolder);
		} else {
			strcpy(temp3,"\0");
		}
		strcat(temp3,temp1);
		strcat(temp3,".png");
		strcpy(sectionGroups[sectionGroupCounter].groupBackground, temp3);
		logMessage("INFO","loadSectionGroups","Loading group background");
		sectionGroups[sectionGroupCounter].groupBackgroundSurface=IMG_Load(sectionGroups[sectionGroupCounter].groupBackground);
		char *temp2 = toUpper(temp1);
		strcpy(sectionGroups[sectionGroupCounter].groupName, temp2);
		free(temp);
		free(temp1);
		free(temp2);
		free(temp3);
		free(sectionGroupPath);
		sectionGroupCounter++;
	}

	for (int i=0;i<n;i++){
		free(files[i]);
	}

	qsort(sectionGroups, sectionGroupCounter, sizeof(struct SectionGroup), cmpfnc);
	ini_free(themeConfig);
	logMessage("INFO","loadSectionGroups","Loaded section groups");
}

int loadSections(char *file) {
	menuSectionCounter=0;
	char pathToSectionsFilePlusFileName[1000];
	snprintf(pathToSectionsFilePlusFileName,sizeof(pathToSectionsFilePlusFileName),"%s",file);
	ini_t *config = ini_load(pathToSectionsFilePlusFileName);
	ini_t *themeConfig = ini_load(pathToThemeConfigFilePlusFileName);

	const char *consoles = ini_get(config, "CONSOLES", "consoleList");
	char *consoles1 = strdup(consoles);
	char *sectionNames[10000];
	const char *value;
	int sectionCounter=0;

	char* tokenizedSectionName=strtok(consoles1,",");

	while(tokenizedSectionName!=NULL) {
		value = ini_get(config, tokenizedSectionName, "execs");
		if(value!=NULL) {
			sectionNames[sectionCounter]=malloc(strlen(tokenizedSectionName)+1);
			strcpy(sectionNames[sectionCounter], tokenizedSectionName);
			sectionCounter++;
		}
		tokenizedSectionName=strtok(NULL,",");
	}
	free(tokenizedSectionName);
	free(consoles1);
	while(menuSectionCounter<sectionCounter) {
		char *sectionName = sectionNames[menuSectionCounter];
		logMessage("INFO","loadSections",sectionName);
		strcpy(menuSections[menuSectionCounter].sectionName, sectionName);
		int execCounter=0;
		value = ini_get(config, sectionName, "execs");
		char *value2 = malloc(3000);
		strcpy(value2,value);
		char* currentExec = strtok(value2,",");
		while(currentExec!=NULL) {
			#ifndef TARGET_PC
			char *tempNameWithoutPath = getNameWithoutPath(currentExec);
			char *tempPathWithoutName = getRomPath(currentExec);
			#else
			char *tempNameWithoutPath = malloc(strlen(currentExec)+1);
			strcpy(tempNameWithoutPath, currentExec);
			char *tempPathWithoutName = "\0";
			#endif

			menuSections[menuSectionCounter].executables[execCounter]=malloc(strlen(tempNameWithoutPath)+1);
			strcpy(menuSections[menuSectionCounter].executables[execCounter],tempNameWithoutPath);
			free(tempNameWithoutPath);

			menuSections[menuSectionCounter].emulatorDirectories[execCounter]=malloc(strlen(tempPathWithoutName)+2);
			strcpy(menuSections[menuSectionCounter].emulatorDirectories[execCounter],tempPathWithoutName);
			strcat(menuSections[menuSectionCounter].emulatorDirectories[execCounter],"/");

			currentExec = strtok(NULL,",");
			execCounter++;
		}
		logMessage("INFO","loadSections","Current exec ready");
		free(value2);
		logMessage("INFO","loadSections","free as a bird");
		for (int i=execCounter;i<10;i++) {
			menuSections[menuSectionCounter].executables[i]=NULL;
			menuSections[menuSectionCounter].emulatorDirectories[i]=NULL;
		}
		logMessage("INFO","loadSections","free as a bird 2");
		menuSections[menuSectionCounter].activeExecutable=0;
		logMessage("INFO","loadSections","Executable set");
		setStringValueInSection (config, sectionName, "romDirs", menuSections[menuSectionCounter].filesDirectories,"\0");
		setStringValueInSection (config, sectionName, "romExts", menuSections[menuSectionCounter].fileExtensions,"\0");
		#ifdef TARGET_RFW
		setStringValueInSection (config, sectionName, "scaling", menuSections[menuSectionCounter].scaling,"3");
		#else
		setStringValueInSection (config, sectionName, "scaling", menuSections[menuSectionCounter].scaling,"0");
		#endif
		setRGBColorInSection(themeConfig, sectionName, "fullscreen_menu_background_color", menuSections[menuSectionCounter].fullScreenMenuBackgroundColor);
		setRGBColorInSection(themeConfig, sectionName, "fullscreen_menu_font_color", menuSections[menuSectionCounter].fullscreenMenuItemsColor);
		setRGBColorInSection(themeConfig, sectionName, "items_font_color", menuSections[menuSectionCounter].menuItemsFontColor);
		setRGBColorInSection(themeConfig, sectionName, "selected_item_background_color", menuSections[menuSectionCounter].bodySelectedTextBackgroundColor);
		setRGBColorInSection(themeConfig, sectionName, "selected_item_font_color", menuSections[menuSectionCounter].bodySelectedTextTextColor);

		value = ini_get(themeConfig, "DEFAULT", "art_font_color");
		if (value!=NULL) {
			setRGBColorInSection(themeConfig, menuSections[menuSectionCounter].sectionName, "art_font_color", menuSections[menuSectionCounter].pictureTextColor);
		} else {
			setRGBColorInSection(themeConfig, menuSections[menuSectionCounter].sectionName, "items_font_color", menuSections[menuSectionCounter].pictureTextColor);
		}

		if (menuSections[menuSectionCounter].systemLogoSurface!=NULL) {
			SDL_FreeSurface(menuSections[menuSectionCounter].systemLogoSurface);
			menuSections[menuSectionCounter].systemLogoSurface = NULL;
			SDL_FreeSurface(menuSections[menuSectionCounter].systemPictureSurface);
			menuSections[menuSectionCounter].systemPictureSurface = NULL;
			SDL_FreeSurface(menuSections[menuSectionCounter].backgroundSurface);
			menuSections[menuSectionCounter].backgroundSurface = NULL;
		}
		logMessage("INFO","loadSections","Setting logo, background and system");
		setThemeResourceValueInSection (themeConfig, sectionName, "logo", menuSections[menuSectionCounter].systemLogo);
		setThemeResourceValueInSection (themeConfig, sectionName, "no_art_picture", menuSections[menuSectionCounter].noArtPicture);
		setThemeResourceValueInSection (themeConfig, sectionName, "background", menuSections[menuSectionCounter].background);
		setThemeResourceValueInSection (themeConfig, sectionName, "system", menuSections[menuSectionCounter].systemPicture);

		value = ini_get(themeConfig, "GENERAL", "system_w");
		systemWidth = atoifgl(value);

		value = ini_get(themeConfig, "GENERAL", "system_h");
		systemHeight = atoifgl(value);

		if (menuSectionCounter==currentSectionNumber) {
			logMessage("INFO","loadSections","Loading system logo");
			menuSections[menuSectionCounter].systemLogoSurface = IMG_Load(menuSections[menuSectionCounter].systemLogo);
			logMessage("INFO","loadSections","Loading system background");
			menuSections[menuSectionCounter].backgroundSurface = IMG_Load(menuSections[menuSectionCounter].background);
			logMessage("INFO","loadSections","Loading system picture");
			menuSections[menuSectionCounter].systemPictureSurface = IMG_Load(menuSections[menuSectionCounter].systemPicture);
		}

		logMessage("INFO","loadSections","Set");

		value = ini_get(config, sectionName, "aliasFile");
		if(value!=NULL) {
			strcpy(menuSections[menuSectionCounter].aliasFileName,value);
		} else {
			strcpy(menuSections[menuSectionCounter].aliasFileName,"\0");
		}

		setStringValueInSection (config, sectionName, "category", menuSections[menuSectionCounter].category,"\0");

		value = ini_get(config, sectionName, "onlyFileNamesNoPathOrExtension");
		if(value!=NULL&&strcmp("yes",value)==0) {
			menuSections[menuSectionCounter].onlyFileNamesNoExtension=1;
		}

		value = ini_get(themeConfig, menuSections[menuSectionCounter].sectionName, "name");

		if (value!=NULL) {
			strcpy(menuSections[menuSectionCounter].fantasyName, value);
		} else {
			strcpy(menuSections[menuSectionCounter].fantasyName, "\0");
		}

		menuSections[menuSectionCounter].currentPage=0;
		menuSections[menuSectionCounter].currentGameInPage=0;
		menuSectionCounter++;
		free(sectionName);
		logMessage("INFO","loadSections","Out");
	}
	value = ini_get(themeConfig, "GENERAL", "colorful_fullscreen_menu");
	if (value == NULL) {
		colorfulFullscreenMenu = 0;
	} else {
		colorfulFullscreenMenu = atoifgl(value);
	}

	value = ini_get(themeConfig, "GENERAL", "font_outline");
	if (value == NULL) {
		fontOutline = 0;
	} else {
		fontOutline = atoifgl(value);
	}
	logMessage("INFO","loadSections","Out 1");
	value = ini_get(themeConfig, "GENERAL", "display_section_group_name");
	if (value == NULL) {
		displaySectionGroupName = 0;
	} else {
		displaySectionGroupName = atoifgl(value);
	}

	value = ini_get(themeConfig, "GENERAL", "show_art");
	if (value == NULL) {
		showArt = 1;
	} else {
		showArt = atoifgl(value);
	}

	value = ini_get(themeConfig, "GENERAL", "items");
	itemsPerPage = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "items_separation");
	itemsSeparation = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "items_in_fullscreen_mode");
	itemsPerPageFullscreen = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "game_list_alignment");
	gameListAlignment = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "game_list_x");
	gameListX = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "game_list_y");
	gameListY = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "game_list_w");
	gameListWidth = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "game_list_position_in_full");
	gameListPositionFullScreen = atoifgl(value);

	logMessage("INFO","loadSections","Out2");

	value = ini_get(themeConfig, "GENERAL", "art_max_w");
	artWidth = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "art_max_h");
	artHeight = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "art_x");
	artX = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "art_y");
	artY = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "system_x");
	systemX = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "system_y");
	systemY = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "font_size");
	fontSize = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "text1_font_size");
	text1FontSize = atoi (value);

	value = ini_get(themeConfig, "GENERAL", "newspaper_mode");
	if (value==NULL) {
		newspaperMode = 0;
	} else {
		newspaperMode = atoi (value);
	}

	value = ini_get(themeConfig, "GENERAL", "text1_x");
	text1X = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "text1_y");
	text1Y = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "text1_alignment");
	text1Alignment = atoifgl(value);

	logMessage("INFO","loadSections","Out 3");

	setThemeResourceValueInSection (themeConfig, "GENERAL", "textX_font", textXFont);

	battX = -1;
	value = ini_get(themeConfig, "GENERAL", "batt_x");
	if(value!=NULL) {
		battX = atoifgl(value);
		value = ini_get(themeConfig, "GENERAL", "batt_y");
		battY = atoifgl(value);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_1", batt1);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_2", batt2);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_3", batt3);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_4", batt4);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_5", batt5);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_6", batt6);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_7", batt7);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_8", batt8);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_9", batt9);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_10", batt10);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_11", batt11);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_12", batt12);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_13", batt13);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_14", batt14);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_15", batt15);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_16", batt16);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_17", batt17);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_18", batt18);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_19", batt19);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_20", batt20);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "batt_charging", battCharging);
		surfaceBatt1 = IMG_Load(batt1);
		surfaceBatt2 = IMG_Load(batt2);
		surfaceBatt3 = IMG_Load(batt3);
		surfaceBatt4 = IMG_Load(batt4);
		surfaceBatt5 = IMG_Load(batt5);
		surfaceBatt6 = IMG_Load(batt6);
		surfaceBatt7 = IMG_Load(batt7);
		surfaceBatt8 = IMG_Load(batt8);
		surfaceBatt9 = IMG_Load(batt9);
		surfaceBatt10 = IMG_Load(batt10);
		surfaceBatt11 = IMG_Load(batt11);
		surfaceBatt12 = IMG_Load(batt12);
		surfaceBatt13 = IMG_Load(batt13);
		surfaceBatt14 = IMG_Load(batt14);
		surfaceBatt15 = IMG_Load(batt15);
		surfaceBatt16 = IMG_Load(batt16);
		surfaceBatt17 = IMG_Load(batt17);
		surfaceBatt18 = IMG_Load(batt18);
		surfaceBatt19 = IMG_Load(batt19);
		surfaceBatt20 = IMG_Load(batt20);
		surfaceBattCharging = IMG_Load(battCharging);
	}
	
	wifiX = -1;
	value = ini_get(themeConfig, "GENERAL", "wifi_x");
	if(value!=NULL) {
		wifiX = atoifgl(value);
		value = ini_get(themeConfig, "GENERAL", "wifi_y");
		wifiY = atoifgl(value);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "wifioff", wifioff);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "wifion", wifion);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "nowifi", nowifi);
		surfaceWifiOff = IMG_Load(wifioff);
		surfaceWifiOn = IMG_Load(wifion);
		surfaceNoWifi = IMG_Load(nowifi);
	}

	value = ini_get(themeConfig, "GENERAL", "display_game_count");
	displayGameCount=0;
	if (value!=NULL && atoi(value)==1) {
		displayGameCount=1;
		setThemeResourceValueInSection (themeConfig, "GENERAL", "game_count_font", gameCountFont);
		strcpy (gameCountText, "# Games Available");
		value = ini_get(themeConfig, "GENERAL", "game_count_text");
		if(value!=NULL) {
			strcpy (gameCountText, value);
		}
		char *temp = replaceWord(gameCountText, "#", "%d");
		strcpy (gameCountText, temp);
		free(temp);
		value = ini_get(themeConfig, "GENERAL", "game_count_font_size");
		gameCountFontSize = atoifgl(value);
		value = ini_get(themeConfig, "GENERAL", "game_count_x");
		gameCountX = atoifgl(value);
		value = ini_get(themeConfig, "GENERAL", "game_count_y");
		gameCountY = atoifgl(value);
		value = ini_get(themeConfig, "GENERAL", "game_count_alignment");
		gameCountAlignment = atoifgl(value);
		value = ini_get(themeConfig, "GENERAL", "game_count_font_color");
		setRGBFromHex(gameCountFontColor, value);
	}

	value = ini_get(themeConfig, "GENERAL", "newspaper_mode");
	if (value==NULL) {
		newspaperMode = 0;
	} else {
		newspaperMode = atoi (value);
	}

	value = ini_get(themeConfig, "GENERAL", "text2_font_size");

	text2FontSize = atoi (value);

	value = ini_get(themeConfig, "GENERAL", "text2_x");
	text2X = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "text2_y");
	text2Y = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "text2_alignment");
	text2Alignment = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "art_text_distance_from_picture");
	artTextDistanceFromPicture = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "art_text_line_separation");
	artTextLineSeparation = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "art_text_font_size");
	artTextFontSize = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "font_size");
	baseFont = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "transparent_shading");
	transparentShading  = atoifgl(value);

	value = ini_get(themeConfig, "GENERAL", "fullscreen_footer_on_top");
	footerOnTop = atoifgl(value);

	logMessage("INFO","loadSections","Out 4");

	setThemeResourceValueInSection (themeConfig, "GENERAL", "favorite_indicator", favoriteIndicator);

	setThemeResourceValueInSection (themeConfig, "GENERAL", "font", menuFont);
	setThemeResourceValueInSection (themeConfig, "GENERAL", "section_groups_folder", sectionGroupsFolder);

	freeFonts();
	initializeFonts();

	logMessage("INFO","loadSections","Out 5");

	strcpy(menuSections[menuSectionCounter].sectionName,"FAVORITES");
	value = ini_get(themeConfig, menuSections[menuSectionCounter].sectionName, "name");

	if (value!=NULL) {
		strcpy(menuSections[menuSectionCounter].fantasyName, value);
	} else {
		strcpy(menuSections[menuSectionCounter].fantasyName, "\0");
	}

	menuSections[menuSectionCounter].emulatorDirectories[0]=strdup("/some/folder/");
	menuSections[menuSectionCounter].activeEmulatorDirectory=0;
	menuSections[menuSectionCounter].executables[0]=NULL;
	menuSections[menuSectionCounter].activeExecutable=0;
	strcpy(menuSections[menuSectionCounter].filesDirectories,"/some/folder");
	strcpy(menuSections[menuSectionCounter].fileExtensions,".zzz");
	strcpy(menuSections[menuSectionCounter].category, "all");
	menuSections[menuSectionCounter].onlyFileNamesNoExtension=0;
	menuSections[menuSectionCounter].hidden=0;
	menuSections[menuSectionCounter].currentPage=0;
	menuSections[menuSectionCounter].currentGameInPage=0;
	setRGBColorInSection(themeConfig, "FAVORITES", "fullscreen_menu_background_color", menuSections[menuSectionCounter].fullScreenMenuBackgroundColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "fullscreen_menu_font_color", menuSections[menuSectionCounter].fullscreenMenuItemsColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "items_font_color", menuSections[menuSectionCounter].menuItemsFontColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "selected_item_background_color", menuSections[menuSectionCounter].bodySelectedTextBackgroundColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "selected_item_font_color", menuSections[menuSectionCounter].bodySelectedTextTextColor);
	value = ini_get(themeConfig, "DEFAULT", "art_font_color");
	if (value!=NULL) {
		setRGBColorInSection(themeConfig, menuSections[menuSectionCounter].sectionName, "art_font_color", menuSections[menuSectionCounter].pictureTextColor);
	} else {
		setRGBColorInSection(themeConfig, menuSections[menuSectionCounter].sectionName, "items_font_color", menuSections[menuSectionCounter].pictureTextColor);
	}

	logMessage("INFO","loadSections","Out 6");

	setThemeResourceValueInSection (themeConfig, "FAVORITES", "logo", menuSections[menuSectionCounter].systemLogo);
	setThemeResourceValueInSection (themeConfig, "FAVORITES", "system", menuSections[menuSectionCounter].systemPicture);
	setThemeResourceValueInSection (themeConfig, "FAVORITES", "no_art_picture", menuSections[menuSectionCounter].noArtPicture);
	setThemeResourceValueInSection (themeConfig, "FAVORITES", "background", menuSections[menuSectionCounter].background);

	menuSections[menuSectionCounter].systemLogoSurface = IMG_Load(menuSections[menuSectionCounter].systemLogo);
	menuSections[menuSectionCounter].backgroundSurface = IMG_Load(menuSections[menuSectionCounter].background);
	menuSections[menuSectionCounter].systemPictureSurface = IMG_Load(menuSections[menuSectionCounter].systemPicture);

	logMessage("INFO","loadSections","Out 7");

	menuSectionCounter++;
	favoritesSectionNumber=menuSectionCounter-1;
	ini_free(config);
	ini_free(themeConfig);
	return menuSectionCounter;
}

int countSections(char *file) {
	char pathToSectionsFilePlusFileName[1000];
	snprintf(pathToSectionsFilePlusFileName,sizeof(pathToSectionsFilePlusFileName),"%s",file);
	ini_t *config = ini_load(pathToSectionsFilePlusFileName);

	const char *consoles = ini_get(config, "CONSOLES", "consoleList");
	char *consoles1 = strdup(consoles);
	const char *value;
	int sectionCounter=0;

	char* tokenizedSectionName=strtok(consoles1,",");

	while(tokenizedSectionName!=NULL) {
		value = ini_get(config, tokenizedSectionName, "execs");
		if(value!=NULL) {
			sectionCounter++;
		}
		tokenizedSectionName=strtok(NULL,",");
	}
	free(consoles1);
	ini_free(config);
	return sectionCounter;
}

void saveLastState() {
	FILE * fp;
	char pathToStatesFilePlusFileName[300];
	snprintf(pathToStatesFilePlusFileName,sizeof(pathToStatesFilePlusFileName),"%s/.simplemenu/last_state.sav",home);
	fp = fopen(pathToStatesFilePlusFileName, "w");
	fprintf(fp, "%d;\n", 110);
	fprintf(fp, "%d;\n", stripGames);
	fprintf(fp, "%d;\n", fullscreenMode);
	fprintf(fp, "%d;\n", footerVisibleInFullscreenMode);
	fprintf(fp, "%d;\n", menuVisibleInFullscreenMode);
	fprintf(fp, "%d;\n", activeTheme);
	fprintf(fp, "%d;\n", timeoutValue);
	fprintf(fp, "%d;\n", activeGroup);
	fprintf(fp, "%d;\n", currentSectionNumber);
	fprintf(fp, "%d;\n", currentMode);
	#if defined MIYOOMINI
    #else
	fprintf(fp, "%d;\n", OCValue);
	fprintf(fp, "%d;\n", sharpnessValue);
	#endif
	for(int groupCount=0;groupCount<sectionGroupCounter;groupCount++) {
		int sectionsNum=countSections(sectionGroups[groupCount].groupPath);
		for (int sectionCount=0;sectionCount<=sectionsNum;sectionCount++) {
			if (groupCount==activeGroup) {
				int isActive = 0;
				if (sectionCount==currentSectionNumber) {
					isActive=1;
				}
				fprintf(fp, "%d;%d;%d;%d;%d;%d\n", isActive, sectionCount, menuSections[sectionCount].currentPage, menuSections[sectionCount].currentGameInPage, menuSections[sectionCount].realCurrentGameNumber, returnTo);
			} else {
				fprintf(fp, "%d;%d;%d;%d;%d;%d\n", sectionGroupStates[groupCount][sectionCount][0], sectionCount, sectionGroupStates[groupCount][sectionCount][1], sectionGroupStates[groupCount][sectionCount][2], sectionGroupStates[groupCount][sectionCount][3], returnTo);
			}
		}
	}
	fclose(fp);
}

void loadLastState() {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char pathToStatesFilePlusFileName[300];
	snprintf(pathToStatesFilePlusFileName,sizeof(pathToStatesFilePlusFileName),"%s/.simplemenu/last_state.sav",home);

	fp = fopen(pathToStatesFilePlusFileName, "r");
	if (fp==NULL) {
		saveLastState();
		return;
	}
	char *configurations[7];
	char *ptr;
	int startInSection = -1;
	int startInPictureMode = -1;
	int stripGamesConfig = -1;
	int startInGroup= -1;
	int footerVisible= -1;
	int menuVisible= -1;
	int themeRead= -1;
	int timeout= -1;
	int groupCounter=-1;
	int savedVersion=-1;
	int itemsRead=-1;
	#if defined MIYOOMINI
    #else
	int OCValueRead=-1;
	int sharpnessValueRead=-1;
    #endif
	while ((read = getline(&line, &len, fp)) != -1) {
		ptr = strtok(line, ";");
		int i=0;
		while(ptr != NULL) {
			configurations[i]=ptr;
			ptr = strtok(NULL, ";");
			i++;
		}
		if (savedVersion==-1) {
			savedVersion=atoifgl(configurations[0]);
			if(savedVersion!=110) {
				saveLastState();
				fclose(fp);
				if (line) {
					free(line);
				}
				return;
			}
		} else if (stripGamesConfig==-1) {
			stripGamesConfig=atoifgl(configurations[0]);
		} else if (startInPictureMode==-1){
			startInPictureMode=atoifgl(configurations[0]);
		} else if(footerVisible==-1) {
			footerVisible=atoifgl(configurations[0]);
		} else if(menuVisible==-1) {
			menuVisible=atoifgl(configurations[0]);
		} else if(themeRead==-1) {
			themeRead=atoifgl(configurations[0]);
		} else if(timeout==-1) {
			timeout=atoifgl(configurations[0]);
		} else if (startInGroup==-1) {
			startInGroup = atoifgl(configurations[0]);
		} else if (startInSection==-1) {
			startInSection=atoifgl(configurations[0]);
		} else if (itemsRead==-1) {
			itemsRead=atoifgl(configurations[0]);
		#if defined MIYOOMINI
        #else
		} else if (OCValueRead==-1) {
			OCValueRead=atoifgl(configurations[0]);
		} else if (sharpnessValueRead==-1) {
			sharpnessValueRead=atoifgl(configurations[0]);
		#endif
		} else {
			if(atoifgl(configurations[1])==0) {
				groupCounter++;
			}
			int isActive =atoifgl(configurations[0]);
			int sectionNumber =atoifgl(configurations[1]);
			int page = atoifgl(configurations[2]);
			int game = atoifgl(configurations[3]);
			int realCurrentGameNumber = atoifgl(configurations[4]);
			int retTo = atoifgl(configurations[5]);
			sectionGroupStates[groupCounter][sectionNumber][0]=isActive;
			sectionGroupStates[groupCounter][sectionNumber][1]=page;
			sectionGroupStates[groupCounter][sectionNumber][2]=game;
			sectionGroupStates[groupCounter][sectionNumber][3]=realCurrentGameNumber;
			sectionGroupStates[groupCounter][sectionNumber][4]=retTo;
			if (groupCounter==startInGroup) {
				menuSections[sectionNumber].currentPage=page;
				menuSections[sectionNumber].currentGameInPage=game;
				menuSections[sectionNumber].realCurrentGameNumber=realCurrentGameNumber;
				returnTo=retTo;
			}
			menuSections[sectionNumber].alphabeticalPaging=0;
		}
	}
	stripGames=stripGamesConfig;
	fullscreenMode=startInPictureMode;
	footerVisibleInFullscreenMode=footerVisible;
	menuVisibleInFullscreenMode=menuVisible;
	activeTheme=themeRead;
	timeoutValue=timeout;
	currentSectionNumber=startInSection;
	activeGroup = startInGroup;
	currentMode=itemsRead;
	#if defined MIYOOMINI
    #else
	OCValue=OCValueRead;
	sharpnessValue=sharpnessValueRead;
	#endif
	fclose(fp);
	if (line) {
		free(line);
	}
	logMessage("INFO","loadLastState","Last state loaded");
}
