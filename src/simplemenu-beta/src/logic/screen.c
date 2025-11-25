#define _GNU_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h>

#include "../headers/constants.h"
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/graphics.h"
#include "../headers/control.h"
#include "../headers/screen.h"
#include "../headers/input.h"
#include "../headers/logic.h"
#include "../headers/string_utils.h"
#include "../headers/doubly_linked_rom_list.h"
#include "../headers/utils.h"
#include "../headers/system_logic.h"

// SDL
extern SDL_Surface* IMG_Load(const char *file);

TTF_Font *font = NULL;
TTF_Font *miniFont = NULL;
TTF_Font *searchFont = NULL;
TTF_Font *picModeFont = NULL;
TTF_Font *BIGFont = NULL;
TTF_Font *headerFont = NULL;
TTF_Font *inMenuGameCountFont = NULL;
TTF_Font *footerFont = NULL;
TTF_Font *sectionCardsGameCountFont = NULL;

TTF_Font *outlineFont = NULL;
TTF_Font *outlineMiniFont = NULL;
TTF_Font *outlineHeaderFont = NULL;
TTF_Font *outlineCustomCountFont = NULL;
TTF_Font *outlineFooterFont = NULL;

TTF_Font *settingsfont = NULL;
TTF_Font *settingsHeaderFont = NULL;
TTF_Font *settingsFooterFont = NULL;
TTF_Font *settingsStatusFont = NULL;

TTF_Font *customHeaderFont = NULL;
TTF_Font *outlineCustomHeaderFont = NULL;

char *options[10];
char *values[10];
char *hints[10];

int countDown;
int refreshName=0;
int refreshCounter=0;
typedef struct {
int sectionIndex;
int romIndex;
const char *displayName;
} SearchResult;

typedef struct {
        const char *name;
        const char *abbreviation;
} SystemAbbreviation;

static int ensureSectionInitialized(int sectionIndex);

typedef struct {
int sectionIndex;
int romIndex;
char *displayName;
} SearchIndexEntry;

typedef struct {
int sectionIndex;
int gameCount;
} SearchIndexSectionMeta;

static int searchKeyboardRow=0;
static int searchKeyboardCol=0;
static int searchSelectionIndex=0;
static int searchFocusOnResults=0;
static int searchPreviousState=BROWSING_GAME_LIST;
static int searchResultsCount=0;
static int searchTotalMatches=0;
static SearchResult searchResults[1024];
static char searchQuery[128];
static SearchIndexEntry *searchIndexEntries=NULL;
static int searchIndexCount=0;
static int searchIndexCapacity=0;
static SearchIndexSectionMeta searchIndexMeta[100];
static int searchIndexMetaCount=0;
static int searchIndexReady=0;
static char searchIndexPath[PATH_MAX];
static int searchSectionGameCountCache[100];
static int searchSectionCountCacheSize=0;
static void drawBoxWithBorder(int x, int y, int width, int height, int *fillColor, int *borderColor, int thickness);
static const char *getSectionAbbreviation(const char *sectionName, const char *fantasyName);
static const char *searchKeyboardRows[] = {
        "ABCDEFGHIJKL",
        "MNOPQRSTUVWX",
        "YZ0123456789",
        " -_./+!?@#%&",
        "=:;[](){}|<>"
};

void displayHeart(int x, int y) {
	if(hideHeartTimer!=NULL) {
		SDL_Surface *heart = loadImage(favoriteIndicator);
		if (heart!=NULL) {
			double wh = heart->w;
			double hh = heart->h;
			double ratioh = 0;  // Used for aspect ratio
			int smoothing = 0;
			ratioh = wh / hh;   // get ratio for scaling image
			hh = heart->h;
			if(hh!=heart->h) {
				smoothing = 0;
			}
			wh = hh*ratioh;
			smoothing = 0;
			drawImage(screen, heart, x-(wh/2), y-(hh/2), 0, 0, wh, hh, 0, smoothing);
		}
	}
}

void drawPictureTextOnScreen(char *buf) {
	if(!footerVisibleInFullscreenMode||!isPicModeMenuHidden) {
		return;
	}
	int h = 0;
	TTF_SizeUTF8(font, buf, NULL, &h);
	char *temp = malloc(strlen(buf)+2);
	if(currentSectionNumber!=favoritesSectionNumber) {
		#if defined MIYOOMINI
		strcpy(temp,buf);
		#else
		if (CURRENT_SECTION.currentGameNode->data->preferences.frequency == OC_OC_LOW||CURRENT_SECTION.currentGameNode->data->preferences.frequency == OC_OC_HIGH) {
			strcpy(temp,"+");
			strcat(temp,buf);
		} else {
			strcpy(temp,buf);
		}
		#endif
	} else {
		#if defined MIYOOMINI
		strcpy(temp,buf);
		#else
		if (favorites[CURRENT_GAME_NUMBER].frequency == OC_OC_LOW||favorites[CURRENT_GAME_NUMBER].frequency == OC_OC_HIGH) {
			strcpy(temp,"+");
			strcat(temp,buf);
		} else {
			strcpy(temp,buf);
		}
		#endif
	}
	if(!isFavoritesSectionSelected()) {
		if (colorfulFullscreenMenu) {
			drawTransparentRectangleToScreen(SCREEN_WIDTH, h+2, 0, footerOnTop?0:SCREEN_HEIGHT-(h+2), CURRENT_SECTION.fullScreenMenuBackgroundColor, 180);
			drawTransparentRectangleToScreen(SCREEN_WIDTH, h+2, 0, footerOnTop?0:SCREEN_HEIGHT-(h+2),(int[]){0,0,0}, 100);
			drawTextOnScreen(font, NULL, SCREEN_WIDTH/2, footerOnTop?2:SCREEN_HEIGHT-h, temp, CURRENT_SECTION.fullscreenMenuItemsColor, footerOnTop?VAlignBottom|HAlignCenter:VAlignBottom|HAlignCenter, (int[]){}, 0);
		} else {
			drawTransparentRectangleToScreen(SCREEN_WIDTH, h+2, 0, footerOnTop?0:SCREEN_HEIGHT-(h+2), (int[]){0,0,0}, 180);
			drawTextOnScreen(font, NULL, SCREEN_WIDTH/2, footerOnTop?2:SCREEN_HEIGHT-h, temp, (int[]){255,255,255}, footerOnTop?VAlignBottom|HAlignCenter:VAlignBottom|HAlignCenter, (int[]){}, 0);
		}
	} else {
		drawTransparentRectangleToScreen(SCREEN_WIDTH, h+2, 0, footerOnTop?0:SCREEN_HEIGHT-(h+2), (int[]){0,0,0}, 180);
		drawTextOnScreen(font, NULL, SCREEN_WIDTH/2, footerOnTop?2:SCREEN_HEIGHT-h, temp, (int[]){255,255,0}, footerOnTop?VAlignBottom|HAlignCenter:VAlignBottom|HAlignCenter, (int[]){}, 0);
	}
	free(temp);
}

void drawImgFallbackTextOnScreen(char *fallBackText) {
	if(!footerVisibleInFullscreenMode) {
		char *temp = malloc(strlen(fallBackText)+2);
		#if defined MIYOOMINI
		strcpy(temp,fallBackText);
		#else
		if (CURRENT_SECTION.currentGameNode->data->preferences.frequency == OC_OC_HIGH || CURRENT_SECTION.currentGameNode->data->preferences.frequency == OC_OC_LOW) {
			strcpy(temp,"+");
			strcat(temp,fallBackText);
		} else {
			strcpy(temp,fallBackText);
		}
		#endif
		drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2), 120, temp, CURRENT_SECTION.menuItemsFontColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
		free(temp);
	} else {
		drawPictureTextOnScreen(fallBackText);
	}
}

void drawTextOnSettingsHeaderLeftWithColor(char *text, int txtColor[]) {
	drawTextOnScreen(settingsHeaderFont, NULL, calculateProportionalSizeOrDistance1(5), calculateProportionalSizeOrDistance1(24), text, txtColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
}

void drawCurrentLetter(char *letter, int textColor[], int x, int y) {
	drawTextOnScreen(font, NULL, x, y, letter, textColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
}

void drawBigWhiteText(char *text) {
	int white[3]={253, 35, 39};
	drawTextOnScreen(BIGFont, NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, text, white, VAlignMiddle | HAlignCenter, (int[]){}, 0);
}

void drawLoadingText() {
	#ifndef NOLOADING
	int white[3]={253, 35, 39};
	drawTextOnScreen(settingsFooterFont, NULL, SCREEN_WIDTH-calculateProportionalSizeOrDistance1(44), SCREEN_HEIGHT-calculateProportionalSizeOrDistance1(8), "LOADING...", white, VAlignMiddle | HAlignCenter, (int[]){}, 0);
	refreshScreen();
	#endif
}

void drawCopyingText() {
	int white[3]={253, 35, 39};
	drawTextOnScreen(settingsFooterFont, NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, "COPYING FILES, PLEASE WAIT", white, VAlignMiddle | HAlignCenter, (int[]){}, 0);
}

void drawCurrentSectionGroup(char *groupName, int textColor[]) {
	drawTextOnScreen(BIGFont, NULL, (SCREEN_WIDTH/2)+calculateProportionalSizeOrDistance1(2), (SCREEN_HEIGHT/2), groupName, textColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
}

void drawGameNameUnderPicture(char *buf, int x, int y, int maxWidth) {
	genericDrawMultiLineTextOnScreen(miniFont, outlineMiniFont, x, y, buf, CURRENT_SECTION.pictureTextColor, VAlignBottom|HAlignCenter, maxWidth, artTextLineSeparation);
}

void drawGameNumber(char *buf, int x, int y) {
	int hAlign = HAlignLeft;
	if(text2Alignment==1) {
		hAlign = HAlignCenter;
	} else if (text2Alignment==2) {
		hAlign = HAlignRight;
	}
	drawTextOnScreen(inMenuGameCountFont, outlineCustomCountFont, x, y, buf, CURRENT_SECTION.fullscreenMenuItemsColor, VAlignMiddle|hAlign, CURRENT_SECTION.fullScreenMenuBackgroundColor, 0);
}

void drawSettingsOptionValueOnScreen(char *value, int position, int txtColor[]) {
	int retW3=3;
	TTF_SizeUTF8(settingsfont, (const char *) value, &retW3, NULL);
	drawTextOnScreen(settingsfont, NULL, SCREEN_WIDTH-5-retW3, position, value, txtColor, VAlignBottom | HAlignLeft, (int[]){}, 0);
}

void drawSettingsOptionOnScreen(char *buf, int position, int txtColor[]) {
	drawTextOnScreen(settingsfont, NULL, 5, position, buf, txtColor, VAlignBottom | HAlignLeft, (int[]){}, 0);
}

void drawScrolledShadedGameNameOnScreenCustom(char *buf, int position){
	static unsigned int temppos=0;
	static Uint32 initwait=0;

	int width=MAGIC_NUMBER;
	int retW = 1;
	TTF_SizeUTF8(font, (const char *) buf, &retW, NULL);

	// create the string
	unsigned int buflen=strlen(buf);
	char *temp;
	if(retW>width)		// if retW > width, we need scroll, the concatenate 2 strings to do the trick
		temp = malloc(buflen*2+5+2);
	else
		temp = malloc(buflen+2);
	
	// concatenate name+spaces+name
	strcpy(temp,buf);
	if(retW>width) {
		strcat(temp,"     ");
		strcat(temp,buf);
	}

	// set wait time before scroll
	if(refreshName==0) {
		temppos=0;
		initwait=SDL_GetTicks();
	}

	// do scroll
	if(retW>width) {				// if name is greater than gamelist width, then do scroll
		refreshName=1;
		if(SDL_GetTicks()-initwait>1000) {	// wait 1 second before scroll
			temppos++;			// character index of the name to display
			if(temppos>=buflen+5)
				temppos=0;
		}
	} else {
		refreshName=0;
		temppos=0;
	}
	
	int hAlign = 0;
	if (gameListAlignment==0) {
		hAlign = HAlignLeft;
	} else if (gameListAlignment==1) {
		hAlign = HAlignCenter;
	} else {
		hAlign = HAlignRight;
	}

	TTF_SizeUTF8(font, (const char *) &temp[temppos], &retW, NULL);
	if (transparentShading) {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, &temp[temppos], menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, (int[]){}, 0, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, &temp[temppos], menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, (int[]){}, 0);
		}
	} else {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, &temp[temppos], menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, menuSections[currentSectionNumber].bodySelectedTextBackgroundColor, 1, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, &temp[temppos], menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, menuSections[currentSectionNumber].bodySelectedTextBackgroundColor, 1);
		}
	}
	free(temp);
}

void drawShadedGameNameOnScreenCustom(char *buf, int position){
	char *temp = malloc(strlen(buf)+2);
	strcpy(temp,buf);
	int hAlign = 0;
	if (gameListAlignment==0) {
		hAlign = HAlignLeft;
	} else if (gameListAlignment==1) {
		hAlign = HAlignCenter;
	} else {
		hAlign = HAlignRight;
	}

	int retW = 1;
	int width=MAGIC_NUMBER;
	TTF_SizeUTF8(font, (const char *) buf, &retW, NULL);
	if (transparentShading) {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, temp, menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, (int[]){}, 0, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, temp, menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, (int[]){}, 0);
		}
	} else {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, temp, menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, menuSections[currentSectionNumber].bodySelectedTextBackgroundColor, 1, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, temp, menuSections[currentSectionNumber].bodySelectedTextTextColor, VAlignBottom | hAlign, menuSections[currentSectionNumber].bodySelectedTextBackgroundColor, 1);
		}
	}
	free(temp);
	refreshName=0;
}

void drawNonShadedGameNameOnScreenCustom(char *buf, int position) {
	int retW = 1;
	int width=MAGIC_NUMBER;
	TTF_SizeUTF8(font, (const char *) buf, &retW, NULL);
	if (gameListAlignment == 0) {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, buf, menuSections[currentSectionNumber].menuItemsFontColor, VAlignBottom | HAlignLeft, (int[]){}, 0, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, buf, menuSections[currentSectionNumber].menuItemsFontColor, VAlignBottom | HAlignLeft, (int[]){}, 0);
		}
	} else if (gameListAlignment == 1) {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, buf, menuSections[currentSectionNumber].menuItemsFontColor, VAlignBottom | HAlignCenter, (int[]){}, 0, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, buf, menuSections[currentSectionNumber].menuItemsFontColor, VAlignBottom | HAlignCenter, (int[]){}, 0);
		}
	} else {
		if (retW>width) {
			drawTextOnScreenMaxWidth(font, outlineFont, gameListX, position, buf, menuSections[currentSectionNumber].menuItemsFontColor, VAlignBottom | HAlignRight, (int[]){}, 0, retW);
		} else {
			drawTextOnScreen(font, outlineFont, gameListX, position, buf, menuSections[currentSectionNumber].menuItemsFontColor, VAlignBottom | HAlignRight, (int[]){}, 0);
		}
	}
}

void drawShadedGameNameOnScreenPicMode(char *buf, int position) {
	char *temp = malloc(strlen(buf)+2);
	strcpy(temp,buf);
	int color[3];
	color[0] = 255;
	color[1] = 255;
	color[2] = 0;
	drawTextOnScreen(font, outlineFont, 5, position, temp, color, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	free(temp);
}

void drawNonShadedGameNameOnScreenPicMode(char *buf, int position) {
	int color[3];
	if (colorfulFullscreenMenu) {
		color[0] = CURRENT_SECTION.fullscreenMenuItemsColor[0];
		color[1] = CURRENT_SECTION.fullscreenMenuItemsColor[1];
		color[2] = CURRENT_SECTION.fullscreenMenuItemsColor[2];
	} else {
		color[0] = 255;
		color[1] = 255;
		color[2] = 255;
	}
	drawTextOnScreen(font, outlineFont, 5, position, buf, color, VAlignMiddle | HAlignLeft, (int[]){}, 0);
}

void displayImageOnMenuScreen(char *fileName) {
	SDL_Surface *screenshot=NULL;

	if(showArt) {
		screenshot = loadImage(fileName);
		if(screenshot==NULL)
			screenshot = loadImage(menuSections[currentSectionNumber].noArtPicture);
	}

	if (screenshot!=NULL) {
		double w = screenshot->w;
		double h = screenshot->h;
		double ratio = 0;  // Used for aspect ratio
		int smoothing = 0;
		ratio = w / h;   // get ratio for scaling image
		h = artHeight;
		w = h*ratio;
		smoothing = 0;
		if (w!=artWidth) {
			ratio = h / w;   // get ratio for scaling image
			w = artWidth;
			h = w*ratio;
			if (h>artHeight) {
				ratio = w / h;   // get ratio for scaling image
				h = artHeight;
				w = h*ratio;
			}
			if(SCREEN_WIDTH==320) {
				smoothing=1;
			}
		}
		currentH = artY+h;
		currentW = artX+w;
//		smoothing=0;
		int heartX = 0;
		int heartY = 0;

		if (showArt) {
			if(newspaperMode) {
				heartX = artX+artWidth/2;
				heartY = artY+h/2;
			} else {
				heartX = artX+artWidth/2;
				heartY = artY+artHeight/2;
			}
		} else {
			heartX = SCREEN_WIDTH/2;
			heartY = SCREEN_HEIGHT/2;
		}
		if (showArt) {
			if(newspaperMode) {
				drawImage(screen, screenshot, artX+(artWidth/2)-w/2, artY, 0, 0, w, h, 0, smoothing);
			} else {
				drawImage(screen, screenshot, artX+(artWidth/2)-w/2, artY+(artHeight/2)-h/2, 0, 0, w, h, 0, smoothing);
			}
		}//proper rectangle
		displayHeart(heartX, heartY);
		if(artTextDistanceFromPicture>=0 && showArt && !newspaperMode) {
			char temp[500];
			snprintf(temp,sizeof(temp),"%d/%d", CURRENT_SECTION.realCurrentGameNumber, CURRENT_SECTION.gameCount);
			if (currentGameNameBeingDisplayed[0]=='+') {
				drawGameNameUnderPicture(currentGameNameBeingDisplayed+1, artX+artWidth/2, artY+artHeight+artTextDistanceFromPicture,artWidth);
			} else {
				drawGameNameUnderPicture(currentGameNameBeingDisplayed, artX+artWidth/2, artY+artHeight+artTextDistanceFromPicture,artWidth);
			}
		}
	} else {
		int heartX = 0;
		int heartY = 0;
		if (showArt) {
			heartX = artX+artWidth/2;
			heartY = artY+(((artWidth/4)*3)/2);
		} else {
			heartX = SCREEN_WIDTH/2;
			heartY = SCREEN_HEIGHT/2;
		}
		displayHeart(heartX, heartY);
		if(artTextDistanceFromPicture>=0 && showArt) {
			char temp[500];
			snprintf(temp,sizeof(temp),"%d/%d", CURRENT_SECTION.realCurrentGameNumber, CURRENT_SECTION.gameCount);
			int artHeight = (artWidth/4)*3;
			if (CURRENT_SECTION.gameCount>0) {
				if(!newspaperMode) {
					if (currentGameNameBeingDisplayed[0]=='+') {
						drawGameNameUnderPicture(currentGameNameBeingDisplayed+1, artX+artWidth/2, artY+artHeight+artTextDistanceFromPicture,artWidth);
					} else {
						drawGameNameUnderPicture(currentGameNameBeingDisplayed, artX+artWidth/2, artY+artHeight+artTextDistanceFromPicture,artWidth);
					}
				}
			}
		}
	}
	if(systemX>0&&systemY>0) {
		displaySurface(CURRENT_SECTION.systemPictureSurface,systemX, systemY);
	}
}

void drawTextOnSettingsFooterWithColor(char *text, int txtColor[]) {
	drawTextOnScreen(settingsStatusFont, NULL, calculateProportionalSizeOrDistance1(5), calculateProportionalSizeOrDistance1(231), text, txtColor, VAlignMiddle| HAlignLeft, (int[]){}, 0);
}

void drawTextOnHeader() {
	int Halign = 0;
	switch (text1Alignment) {
	case 0:
		Halign = HAlignLeft;
		break;
	case 1:
		Halign = HAlignCenter;
		break;
	case 2:
		Halign = HAlignRight;
		break;
	}
	if (currentSectionNumber==favoritesSectionNumber) {
		if(favoritesSize>0) {
			if (favorites[CURRENT_GAME_NUMBER].sectionAlias[0]!=' ') {
				drawTextOnScreen(customHeaderFont, outlineCustomHeaderFont, text1X, text1Y, favorites[CURRENT_GAME_NUMBER].sectionAlias, menuSections[currentSectionNumber].fullscreenMenuItemsColor, VAlignMiddle | Halign, CURRENT_SECTION.fullScreenMenuBackgroundColor, 0);
			} else {
				drawTextOnScreen(customHeaderFont, outlineCustomHeaderFont, text1X, text1Y, favorites[CURRENT_GAME_NUMBER].section, menuSections[currentSectionNumber].fullscreenMenuItemsColor, VAlignMiddle | Halign, CURRENT_SECTION.fullScreenMenuBackgroundColor, 0);
			}
		} else {
			if (menuSections[favoritesSectionNumber].fantasyName[0]!='\0') {
				drawTextOnScreen(customHeaderFont, outlineCustomHeaderFont, text1X, text1Y, menuSections[favoritesSectionNumber].fantasyName, menuSections[currentSectionNumber].fullscreenMenuItemsColor, VAlignMiddle | Halign, CURRENT_SECTION.fullScreenMenuBackgroundColor, 0);
			} else {
				drawTextOnScreen(customHeaderFont, outlineCustomHeaderFont, text1X, text1Y, menuSections[favoritesSectionNumber].sectionName, menuSections[currentSectionNumber].fullscreenMenuItemsColor, VAlignMiddle | Halign, CURRENT_SECTION.fullScreenMenuBackgroundColor, 0);
			}
		}
	} else {
		drawTextOnScreen(customHeaderFont, outlineCustomHeaderFont, text1X, text1Y, strlen(menuSections[currentSectionNumber].fantasyName)>1?menuSections[currentSectionNumber].fantasyName:menuSections[currentSectionNumber].sectionName, menuSections[currentSectionNumber].fullscreenMenuItemsColor, VAlignMiddle | Halign, CURRENT_SECTION.fullScreenMenuBackgroundColor, 0);
	}
}

void initializeSettingsFonts() {
	logMessage("INFO","initializeSettingsFonts","Initializing Settings Fonts");
	char *akashi = "resources/akashi.ttf";
	settingsfont = TTF_OpenFont(akashi, calculateProportionalSizeOrDistance1(14));
	settingsHeaderFont = TTF_OpenFont(akashi, calculateProportionalSizeOrDistance1(27));
	settingsStatusFont = TTF_OpenFont(akashi, calculateProportionalSizeOrDistance1(14));
	settingsFooterFont = TTF_OpenFont(akashi, calculateProportionalSizeOrDistance1(15));
	logMessage("INFO","initializeSettingsFonts","Settings Fonts initialized");
}

void initializeFonts() {
	TTF_Init();
	char *akashi = "resources/akashi.ttf";

	font = TTF_OpenFont(menuFont, fontSize);
	outlineFont = TTF_OpenFont(menuFont, fontSize);

        int searchFontSize = artTextFontSize - calculateProportionalSizeOrDistance1(5);
        if (searchFontSize < calculateProportionalSizeOrDistance1(7)) {
                searchFontSize = calculateProportionalSizeOrDistance1(7);
        }
        miniFont = TTF_OpenFont(menuFont, artTextFontSize);
        outlineMiniFont = TTF_OpenFont(menuFont, artTextFontSize);
        searchFont = TTF_OpenFont(menuFont, searchFontSize);

	picModeFont = TTF_OpenFont(menuFont, fontSize+calculateProportionalSizeOrDistance1(5));
	BIGFont = TTF_OpenFont(akashi, calculateProportionalSizeOrDistance1(16)+calculateProportionalSizeOrDistance1(17));
	headerFont = TTF_OpenFont(menuFont, fontSize+calculateProportionalSizeOrDistance1(6));
	outlineHeaderFont = TTF_OpenFont(menuFont, fontSize+calculateProportionalSizeOrDistance1(6));

	footerFont = TTF_OpenFont(menuFont, fontSize+calculateProportionalSizeOrDistance1(2));
	outlineFooterFont = TTF_OpenFont(menuFont, fontSize+calculateProportionalSizeOrDistance1(2));

	customHeaderFont = TTF_OpenFont(textXFont, text1FontSize);
	outlineCustomHeaderFont = TTF_OpenFont(textXFont, text1FontSize);

	inMenuGameCountFont = TTF_OpenFont(textXFont, text2FontSize);
	outlineCustomCountFont = TTF_OpenFont(textXFont, text2FontSize);
        if (menuFont!=NULL && strlen(menuFont)>2) {
                TTF_SetFontOutline(outlineFont,fontOutline);
                TTF_SetFontOutline(outlineMiniFont,fontOutline);
                TTF_SetFontOutline(outlineHeaderFont,fontOutline);
                TTF_SetFontOutline(outlineFooterFont,fontOutline);
                TTF_SetFontOutline(outlineCustomHeaderFont,fontOutline);
                TTF_SetFontOutline(outlineCustomCountFont,fontOutline);
                TTF_SetFontOutline(searchFont,fontOutline);
        }

	sectionCardsGameCountFont = TTF_OpenFont(gameCountFont, gameCountFontSize);

	logMessage("INFO","initializeFonts","Fonts initialized");
}

void freeFonts() {
	TTF_CloseFont(font);
	font = NULL;
	TTF_CloseFont(outlineFont);
	outlineFont = NULL;
	TTF_CloseFont(headerFont);
	headerFont = NULL;
	TTF_CloseFont(customHeaderFont);
	customHeaderFont = NULL;
	TTF_CloseFont(inMenuGameCountFont);
	inMenuGameCountFont = NULL;
	TTF_CloseFont(footerFont);
	footerFont = NULL;
        TTF_CloseFont(picModeFont);
        picModeFont = NULL;
        TTF_CloseFont(miniFont);
        miniFont = NULL;
        TTF_CloseFont(searchFont);
        searchFont = NULL;
        TTF_CloseFont(BIGFont);
	BIGFont = NULL;
	TTF_CloseFont(outlineCustomHeaderFont);
	outlineCustomHeaderFont = NULL;
	TTF_CloseFont(outlineMiniFont);
	outlineMiniFont = NULL;
	TTF_CloseFont(outlineHeaderFont);
	outlineHeaderFont = NULL;
	TTF_CloseFont(outlineCustomCountFont);
	outlineCustomCountFont = NULL;
	TTF_CloseFont(outlineFooterFont);
	outlineFooterFont = NULL;
	TTF_CloseFont(sectionCardsGameCountFont);
	sectionCardsGameCountFont = NULL;
}

void freeSettingsFonts() {
	TTF_CloseFont(settingsfont);
	settingsfont = NULL;
	TTF_CloseFont(settingsHeaderFont);
	settingsHeaderFont = NULL;
	TTF_CloseFont(settingsFooterFont);
	settingsFooterFont = NULL;
	TTF_CloseFont(settingsStatusFont);
	settingsStatusFont = NULL;
}

void resizeSectionSystemLogo(struct MenuSection *section) {
	section->systemLogoSurface = resizeSurfaceToScreenSize(section->systemLogoSurface);
}

void resizeSectionBackground(struct MenuSection *section) {
	section->backgroundSurface = resizeSurfaceToScreenSize(section->backgroundSurface);
}

void resizeGroupBackground(struct SectionGroup *group) {
	group->groupBackgroundSurface = resizeSurfaceToScreenSize(group->groupBackgroundSurface);
}

void resizeSectionSystemPicture(struct MenuSection *section) {
	section->systemPictureSurface = resizeSurface(section->systemPictureSurface, systemWidth, systemHeight);
}

void drawError(char *errorMessage, int textColor[]) {
	if(strchr(errorMessage,'-')==NULL) {
		drawTextOnScreen(settingsfont, NULL, (SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(3), errorMessage, textColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
	} else {
		char *line2 = strchr(errorMessage,'-');
		int index = (line2-errorMessage);
		line2++;
		char line1[200];
		strcpy(line1, errorMessage);
		line1[index]='\0';
		drawTextOnScreen(settingsfont, NULL, (SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(3)-calculateProportionalSizeOrDistance1(12), line1, textColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
		drawTextOnScreen(settingsfont, NULL, (SCREEN_WIDTH/2), (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(3)+calculateProportionalSizeOrDistance1(12), line2, textColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
	}
}

void displayBackgroundPicture() {
	if(fullscreenMode) {
		drawRectangleToScreen(SCREEN_WIDTH,SCREEN_HEIGHT,0,0,CURRENT_SECTION.bodyBackgroundColor);
	} else {
		drawRectangleToScreen(SCREEN_WIDTH,SCREEN_HEIGHT,0,0,CURRENT_SECTION.bodyBackgroundColor);
	}
}


void showErrorMessage(char *errorMessage) {
	int width = ((calculateProportionalSizeOrDistance1(strlen(errorMessage)))*(180)/calculateProportionalSizeOrDistance1(18));
	int height = calculateProportionalSizeOrDistance1(40);
	if(strchr(errorMessage,'-')!=NULL) {
		height = calculateProportionalSizeOrDistance1(60);
		width = ((calculateProportionalSizeOrDistance1(strlen(errorMessage))/2*calculateProportionalSizeOrDistance1(200))/calculateProportionalSizeOrDistance1(18))+calculateProportionalSizeOrDistance1(20);
	}
	int filling[3];
	int borderColor[3];
	borderColor[0]=255;
	borderColor[1]=0;
	borderColor[2]=0;
	filling[0]=100;
	filling[1]=0;
	filling[2]=0;
	int textColor[3]={255, 255, 255};
	drawRectangleToScreen(width+calculateProportionalSizeOrDistance1(10), height+calculateProportionalSizeOrDistance1(10), SCREEN_WIDTH/2-width/2-calculateProportionalSizeOrDistance1(5),SCREEN_HEIGHT/2-height/2-calculateProportionalSizeOrDistance1(5), borderColor);
	drawRectangleToScreen(width, height, SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2-height/2, filling);
	drawError(errorMessage, textColor);
	itsStoppedBecauseOfAnError=0;
}

int letterExistsInGameList(char *letter, char* letters) {
	if (strstr(letters,letter)!=NULL) {
		return 1;
	}
	return 0;
}

void showLetter(struct Rom *rom) {
	int rectangleHeight = 80;
	int rectangleX = (SCREEN_WIDTH/2);
	int rectangleY = (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(3);
	int filling[3];
	filling[0]=CURRENT_SECTION.fullScreenMenuBackgroundColor[0];
	filling[1]=CURRENT_SECTION.fullScreenMenuBackgroundColor[1];
	filling[2]=CURRENT_SECTION.fullScreenMenuBackgroundColor[2];
	int textColor[3] = {CURRENT_SECTION.fullscreenMenuItemsColor[0], CURRENT_SECTION.fullscreenMenuItemsColor[1], CURRENT_SECTION.fullscreenMenuItemsColor[2]};
	if (fullscreenMode) {
		filling[0] = 0;
		filling[1] = 0;
		filling[2] = 0;
		textColor[0]=255;
		textColor[1]=255;
		textColor[2]=255;
		rectangleHeight=calculateProportionalSizeOrDistance1(20);
		rectangleX = 0;
		rectangleY = calculateProportionalSizeOrDistance1(220);
	}
	filling[0] = CURRENT_SECTION.fullScreenMenuBackgroundColor[0];
	filling[1] = CURRENT_SECTION.fullScreenMenuBackgroundColor[1];
	filling[2] = CURRENT_SECTION.fullScreenMenuBackgroundColor[2];
	textColor[0]=CURRENT_SECTION.fullscreenMenuItemsColor[0];
	textColor[1]=CURRENT_SECTION.fullscreenMenuItemsColor[0];
	textColor[2]=CURRENT_SECTION.fullscreenMenuItemsColor[0];
	rectangleHeight=calculateProportionalSizeOrDistance1(19);
	rectangleX = 0;
	rectangleY = calculateProportionalSizeOrDistance1(222);
	if (!fullscreenMode) {
		drawRectangleToScreen(SCREEN_WIDTH, rectangleHeight, rectangleX, rectangleY, filling);
		drawTransparentRectangleToScreen(SCREEN_WIDTH, rectangleHeight, rectangleX, rectangleY, (int[]){0,0,0},255);
	} else {
		drawRectangleToScreen(SCREEN_WIDTH, rectangleHeight, rectangleX, rectangleY, (int[]){0,0,0});
	}
	char currentGameFirstLetter[2]="";
	char *currentGame = malloc(500);
	currentGame=getFileNameOrAlias(rom);
	currentGameFirstLetter[0]=toupper(currentGame[0]);
	currentGameFirstLetter[1]='\0';

	char *letters[] = {"#", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
	char *existingLetters = getCurrentSectionExistingLetters();

	if(isdigit(currentGameFirstLetter[0])||!isalpha(currentGameFirstLetter[0])) {
		currentGameFirstLetter[0]='#';
	}
	int x = 0;
	int y = calculateProportionalSizeOrDistance1(231);
	for (int i=0;i<27;i++) {
		if (!letterExistsInGameList(letters[i], existingLetters)) {
			textColor[0]=50;
			textColor[1]=0;
			textColor[2]=0;
		} else {
			textColor[0]=255;
			textColor[1]=255;
			textColor[2]=255;
		}
		if (strcmp(letters[i],currentGameFirstLetter)==0) {
			textColor[0]=253;
			textColor[1]=35;
			textColor[2]=39;
		}
		if (strcmp(letters[i],"N")==0) {
			x+=calculateProportionalSizeOrDistance1(14);
		} else if (strcmp(letters[i],"A")==0) {
			x+=calculateProportionalSizeOrDistance1(13);
		} else if (strcmp(letters[i],"B")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"C")==0) {
			x+=calculateProportionalSizeOrDistance1(11);
		} else if (strcmp(letters[i],"D")==0) {
			x+=calculateProportionalSizeOrDistance1(11);
		} else if (strcmp(letters[i],"F")==0) {
			x+=calculateProportionalSizeOrDistance1(10);
		} else if (strcmp(letters[i],"G")==0) {
			x+=calculateProportionalSizeOrDistance1(10);
		} else if (strcmp(letters[i],"H")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"I")==0) {
			x+=calculateProportionalSizeOrDistance1(10);
		} else if (strcmp(letters[i],"J")==0) {
			x+=calculateProportionalSizeOrDistance1(8);
		} else if (strcmp(letters[i],"K")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"M")==0) {
			x+=calculateProportionalSizeOrDistance1(13);
		} else if (strcmp(letters[i],"O")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"P")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"Q")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"R")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"S")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"V")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else if (strcmp(letters[i],"W")==0) {
			x+=calculateProportionalSizeOrDistance1(13);
		} else if (strcmp(letters[i],"X")==0) {
			x+=calculateProportionalSizeOrDistance1(14);
		} else if (strcmp(letters[i],"Y")==0) {
			x+=calculateProportionalSizeOrDistance1(11);
		} else if (strcmp(letters[i],"Z")==0) {
			x+=calculateProportionalSizeOrDistance1(12);
		} else {
			x+=calculateProportionalSizeOrDistance1(11);
		}
		if(!is43()&&x>calculateProportionalSizeOrDistance1(14)) {
			x+=calculateProportionalSizeOrDistance1(4);
		}
		drawCurrentLetter(letters[i], textColor, x, y);
	}
	free(existingLetters);
	free(currentGame);
}


void showCurrentGroup() {
	int backgroundColor[3];
	backgroundColor[0]=50;
	backgroundColor[1]=50;
	backgroundColor[2]=50;
	int textColor[3]= {255, 255, 255};
	char *tempString = malloc(strlen(sectionGroups[activeGroup].groupName)+1);
	strcpy(tempString,sectionGroups[activeGroup].groupName);
	strcat(tempString,"\0");

	drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, backgroundColor);

	if (sectionGroups[activeGroup].groupBackgroundSurface!=NULL) {
		displaySurface(sectionGroups[activeGroup].groupBackgroundSurface, 0, 0);
	}

	if (displaySectionGroupName || sectionGroups[activeGroup].groupBackgroundSurface==NULL) {
		drawTransparentRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(70), 0, SCREEN_HEIGHT/2-calculateProportionalSizeOrDistance1(38), (int[]){0,0,0}, 50);
		drawCurrentSectionGroup(tempString, textColor);
	}
	free(tempString);
	logMessage("INFO", "showCurrentGroup", "Group displayed");
}

void showRomPreferences() {
	int textColor[3];
	textColor[0]=253;
	textColor[1]=35;
	textColor[2]=39;
	int valueColor[3] = {255,255,255};
	int problematicGray[3] = {124,0,2};
	int textWidth;

	char *emuName = malloc(strlen(CURRENT_SECTION.executables[CURRENT_SECTION.currentGameNode->data->preferences.emulator])+1);
	strcpy(emuName,CURRENT_SECTION.executables[CURRENT_SECTION.currentGameNode->data->preferences.emulator]);
	strcat(emuName,"\0");
    
    #if defined MIYOOMINI
    #else
	char *frequency = malloc(10);
	snprintf(frequency, 10, "%d", CURRENT_SECTION.currentGameNode->data->preferences.frequency);
    #endif

	int width=calculateProportionalSizeOrDistance1(315);
	int height = calculateProportionalSizeOrDistance1(72);

	//Main rectangle
	drawRectangleToScreen(width+calculateProportionalSizeOrDistance1(4), height+calculateProportionalSizeOrDistance1(4), SCREEN_WIDTH/2-(width/2+calculateProportionalSizeOrDistance1(2)), SCREEN_HEIGHT/2-(height/2+calculateProportionalSizeOrDistance1(2)), (int[]) {50,0,0});

	//Overlay
	drawRectangleToScreen(width, height, SCREEN_WIDTH/2-width/2, SCREEN_HEIGHT/2-height/2, (int[]) {0,0,0});

	//Title Bar
	drawRectangleToScreen(width, height/4, SCREEN_WIDTH/2-width/2, SCREEN_HEIGHT/2-height/2, (int[]){253,35,39});

	//Selection
	if (chosenChoosingOption==0) {
		drawRectangleToScreen(width, height/4, SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2-height/4, problematicGray);
	} else if (chosenChoosingOption==1) {
		drawRectangleToScreen(width, height/4, SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2, problematicGray);
	} else {
		drawRectangleToScreen(width, height/4, SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2+height/4, problematicGray);
	}

	//Name
	char * name = getNameWithoutPath(CURRENT_SECTION.currentGameNode->data->name);
	drawTextOnScreen(font, NULL, calculateProportionalSizeOrDistance1(6), (SCREEN_HEIGHT/2)-calculateProportionalSizeOrDistance1(28), name, (int[]) {255,255,255}, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	free(name);

#if defined MIYOOMINI
	TTF_SizeUTF8(font, (const char *) "Autostart: " , &textWidth, NULL);
	textWidth+=calculateProportionalSizeOrDistance1(2);

    drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+calculateProportionalSizeOrDistance1(4), (SCREEN_HEIGHT/2)-calculateProportionalSizeOrDistance1(9), "Set options for the game", textColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
#else
	TTF_SizeUTF8(font, (const char *) "Overclock: " , &textWidth, NULL);
	textWidth+=calculateProportionalSizeOrDistance1(2);

	//Frequency option text
	drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+calculateProportionalSizeOrDistance1(4), (SCREEN_HEIGHT/2)-calculateProportionalSizeOrDistance1(9), "Overclock: ", textColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	//Frequency option value
#if defined TARGET_OD_BETA || defined TARGET_RFW || defined TARGET_BITTBOY || defined TARGET_PC
	if (CURRENT_SECTION.currentGameNode->data->preferences.frequency==OC_OC_LOW || CURRENT_SECTION.currentGameNode->data->preferences.frequency==OC_OC_HIGH) {
		drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+textWidth+1, (SCREEN_HEIGHT/2)-calculateProportionalSizeOrDistance1(9), "yes", valueColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	} else {
		drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+textWidth+1, (SCREEN_HEIGHT/2)-calculateProportionalSizeOrDistance1(9), "no", valueColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	}
#else
	drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+textWidth, (SCREEN_HEIGHT/2)-calculateProportionalSizeOrDistance1(9), "not available", valueColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
#endif
	drawRectangleToScreen(width, calculateProportionalSizeOrDistance1(1), SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2, problematicGray);
#endif
	
	drawRectangleToScreen(width, calculateProportionalSizeOrDistance1(1), SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2, problematicGray);	
	
	//Launch at boot option text
	drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+calculateProportionalSizeOrDistance1(4), (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(9), "Autostart: ", textColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	//Launch at boot option value
	if (launchAtBoot) {
		drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+textWidth, (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(9), "yes", valueColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	} else {
		drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+textWidth, (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(9), "no", valueColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	}

	drawRectangleToScreen(width, calculateProportionalSizeOrDistance1(1), SCREEN_WIDTH/2-width/2,SCREEN_HEIGHT/2+height/4, problematicGray);

	//Emulator option text
	drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+calculateProportionalSizeOrDistance1(4), (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(27), "Emulator: ", textColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
	//Emulator option value
	drawTextOnScreen(font, NULL, (SCREEN_WIDTH/2)-width/2+textWidth, (SCREEN_HEIGHT/2)+calculateProportionalSizeOrDistance1(27), emuName, valueColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);

	free(emuName);
#if defined MIYOOMINI
#else
	free(frequency);
#endif
	
	logMessage("INFO","showRomPreferences","Preferences shown");
}

void showConsole() {
	int backgroundColor[3];
	backgroundColor[0]=0;
	backgroundColor[1]=0;
	backgroundColor[2]=0;
	drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, backgroundColor);
	if (CURRENT_SECTION.systemLogoSurface!=NULL) {
		displaySurface(CURRENT_SECTION.systemLogoSurface, 0, 0);
		char *gameCount=malloc(100);
		snprintf(gameCount,100,gameCountText,CURRENT_SECTION.gameCount);

		int alignment = 0;
		switch(gameCountAlignment) {
			case 1:
				alignment = VAlignMiddle|HAlignCenter;
				break;
			case 2:
				alignment = VAlignMiddle|HAlignLeft;
				break;
		}

		if (displayGameCount) {
			drawTextOnScreen(sectionCardsGameCountFont, NULL, gameCountX, gameCountY, gameCount, gameCountFontColor, alignment, (int[]){}, 0);
			free(gameCount);
		}

	} else {
		drawRectangleToScreen(SCREEN_WIDTH,SCREEN_HEIGHT,0,0,(int[]){253,35,39});
		drawTextOnScreen(font,NULL,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,CURRENT_SECTION.sectionName,(int[]){0,0,0},VAlignMiddle|HAlignCenter, (int[]){}, 0);
	}
	logMessage("INFO","showConsole","Current console shown");
}

void displayGamePicture(struct Rom *rom) {
	char *pictureWithFullPath=malloc(600);
	char *tempGameName=malloc(300);
	char *originalGameName = NULL;
	if (currentSectionNumber==favoritesSectionNumber) {
		if (favoritesSize == 0) {
			return;
		}
		struct Favorite favorite = favorites[CURRENT_GAME_NUMBER];
		strcpy(pictureWithFullPath, favorite.filesDirectory);
		tempGameName=getGameName(favorite.name);
		originalGameName = favorite.name;
	} else {
		if (rom==NULL) {
			strcpy(pictureWithFullPath, "NO GAMES FOUND");
			tempGameName=getGameName("NO GAMES FOUND");
			originalGameName = tempGameName;
		} else {
			strcpy(pictureWithFullPath, rom->directory);
			tempGameName=getGameName(rom->name);
			originalGameName = rom->name;
		}
	}

	int len = strlen(originalGameName);
	const char *lastFour = &originalGameName[len-4];

	if (strcmp(lastFour,".png") != 0) {
		strcat(pictureWithFullPath,mediaFolder);
		strcat(pictureWithFullPath,"/");
	}
	strcat(pictureWithFullPath,tempGameName);
	strcat(pictureWithFullPath,".png");

	displayBackgroundPicture();
	if (rom==NULL) {
		if (displayCenteredImageOnScreen(pictureWithFullPath, tempGameName, 1,1)!=0) {
			drawImgFallbackTextOnScreen(tempGameName);
		}
		return;
	}
	stripGameNameLeaveExtension(tempGameName);
	if (strlen(CURRENT_SECTION.aliasFileName)>1||currentSectionNumber==favoritesSectionNumber) {
		char* displayName=NULL;
		if (rom!=NULL) {
			displayName=getFileNameOrAlias(rom);
		}
		if (!isFavoritesSectionSelected()&&rom!=NULL&&(stripGames||strlen(CURRENT_SECTION.aliasFileName)>1)) {
			if (stripGames) {
				strcpy(displayName,getAliasWithoutAlternateNameOrParenthesis(rom->alias));
			} else {
				strcpy(displayName,rom->alias);
			}
			displayCenteredImageOnScreen(pictureWithFullPath, displayName, 1,1);

			if (displayCenteredImageOnScreen(pictureWithFullPath, displayName, 1,1)!=0) {
				drawImgFallbackTextOnScreen(displayName);
			}
			drawPictureTextOnScreen(displayName);
		} else {
			if (isFavoritesSectionSelected()) {
				if (rom!=NULL) {
					if (strlen(rom->alias)<2) {
						char tmp[300];
						strcpy(tmp,getNameWithoutPath(rom->name));
						strcpy(tmp,getNameWithoutExtension(tmp));
						if (stripGames) {
							char * temp1 = getAliasWithoutAlternateNameOrParenthesis(tmp);
							if (displayCenteredImageOnScreen(pictureWithFullPath, temp1, 1,1)!=0) {
								drawImgFallbackTextOnScreen(temp1);
							}
							drawPictureTextOnScreen(temp1);
							free(temp1);
						} else {
							if (displayCenteredImageOnScreen(pictureWithFullPath, tmp, 1,1)!=0) {
								drawImgFallbackTextOnScreen(tmp);
							}
							drawPictureTextOnScreen(tmp);
						}
					} else {
						if (stripGames) {
							char * temp1 = getAliasWithoutAlternateNameOrParenthesis(rom->alias);
							if (displayCenteredImageOnScreen(pictureWithFullPath, temp1, 1,1)!=0) {
								drawImgFallbackTextOnScreen(temp1);
							}
							drawPictureTextOnScreen(temp1);
							free(temp1);
						} else {
							if (displayCenteredImageOnScreen(pictureWithFullPath, rom->alias, 1,1)!=0) {
								drawImgFallbackTextOnScreen(rom->alias);
							}
							drawPictureTextOnScreen(rom->alias);
						}
					}
				}
			}
		}
		free(displayName);
	} else {
		if (stripGames) {
			if (rom!=NULL) {
				if (rom->alias==NULL||strlen(rom->alias)<2) {
					if (displayCenteredImageOnScreen(pictureWithFullPath, tempGameName, 1,1)!=0) {
						drawImgFallbackTextOnScreen(tempGameName);
					}
					drawPictureTextOnScreen(tempGameName);
				} else {
					if (displayCenteredImageOnScreen(pictureWithFullPath, rom->alias, 1,1)!=0) {
						drawImgFallbackTextOnScreen(rom->alias);
					}
					drawPictureTextOnScreen(rom->alias);
				}
			}
		} else {
			if (rom!=NULL) {
				if (rom->alias==NULL||strlen(rom->alias)<2) {
					char tmp[300];
					strcpy(tmp,getNameWithoutPath(rom->name));
					strcpy(tmp,getNameWithoutExtension(tmp));
					if (displayCenteredImageOnScreen(pictureWithFullPath, tmp, 1,1)!=0) {
						drawImgFallbackTextOnScreen(tmp);
					}
					drawPictureTextOnScreen(tmp);
				} else {
					if (displayCenteredImageOnScreen(pictureWithFullPath, rom->alias, 1,1)!=0) {
						drawImgFallbackTextOnScreen(rom->alias);
					}
					drawPictureTextOnScreen(rom->alias);
				}
			}
		}
	}
	if (!isPicModeMenuHidden&&menuVisibleInFullscreenMode) {
		if(!isFavoritesSectionSelected()) {
			if (colorfulFullscreenMenu) {
				drawTransparentRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, CURRENT_SECTION.fullScreenMenuBackgroundColor, 180);
				drawTransparentRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, (int[]){0,0,0},100);
			} else {
				drawTransparentRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, (int[]){0,0,0}, 180);
			}
		} else {
			drawTransparentRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, (int[]){0,0,0}, 180);
		}
	}

	free(pictureWithFullPath);
	free(tempGameName);
	logMessage("INFO","displayGamePicture","Displayed game picture - fullscreen");
}

void displayGamePictureInMenu(struct Rom *rom) {
	char *pictureWithFullPath=malloc(600);
	char *tempGameName=malloc(300);
	char *originalGameName = NULL;
	if (currentSectionNumber==favoritesSectionNumber) {
		if (favoritesSize == 0) {
			return;
		}
		struct Favorite favorite = favorites[CURRENT_GAME_NUMBER];
		strcpy(pictureWithFullPath, favorite.filesDirectory);
		originalGameName = favorite.name;
		tempGameName=getGameName(favorite.name);
	} else {
		if (rom==NULL) {
			logMessage("INFO","displayGamePictureInMenu","No games!");
			strcpy(pictureWithFullPath, "NO GAMES FOUND");
			tempGameName=getGameName("NO GAMES FOUND");
			originalGameName = tempGameName;
		} else {
			logMessage("INFO","displayGamePictureInMenu","Displaying...");
			strcpy(pictureWithFullPath, rom->directory);
			originalGameName = rom->name;
			tempGameName=getGameName(rom->name);
		}
	}

	int len = strlen(originalGameName);
	const char *lastFour = &originalGameName[len-4];

	if (strcmp(lastFour,".png") != 0) {
		strcat(pictureWithFullPath,mediaFolder);
		strcat(pictureWithFullPath,"/");
	}

	strcat(pictureWithFullPath,tempGameName);
	strcat(pictureWithFullPath,".png");
	displayImageOnMenuScreen(pictureWithFullPath);

	free(pictureWithFullPath);
	free(tempGameName);
	logMessage("INFO","displayGamePictureInMenu","Displayed game picture");
}

void drawHeader() {
	drawTextOnHeader();
}

void drawTimedShutDownScreen() {
	int black[] = {0,0,0};
	drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, black);
	char text[30];
	char text1[30];
	char text2[30];
	if (shutDownEnabled) {
		snprintf(text,30,"SHUTTING DOWN");
	} else {
		snprintf(text,30,"QUITTING");
	}
	snprintf(text1,30,"IN %d SECONDS",countDown);
	snprintf(text2,30,"X TO CANCEL");
	drawTextOnScreen(BIGFont, NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT/4, text, (int[]){253,35,39}, VAlignMiddle | HAlignCenter, (int[]){0,0,0}, 0);
	drawTextOnScreen(BIGFont, NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, text1, (int[]){253,35,39}, VAlignMiddle | HAlignCenter, (int[]){0,0,0}, 0);
	drawTextOnScreen(BIGFont, NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT-SCREEN_HEIGHT/4, text2, (int[]){253,35,39}, VAlignMiddle | HAlignCenter, (int[]){0,0,0}, 0);
}

void drawShutDownScreen() {
	int black[] = {0,0,0};
	drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, black);
	if (selectedShutDownOption==1) {
		#ifdef MIYOOMINI
		if (mmModel)
			drawBigWhiteText("SHUTTING DOWN");
		else
			drawBigWhiteText("REBOOTING");
		#else
		drawBigWhiteText("REBOOTING");
		#endif
	} else {
		drawBigWhiteText("SHUTTING DOWN");
	}
}

void drawGameList() {
	currentGameNameBeingDisplayed[0]=' ';
	currentGameNameBeingDisplayed[1]='\0';
	gamesInPage=0;
	int nextLine = 0;
	if(!fullscreenMode) {
		nextLine = gameListY;
	} else {
		nextLine = gameListPositionFullScreen;
	}

	char *nameWithoutExtension;
	struct Node* currentNode;
	char *buf;
	currentNode = GetNthNode(ITEMS_PER_PAGE*CURRENT_SECTION.currentPage);
	for (int i=0;i<ITEMS_PER_PAGE;i++) {
		if (currentNode==NULL) {
			break;
		}
		struct Rom* rom = currentNode->data;
		gamesInPage++;
		if (rom->alias!=NULL &&  (strlen(rom->alias)>2)) {
			nameWithoutExtension=malloc(strlen(rom->alias)+1);
			strcpy(nameWithoutExtension,rom->alias);
			if(stripGames) {
				char* temp1 = getAliasWithoutAlternateNameOrParenthesis(rom->alias);
				free(nameWithoutExtension);
				nameWithoutExtension=malloc(strlen(temp1)+1);
				strcpy(nameWithoutExtension,temp1);
				free(temp1);
			}
		} else {
			nameWithoutExtension=malloc(strlen(rom->name)+1);
			strcpy(nameWithoutExtension,rom->name);
			if(stripGames) {
				char* temp1 = strdup(nameWithoutExtension);
				stripGameName(temp1);
				free(nameWithoutExtension);
				nameWithoutExtension=malloc(strlen(temp1)+1);
				strcpy(nameWithoutExtension,temp1);
				free(temp1);
			} else {
				char *nameWithoutPath=getNameWithoutPath(nameWithoutExtension);
				free(nameWithoutExtension);
				nameWithoutExtension=getNameWithoutExtension(nameWithoutPath);
				free(nameWithoutPath);
			}
		}
		buf=strdup(nameWithoutExtension);

		char *temp = malloc(strlen(buf)+2);
		if(currentSectionNumber!=favoritesSectionNumber) {
			#if defined MIYOOMINI
			strcpy(temp,buf);
			#else
			if (rom->preferences.frequency == OC_OC_HIGH||rom->preferences.frequency == OC_OC_LOW) {
				strcpy(temp,"+");
				strcat(temp,buf);
			} else {
				strcpy(temp,buf);
			}
			#endif
		} else {
			if (CURRENT_SECTION.gameCount>0) {
				#if defined MIYOOMINI
				strcpy(temp,buf);
				#else
				if (favorites[i].frequency == OC_OC_HIGH||favorites[i].frequency == OC_OC_LOW) {
					strcpy(temp,"+");
					strcat(temp,buf);
				} else {
					strcpy(temp,buf);
				}
				#endif
			} else {
				free(nameWithoutExtension);
				free(buf);
				free(temp);
				currentNode=NULL;
			}
		}

		if (i==menuSections[currentSectionNumber].currentGameInPage) {
			if(strlen(buf)>0) {
				if(fullscreenMode) {
					if(!isPicModeMenuHidden&&menuVisibleInFullscreenMode) {
						drawShadedGameNameOnScreenPicMode(temp, nextLine);
					}
				} else {
					MAGIC_NUMBER = gameListWidth;
					strcpy(currentGameNameBeingDisplayed,temp);
					displayGamePictureInMenu(rom);
					drawScrolledShadedGameNameOnScreenCustom(temp, nextLine);
					// drawShadedGameNameOnScreenCustom(temp, nextLine);
				}
			}
		} else {
			if(strlen(buf)>0) {
				if(fullscreenMode) {
					if(!isPicModeMenuHidden&&menuVisibleInFullscreenMode) {
						drawNonShadedGameNameOnScreenPicMode(temp, nextLine);
					}
				} else {
					MAGIC_NUMBER = gameListWidth;
					drawNonShadedGameNameOnScreenCustom(temp, nextLine);
				}
			}
		}
		if (!fullscreenMode) {
			nextLine+=itemsSeparation;
		} else {
			nextLine+=itemsSeparation;
		}
		free(nameWithoutExtension);
		free(buf);
		free(temp);
		currentNode = currentNode->next;
	}
	MAGIC_NUMBER = SCREEN_WIDTH-2;
	logMessage("INFO","drawGameList","Game list - Done");
}

void setupDecorations() {
	if (text1X!=-1&&text1Y!=-1) {
		drawHeader();
	}
	char *gameNumber=malloc(10);
	snprintf(gameNumber,10,"%d/%d",CURRENT_SECTION.gameCount>0?(CURRENT_SECTION.currentGameInPage+ITEMS_PER_PAGE*CURRENT_SECTION.currentPage)+1:0,CURRENT_SECTION.gameCount);
	drawGameNumber(gameNumber, text2X, text2Y);
	free(gameNumber);
	logMessage("INFO","setupDecorations","Decorations set");
}

void drawBatteryMeter() {
	int batteryLevel95to100[] = {1,255,1};
	int batteryLevel90to95[] = {1,255,1};
	int batteryLevel85to90[] = {1,255,1};
	int batteryLevel80to85[] = {1,255,1};
	int batteryLevel75to80[] = {153,254,0};
	int batteryLevel70to75[] = {153,254,0};
	int batteryLevel65to70[] = {153,254,0};
	int batteryLevel60to65[] = {153,254,0};
	int batteryLevel55to60[] = {255,254,3};
	int batteryLevel50to55[] = {255,254,3};
	int batteryLevel45to50[] = {255,254,3};
	int batteryLevel40to45[] = {255,254,3};
	int batteryLevel35to40[] = {255,152,1};
	int batteryLevel30to35[] = {255,152,1};
	int batteryLevel25to30[] = {255,152,1};
	int batteryLevel20to25[] = {255,152,1};
	int batteryLevel15to20[] = {255,51,0};
	int batteryLevel10to15[] = {255,51,0};
	int batteryLevel5to10[] = {255,51,0};
	int batteryLevel0to5[] = {255,51,0};

	int gray5[]={121, 121, 121};

	int *levels[20];
	levels[0] = batteryLevel0to5;
	levels[1] = batteryLevel5to10;
	levels[2] = batteryLevel10to15;
	levels[3] = batteryLevel15to20;
	levels[4] = batteryLevel20to25;
	levels[5] = batteryLevel25to30;
	levels[6] = batteryLevel30to35;
	levels[7] = batteryLevel35to40;
	levels[8] = batteryLevel40to45;
	levels[9] = batteryLevel45to50;
	levels[10] = batteryLevel50to55;
	levels[11] = batteryLevel55to60;
	levels[12] = batteryLevel60to65;
	levels[13] = batteryLevel65to70;
	levels[14] = batteryLevel70to75;
	levels[15] = batteryLevel75to80;
	levels[16] = batteryLevel80to85;
	levels[17] = batteryLevel85to90;
	levels[18] = batteryLevel90to95;
	levels[19] = batteryLevel95to100;

	drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(4), 0, calculateProportionalSizeOrDistance1(42), gray5);
	int pos = (lastChargeLevel);
	logMessage("INFO","drawSettingsScreen","Positioning batt 1");
	if (pos<21) {
		for (int i=pos-1;i>=0;i--) {
			drawRectangleToScreen(SCREEN_WIDTH/20, calculateProportionalSizeOrDistance1(4), (SCREEN_WIDTH/20)*i, calculateProportionalSizeOrDistance1(42), levels[i]);
		}
	} else {
		drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(4), 0, calculateProportionalSizeOrDistance1(42), (int[]){80,80,255});
	}
}

void drawSpecialScreen(char *title, char **options, char** values, char** hints, int interactive) {
	int headerAndFooterBackground[3]={253,35,39};
	int headerAndFooterText[3]={255,255,255};
	int bodyText[3]= {253,35,39};
	int bodyHighlightedText[3]= {255,255,255};
	int bodyBackground[3]={0,0,0};
	int problematicGray[3] = {124,0,2};

	logMessage("INFO","drawSettingsScreen","Setting options and values");

	logMessage("INFO","drawSettingsScreen","Drawing shit");
	drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT-calculateProportionalSizeOrDistance1(22), 0,calculateProportionalSizeOrDistance1(22), bodyBackground);
	drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(42), 0, 0, headerAndFooterBackground);
	drawTextOnSettingsHeaderLeftWithColor(title,headerAndFooterText);

	#if defined MIYOOMINI
    if (strcmp(title, "SCREEN SETTINGS") == 0) {
    int squareWidth = (((640-26)+10)/8)-10;
    int squareHeight = 32;
    int squareSpacing = 10;

    int startX = 13;
    int startY = 402;

    int colors[8][3] = {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {0, 255, 255},
        {255, 0, 255},
        {255, 255, 255},
        {125, 125, 125}
    };

    for (int i = 0; i < 8; i++) {
        int color[3] = { colors[i][0], colors[i][1], colors[i][2] };
        drawRectangleToScreen(squareWidth, squareHeight, startX, startY, color);
        startX += squareWidth + squareSpacing;
	}
	}
    #endif
	
	drawBatteryMeter();

	int nextLine = calculateProportionalSizeOrDistance1(50);
	int nextLineText = calculateProportionalSizeOrDistance1(50);
	int selected=0;
	#if defined TARGET_RFW
	int max = 9;
	#elif defined TARGET_OD || defined TARGET_OD_BETA || defined TARGET_PC
	int max = 9;
	#else
	int max = 9;
	#endif
	logMessage("INFO","drawSettingsScreen","Defining number of items");
	logMessage("INFO","drawSettingsScreen","About to go through items");
	for (int i=0;i<max;i++) {
		logMessage("INFO","drawSettingsScreen",options[i]);
		char temp[300];
		strcpy(temp,options[i]);
		if(strlen(values[i])>0) {
			strcat(temp,values[i]);
		}
		if(i==chosenSetting && interactive) {
			logMessage("INFO","drawSettingsScreen","Chosen setting");
			logMessage("INFO","drawSettingsScreen",options[i]);
			logMessage("INFO","drawSettingsScreen",values[i]);
			if (i==0) {
				drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(19), 0, nextLine-calculateProportionalSizeOrDistance1(4), problematicGray);
			} else if (i==max){
				drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(21), 0, nextLine-calculateProportionalSizeOrDistance1(4), problematicGray);
			} else {
				drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(20), 0, nextLine-calculateProportionalSizeOrDistance1(4), problematicGray);
			}
			selected=i;
		} else {
			if(!interactive) {
				selected=0;
			}
			logMessage("INFO","drawSettingsScreen","Non-Chosen setting");
			logMessage("INFO","drawSettingsScreen",options[i]);
			logMessage("INFO","drawSettingsScreen",values[i]);
		}
		drawSettingsOptionOnScreen(options[i], nextLineText, bodyText);
		drawSettingsOptionValueOnScreen(values[i], nextLineText, bodyHighlightedText);
		int lineColor[] = { 50,0,0};
		logMessage("INFO","drawSettingsScreen","Drawing rect");
		if (i<max)  {
			drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(1), 0, nextLine+calculateProportionalSizeOrDistance1(15), lineColor);
		}
		logMessage("INFO","drawSettingsScreen","Next line 1");
		nextLine+=calculateProportionalSizeOrDistance1(19);
		logMessage("INFO","drawSettingsScreen","Next line 2");
		nextLineText+=calculateProportionalSizeOrDistance1(19);
		logMessage("INFO","drawSettingsScreen","---");
	}
	logMessage("INFO","drawSettingsScreen","Out of the loop");
	drawRectangleToScreen(SCREEN_WIDTH, calculateProportionalSizeOrDistance1(22), 0, SCREEN_HEIGHT-calculateProportionalSizeOrDistance1(22), headerAndFooterBackground);
	logMessage("INFO","drawSettingsScreen","Drawing Footer");
	drawTextOnSettingsFooterWithColor(hints[selected], bodyBackground);
}

void setupHelpScreen(int page) {
	switch (page) {
		case 1:
			options[0]="A";
			values[0]="Confirm";

			options[1]="B";
			values[1]="Back";

			options[2]="X";
			values[2]="Mark favorite";

			options[3]="Y";
			values[3]="Show/Hide favorites";

#if defined TARGET_BITTBOY
			options[4]="R";
#else
			options[4]="R1";
#endif
			values[4]="Fullscreen mode";
			
			options[5]="L1";
		    values[5]="Open Search screen";

			options[6]="Select";
			values[6]="Game options";

			options[7]="Up/Down/Left/Right";
			values[7]="Scroll";
			
		    options[8]="Press B";
			values[8]="Function";
			
			hints[0] = "PAGE 1/2 - PRESS B TO RETURN";
			break;
		case 2:
			options[0] = "B+Select";
			values[0] = "Random select";

			options[1] = "B+X";
			values[1] = "Delete game";
			
			options[2]="B+Left/Right";
			values[2]="Previous/Next letter";

			options[3]="B+Up/Down";
			values[3]="Quick switch";
			
			hints[0] = "PAGE 2/2 - PRESS B TO RETURN";
			break;
	}
}

void setupAppearanceSettings() {
	options[0]="Tidy rom names ";
	if (stripGames) {
		values[0] = "enabled";
	} else {
		values[0] = "disabled";
	}
	hints[0] = "CUT DETAILS OUT OF ROM NAMES";

	options[1]="Fullscreen rom names ";
	if (footerVisibleInFullscreenMode) {
		values[1] = "enabled";
	} else {
		values[1] = "disabled";
	}
	hints[1] = "DISPLAY THE CURRENT ROM NAME";

	options[2]="Fullscreen menu ";
	if (menuVisibleInFullscreenMode) {
		values[2] = "enabled";
	} else {
		values[2] = "disabled";
	}
	hints[2] = "DISPLAY A TRANSLUCENT MENU";
}

#ifdef MIYOOMINI
void setupScreenSettings() {
    options[0]="Lumination ";
	values[0]=malloc(100);
	sprintf(values[0], "%d", luminationValue);
	hints[0] = "CHANGE THE LUMINATION";

    options[1]="Hue ";
	values[1]=malloc(100);
	sprintf(values[1], "%d", hueValue);
	hints[1] = "CHANGE THE COLOR";

    options[2]="Saturation ";
	values[2]=malloc(100);
	sprintf(values[2], "%d", saturationValue);
	hints[2] = "CHANGE THE SATURATION";

    options[3]="Contrast ";
	values[3]=malloc(100);
	sprintf(values[3], "%d", contrastValue);
	hints[3] = "CHANGE THE CONTRAST";
	
	options[4]="Gamma ";
	values[4]=malloc(100);
	sprintf(values[4], "%d", gammaValue);
	hints[4] = "CHANGE THE TEMP COLOR";
}
#endif

void setupSystemSettings() {
#ifdef MIYOOMINI
	options[0]="Volume ";
	values[0]=malloc(100);
	sprintf(values[0], "%d", volValue);
	hints[0] = "ADJUST VOLUME LEVEL";
#else
	options[0]="Sound ";
	hints[0] = "PRESS A TO LAUNCH ALSAMIXER";
#endif
	
	options[1]="Brightness ";
	values[1]=malloc(100);
	sprintf(values[1],"%d",brightnessValue);
	hints[1] = "ADJUST BRIGHTNESS LEVEL";
#if defined MIYOOMINI
#else
	options[2]="Sharpness ";
	values[2]=malloc(100);
//	char *temp = getenv("SDL_VIDEO_KMSDRM_SCALING_SHARPNESS");
//	if (temp!=NULL) {
//		sprintf(values[2],"%s",temp);
//		sharpnessValue=atoi(values[2]);
//	} else {
//		sharpnessValue=0;
//		values[2]="0";
//	}
	sprintf(values[2],"%d",sharpnessValue);
	hints[2] = "ADJUST SHARPNESS LEVEL";
	options[3]="Screen timeout ";
	values[3]=malloc(100);
	if (timeoutValue>0&&hdmiEnabled==0) {
		sprintf(values[3],"%d",timeoutValue);
	} else {
		sprintf(values[3],"%s","always on");
	}
	hints[3] = "MINUTES UNTIL THE SCREEN TURNS OFF";
#endif
#if defined (MIYOOMINI)
	options[2]="Cpu clock";
#else
	options[4]="Overclocking level";
#endif

#if defined TARGET_OD_BETA || defined TARGET_PC
	if (OCValue==OC_OC_LOW) {
		values[4]="low";
	} else if (OCValue==OC_OC_HIGH){
		values[4]="high";
	}
#else
#if defined (MIYOOMINI)
	FILE *fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
	int max_freq;
	fscanf(fp, "%d", &max_freq);
    fclose(fp);
	if (max_freq==1200000) {
		values[2]="1200Mhz";
	} else if (max_freq==1100000){
		values[2]="1100Mhz";
	} else if (max_freq==1000000){
		values[2]="1000Mhz";
	} else if (max_freq==800000){
		values[2]="800Mhz";
	} else if (max_freq==600000){
		values[2]="600Mhz";
	} else if (max_freq==400000){
		values[2]="400Mhz";
	}
#else
	values[4]="not available";
#endif
#endif
#if defined (MIYOOMINI)
	hints[2] = "CHANGE THE CPU SPEED";
#else
	hints[4] = "AFFECTS THE ROM MENU OC SETTING";
#endif

#ifdef MIYOOMINI
#else
#if defined TARGET_RFW || defined TARGET_OD_BETA
	options[5]="HDMI ";
	values[5] = " \0";
	if (hdmiChanged==1) {
		values[5] = "enabled";
	} else {
		values[5] = "disabled";
	}
#endif

#if defined TARGET_RFW
	hints[5] = "PRESS A TO ENABLE USB";
#elif defined TARGET_OD_BETA
	hints[5] = "PRESS A TO REBOOT AND ENABLE HDMI";
#else
	hints[5] = "ENABLE OR DISABLE HDMI";
#endif
#endif
	
#if defined MIYOOMINI
	options[3] = "Audio fix ";
    if (audioFix) {
		values[3] = "yes";
	} else {
		values[3] = "no";
	}
    hints[3] = "ENABLE OR DISABLE AUDIOSERVER";
	
	options[4]= "Music menu ";
	if (musicEnabled) {
		values[4] = "ON";
	} else {
		values[4] = "OFF";
	}
	hints[4] = "ENABLE OR DISABLE MUSIC MENU";

    options[5] = "Screen settings ";
	hints[5] = "SCREEN OPTIONS";
	
	options[6]= "Loading screen ";
	if (loadingScreenEnabled) {
		values[6] = "ON";
	} else {
		values[6] = "OFF";
	}
	hints[6] = "ENABLE OR DISABLE LOADING SCREEN";
	
	options[7] = "Screen timeout ";
	values[7] = malloc(100);
	if (timeoutValue>0) {
		sprintf(values[7],"%d",timeoutValue);
	} else {
		sprintf(values[7],"%s","always on");
	}
	hints[7] = "SECONDS UNTIL THE SCREEN TURNS OFF";
#endif
}

void setupSettingsScreen() {
	options[0]="Session ";
	hints[0] = "A TO CONFIRM - LEFT/RIGHT TO CHOOSE";
	if (shutDownEnabled) {
		switch (selectedShutDownOption) {
			case 0:
				values[0] = "shutdown";
				break;
			case 1:
				#ifdef MIYOOMINI
				if (mmModel)
					values[0] = "shutdown";
				else
					values[0] = "reboot";
				#else
				values[0] = "reboot";
				#endif
				break;
		}
	} else {
		switch (selectedShutDownOption) {
			case 0:
				values[0] = "quit";
				break;
			case 1:
				#ifdef MIYOOMINI
				if (mmModel)
					values[0] = "shutdown";
				else
					values[0] = "reboot";
				#else
				values[0] = "reboot";
				#endif
				break;
			case 2:
				values[0] = "shutdown";
				break;
		}
	}

	options[1]="Theme ";
	char *themeName=getNameWithoutPath((themes[activeTheme]));
	values[1] = themeName;
	hints[1] = "LAUNCHER THEME";
	#if defined MIYOOMINI
	#else
	options[2]="Default launcher ";
	#endif
	#ifdef MIYOOMINI
	#else
	if (shutDownEnabled) {
		values[2] = "yes";
	} else {
		values[2] = "no";
	}
	
	hints[2] = "LAUNCH AFTER BOOTING";
	#endif
	
	#if defined MIYOOMINI
	options[2]="Appearance ";
	hints[2] = "APPEARANCE OPTIONS";

	options[3]="System ";
	hints[3] = "SYSTEM OPTIONS";
	
	if (mmModel) {
		options[4]="RetroArch ";
		hints[4] = "RETROARCH CONFIGURATION";

		options[5]="Help ";
		hints[5] = "HOW TO USE THIS MENU";
	} else {
		options[4]= "Autowifi ";
		if (wifiEnabled) {
			values[4] = "ON";
		} else {
			values[4] = "OFF";
		}
		hints[4] = "ENABLE OR DISABLE AUTO WIFI IN BOOT";
		
		options[5]= "Wifi ";
		hints[5] = "WIFI MANAGER";
		
		options[6]="Scraper ";
		hints[6] = "DOWNLOAD COVERS FROM SCREENSCRAPER";
		
		options[7]="RetroArch ";
		hints[7] = "RETROARCH CONFIGURATION";

		options[8]="Help ";
		hints[8] = "HOW TO USE THIS MENU";
	}

	#else
	options[3]="Appearance ";
	hints[3] = "APPEARANCE OPTIONS";

	options[4]="System ";
	hints[4] = "SYSTEM OPTIONS";

	options[5]="Help ";
	hints[5] = "HOW TO USE THIS MENU";
	#endif
}


void clearOptionsValuesAndHints() {
        for (int i=0; i<10; i++) {
                options[i] = " ";
                values[i] = " ";
                hints[i] = " ";
        }
}

static int getKeyboardRowLength(int row) {
        return strlen(searchKeyboardRows[row]);
}

static int getKeyboardRowCount() {
        return sizeof(searchKeyboardRows) / sizeof(searchKeyboardRows[0]);
}

static void clampKeyboardPosition() {
        if (searchKeyboardRow < 0) {
                searchKeyboardRow = 0;
        }
        if (searchKeyboardRow >= getKeyboardRowCount()) {
                searchKeyboardRow = getKeyboardRowCount() - 1;
        }
        int maxCol = getKeyboardRowLength(searchKeyboardRow) - 1;
        if (searchKeyboardCol < 0) {
                searchKeyboardCol = 0;
        }
        if (searchKeyboardCol > maxCol) {
                searchKeyboardCol = maxCol;
        }
}

static void clampSearchSelection() {
if (searchSelectionIndex >= searchResultsCount) {
searchSelectionIndex = searchResultsCount > 0 ? searchResultsCount - 1 : 0;
}
if (searchSelectionIndex < 0) {
searchSelectionIndex = 0;
}
}

static void ensureSearchIndexPath() {
if (strlen(searchIndexPath) == 0) {
snprintf(searchIndexPath, sizeof(searchIndexPath), "%s/.simplemenu/search_index.dat", getenv("HOME"));
}
}

static void resetSearchIndexEntries() {
for (int i = 0; i < searchIndexCount; i++) {
free(searchIndexEntries[i].displayName);
}
free(searchIndexEntries);
searchIndexEntries = NULL;
searchIndexCount = 0;
searchIndexCapacity = 0;
searchIndexMetaCount = 0;
}

void searchInvalidateIndex() {
        resetSearchIndexEntries();
        searchIndexReady = 0;
}

static void appendSearchIndexEntry(int sectionIndex, int romIndex, const char *displayName) {
if (searchIndexCount >= searchIndexCapacity) {
int newCapacity = searchIndexCapacity == 0 ? 256 : searchIndexCapacity * 2;
SearchIndexEntry *resized = realloc(searchIndexEntries, newCapacity * sizeof(SearchIndexEntry));
if (resized == NULL) {
return;
}
searchIndexEntries = resized;
searchIndexCapacity = newCapacity;
}
searchIndexEntries[searchIndexCount].sectionIndex = sectionIndex;
searchIndexEntries[searchIndexCount].romIndex = romIndex;
searchIndexEntries[searchIndexCount].displayName = strdup(displayName);
searchIndexCount++;
}

static void recordSearchIndexSectionMeta(int sectionIndex, int gameCount) {
if (searchIndexMetaCount < (int)(sizeof(searchIndexMeta) / sizeof(searchIndexMeta[0]))) {
searchIndexMeta[searchIndexMetaCount].sectionIndex = sectionIndex;
searchIndexMeta[searchIndexMetaCount].gameCount = gameCount;
searchIndexMetaCount++;
}
}

static void drawSearchIndexProgress(const char *message, int current, int total) {
int backgroundColor[3] = {6, 6, 6};
int borderColor[3] = {120, 95, 70};
int fillColor[3] = {20, 18, 18};
drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, backgroundColor);
int boxWidth = SCREEN_WIDTH - calculateProportionalSizeOrDistance1(40);
int boxHeight = calculateProportionalSizeOrDistance1(80);
int boxX = (SCREEN_WIDTH - boxWidth) / 2;
int boxY = (SCREEN_HEIGHT - boxHeight) / 2;
drawBoxWithBorder(boxX, boxY, boxWidth, boxHeight, fillColor, borderColor, 2);
char progress[128];
snprintf(progress, sizeof(progress), "%s (%d/%d)", message, current, total);
drawTextOnScreen(searchFont, NULL, boxX + boxWidth/2, boxY + boxHeight/2 - calculateProportionalSizeOrDistance1(8), (char *)progress,
        (int[]){235, 235, 235}, VAlignMiddle | HAlignCenter, (int[]){}, 0);
int barWidth = boxWidth - calculateProportionalSizeOrDistance1(20);
int barHeight = calculateProportionalSizeOrDistance1(12);
int barX = boxX + calculateProportionalSizeOrDistance1(10);
int barY = boxY + boxHeight - calculateProportionalSizeOrDistance1(22);
drawBoxWithBorder(barX, barY, barWidth, barHeight, (int[]){35, 30, 30}, borderColor, 1);
if (total > 0 && current > 0) {
        int filledWidth = (barWidth * current) / total;
        if (filledWidth > barWidth) {
                filledWidth = barWidth;
        }
        drawRectangleToScreen(filledWidth, barHeight - 2, barX + 1, barY + 1, (int[]){245, 180, 68});
}
SDL_Flip(screen);
}

static int recountSectionGameCount(int sectionIndex) {
        int previousSection = currentSectionNumber;
        if (sectionIndex < 0 || sectionIndex >= menuSectionCounter) {
                return 0;
        }
	currentSectionNumber = sectionIndex;
	menuSections[sectionIndex].counted = 0;
	int count = theSectionHasGames(&menuSections[sectionIndex]);
	menuSections[sectionIndex].gameCount = count;
	currentSectionNumber = previousSection;
	return count;
}

static int currentGameCountForSection(int sectionIndex) {
        return recountSectionGameCount(sectionIndex);
}

static void refreshSectionGameCountCache() {
        searchSectionCountCacheSize = menuSectionCounter;
        for (int sectionIndex = 0; sectionIndex < menuSectionCounter && sectionIndex < (int)(sizeof(searchSectionGameCountCache) / sizeof(searchSectionGameCountCache[0])); sectionIndex++) {
                if (sectionIndex == favoritesSectionNumber) {
                        searchSectionGameCountCache[sectionIndex] = 0;
                        continue;
                }
                searchSectionGameCountCache[sectionIndex] = currentGameCountForSection(sectionIndex);
        }
}

static int searchIndexMatchesCounts() {
        int seenSections[100] = {0};
        for (int i = 0; i < searchIndexMetaCount; i++) {
                int sectionIndex = searchIndexMeta[i].sectionIndex;
                if (sectionIndex == favoritesSectionNumber) {
                        continue;
                }
                int currentCount = (sectionIndex < searchSectionCountCacheSize) ? searchSectionGameCountCache[sectionIndex] : currentGameCountForSection(sectionIndex);
                seenSections[sectionIndex] = 1;
                if (currentCount != searchIndexMeta[i].gameCount) {
                        return 0;
                }
        }
        for (int sectionIndex = 0; sectionIndex < menuSectionCounter; sectionIndex++) {
                if (sectionIndex == favoritesSectionNumber) {
                        continue;
                }
                int count = (sectionIndex < searchSectionCountCacheSize) ? searchSectionGameCountCache[sectionIndex] : currentGameCountForSection(sectionIndex);
                if (count > 0 && !menuSections[sectionIndex].hidden && !seenSections[sectionIndex]) {
                        return 0;
                }
        }
        return 1;
}

static int loadSearchIndexFromDisk() {
ensureSearchIndexPath();
FILE *fp = fopen(searchIndexPath, "rb");
if (fp == NULL) {
return 0;
}
char magic[7] = {0};
if (fread(magic, 1, 6, fp) != 6 || strncmp(magic, "SMIDX1", 6) != 0) {
fclose(fp);
return 0;
}
int storedSectionCount = 0;
int storedEntryCount = 0;
if (fread(&storedSectionCount, sizeof(int), 1, fp) != 1 || fread(&storedEntryCount, sizeof(int), 1, fp) != 1) {
fclose(fp);
return 0;
}
searchIndexMetaCount = 0;
for (int i = 0; i < storedSectionCount; i++) {
int sectionIndex = 0;
int gameCount = 0;
if (fread(&sectionIndex, sizeof(int), 1, fp) != 1 || fread(&gameCount, sizeof(int), 1, fp) != 1) {
resetSearchIndexEntries();
fclose(fp);
return 0;
}
if (sectionIndex >= menuSectionCounter) {
resetSearchIndexEntries();
fclose(fp);
return 0;
}
recordSearchIndexSectionMeta(sectionIndex, gameCount);
}
if (!searchIndexMatchesCounts()) {
resetSearchIndexEntries();
fclose(fp);
return 0;
}
for (int i = 0; i < storedEntryCount; i++) {
int sectionIndex = 0;
int romIndex = 0;
uint16_t nameLength = 0;
if (fread(&sectionIndex, sizeof(int), 1, fp) != 1 || fread(&romIndex, sizeof(int), 1, fp) != 1 ||
fread(&nameLength, sizeof(uint16_t), 1, fp) != 1) {
resetSearchIndexEntries();
fclose(fp);
return 0;
}
char *buffer = malloc(nameLength + 1);
if (buffer == NULL) {
resetSearchIndexEntries();
fclose(fp);
return 0;
}
if (fread(buffer, 1, nameLength, fp) != nameLength) {
free(buffer);
resetSearchIndexEntries();
fclose(fp);
return 0;
}
buffer[nameLength] = '\0';
appendSearchIndexEntry(sectionIndex, romIndex, buffer);
free(buffer);
}
fclose(fp);
searchIndexReady = 1;
return 1;
}

static void persistSearchIndexToDisk() {
ensureSearchIndexPath();
char indexDir[PATH_MAX];
snprintf(indexDir, sizeof(indexDir), "%s/.simplemenu", getenv("HOME"));
mkdir(indexDir, 0755);
FILE *fp = fopen(searchIndexPath, "wb");
if (fp == NULL) {
return;
}
const char magic[6] = "SMIDX1";
fwrite(magic, 1, sizeof(magic), fp);
fwrite(&searchIndexMetaCount, sizeof(int), 1, fp);
fwrite(&searchIndexCount, sizeof(int), 1, fp);
for (int i = 0; i < searchIndexMetaCount; i++) {
fwrite(&searchIndexMeta[i].sectionIndex, sizeof(int), 1, fp);
fwrite(&searchIndexMeta[i].gameCount, sizeof(int), 1, fp);
}
for (int i = 0; i < searchIndexCount; i++) {
uint16_t length = strlen(searchIndexEntries[i].displayName);
fwrite(&searchIndexEntries[i].sectionIndex, sizeof(int), 1, fp);
fwrite(&searchIndexEntries[i].romIndex, sizeof(int), 1, fp);
fwrite(&length, sizeof(uint16_t), 1, fp);
fwrite(searchIndexEntries[i].displayName, 1, length, fp);
}
fclose(fp);
}

static int buildSearchIndex() {
        resetSearchIndexEntries();
        searchIndexReady = 0;
        refreshSectionGameCountCache();
        int sectionsWithGames = 0;
        for (int sectionIndex = 0; sectionIndex < menuSectionCounter; sectionIndex++) {
                if (sectionIndex == favoritesSectionNumber) {
                        continue;
                }
                int count = (sectionIndex < searchSectionCountCacheSize) ? searchSectionGameCountCache[sectionIndex] : currentGameCountForSection(sectionIndex);
                if (count > 0 && !menuSections[sectionIndex].hidden) {
                        sectionsWithGames++;
                }
        }
        int progress = 0;
drawSearchIndexProgress("Preparing search index", progress, sectionsWithGames);
for (int sectionIndex = 0; sectionIndex < menuSectionCounter; sectionIndex++) {
                if (sectionIndex == favoritesSectionNumber) {
                        continue;
                }
                int count = (sectionIndex < searchSectionCountCacheSize) ? searchSectionGameCountCache[sectionIndex] : currentGameCountForSection(sectionIndex);
                if (count == 0 || menuSections[sectionIndex].hidden) {
                        continue;
                }
recordSearchIndexSectionMeta(sectionIndex, count);
progress++;
drawSearchIndexProgress("Building search index", progress, sectionsWithGames);
if (!ensureSectionInitialized(sectionIndex)) {
continue;
}
struct Node *current = menuSections[sectionIndex].head;
int index = 0;
while (current != NULL) {
char *name = getFileNameOrAlias(current->data);
appendSearchIndexEntry(sectionIndex, index, name);
free(name);
current = current->next;
index++;
}
}
persistSearchIndexToDisk();
searchIndexReady = 1;
return 1;
}

static int ensureSearchIndexReady() {
        refreshSectionGameCountCache();
        if (searchIndexReady && searchIndexMatchesCounts()) {
                return 1;
        }
        resetSearchIndexEntries();
        if (loadSearchIndexFromDisk()) {
                return 1;
}
return buildSearchIndex();
}

static int ensureSectionInitialized(int sectionIndex) {
        if (sectionIndex < 0 || sectionIndex >= menuSectionCounter) {
                return 0;
        }
        if (menuSections[sectionIndex].initialized) {
                return 1;
        }
        int previousSection = currentSectionNumber;
        int previousState = currentState;
        currentSectionNumber = sectionIndex;
        loadGameList(0);
        currentState = previousState;
        currentSectionNumber = previousSection;
        return menuSections[sectionIndex].initialized;
}

static const char *getSectionAbbreviation(const char *sectionName, const char *fantasyName) {
        static const SystemAbbreviation abbreviations[] = {
			{"AMIGA", "AMI"},
			{"AMSTRAD CPC", "CPC"},
			{"ARDUINO", "ARD"},
			{"ATARI 2600", "A26"},
			{"ATARI 5200", "A52"},
			{"ATARI 7800", "A78"},
			{"ATARI LYNX", "LYX"},
			{"ATARI ST", "AST"},
			{"COMMODORE 64", "C64"},
			{"CPS1", "CPS1"},
			{"CPS12", "CPS2"},
			{"CPS3", "CPS3"},
			{"DAPHNE", "DPN"},
			{"DOOM", "DOM"},
			{"DOS", "DOS"},
			{"FDS", "FDS"},
			{"FINALBURN NEO", "FBN"},
			{"FINALBURN ALPHA", "FBA"},
			{"GAME & WATCH", "G&W"},
			{"GAME BOY", "GB"},
			{"GAME BOY COLOR", "GBC"},
			{"GAME BOY ADVANCE", "GBA"},
			{"GAME GEAR", "GG"},
			{"GAMES", "PORT"},
			{"INTELLIVISION", "INT"},
			{"MAME", "MAM"},
			{"MASTER SYSTEM", "SMS"},
			{"MSU-1", "MSU1"},
			{"MSU-MD", "MSUM"},
			{"MSX", "MSX"},
			{"NES", "NES"},
			{"FC", "NES"},
			{"NEO GEO", "NEO"},
			{"NEO GEO CD", "NCD"},
			{"NEO GEO POCKET", "NGP"},
			{"NINTENDO DS", "NDS"},
			{"ODYSSEY2", "ODY"},
			{"OPENBOR", "BOR"},
			{"OVERLAYS", "OVER"},
			{"PC98", "PC98"},
			{"PC ENGINE", "PCE"},
			{"PC ENGINE CD", "PCEC"},
			{"PICO-8", "P8"},
			{"PLAYSTATION", "PSX"},
			{"PSX", "PSX"},
			{"POKEMON MINI", "MINI"},
			{"QUAKE", "QKE"},
			{"SCUMMVM", "SCM"},
			{"SGB", "SGB"},
			{"SEGA 32X", "32X"},
			{"SEGA CD", "SCD"},
			{"SEGA GENESIS", "SMD"},
			{"SEGA SG-1000", "SG10"},
			{"SNES", "SNES"},
			{"SUPERVISION", "SVIS"},
			{"TIC-80", "TIC"},
			{"VIRTUAL BOY", "VBOY"},
			{"WOLF3D", "W3D"},
			{"WONDERSWAN", "WS"},
			{"ZX SPECTRUM", "ZXS"},
			{"X68000", "X68"}
        };

        const char *name = (fantasyName != NULL && strlen(fantasyName) > 0) ? fantasyName : sectionName;
        if (name == NULL) {
                return "?";
        }

        size_t bestMatchLength = 0;
        const char *bestAbbreviation = NULL;

        for (size_t i = 0; i < sizeof(abbreviations) / sizeof(abbreviations[0]); i++) {
                if (strcasecmp(name, abbreviations[i].name) == 0) {
                        return abbreviations[i].abbreviation;
                }

                const char *found = strcasestr(name, abbreviations[i].name);
                if (found != NULL) {
                        size_t matchLength = strlen(abbreviations[i].name);
                        if (matchLength > bestMatchLength) {
                                bestMatchLength = matchLength;
                                bestAbbreviation = abbreviations[i].abbreviation;
                        }
                }
        }

        if (bestAbbreviation != NULL) {
                return bestAbbreviation;
        }

        static char fallback[5];
        int index = 0;
        for (const char *c = name; *c != '\0' && index < 3; c++) {
                if (isalnum((unsigned char)*c)) {
                        fallback[index++] = (char)toupper((unsigned char)*c);
                }
        }
        fallback[index] = '\0';
        if (index == 0) {
                strcpy(fallback, "?");
        }
        return fallback;
}

static void ellipsizeTextForWidth(TTF_Font *fontToUse, const char *text, int maxWidth, char *out, size_t outSize) {
        if (outSize == 0) {
                return;
        }
        if (text == NULL) {
                out[0] = '\0';
                return;
        }

        int fullWidth = 0;
        getTextWidth(fontToUse, text, &fullWidth);
        if (fullWidth <= maxWidth || maxWidth <= 0) {
                snprintf(out, outSize, "%s", text);
                return;
        }

        const char *ellipsis = "...";

        size_t len = strlen(text);
        while (len > 0) {
                char candidate[600];
                snprintf(candidate, sizeof(candidate), "%.*s%s", (int)len, text, ellipsis);
                int candidateWidth = 0;
                getTextWidth(fontToUse, candidate, &candidateWidth);
                if (candidateWidth <= maxWidth || len == 1) {
                        snprintf(out, outSize, "%s", candidate);
                        return;
                }
                len--;
        }

        snprintf(out, outSize, "%s", ellipsis);
}

static void drawHighlightedText(TTF_Font *fontToUse, int x, int y, char *text, char *query, int *baseColor, int *highlightColor) {
        if (query == NULL || strlen(query) == 0) {
                drawTextOnScreen(fontToUse, NULL, x, y, text, baseColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
                return;
        }
        char *match = strcasestr(text, query);
        if (match == NULL) {
                drawTextOnScreen(fontToUse, NULL, x, y, text, baseColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
                return;
        }

        int prefixLength = match - text;
        int matchLength = strlen(query);
        char prefix[600] = "";
        char highlighted[200] = "";
        char suffix[600] = "";

        strncpy(prefix, text, prefixLength);
        prefix[prefixLength] = '\0';
        strncpy(highlighted, match, matchLength);
        highlighted[matchLength] = '\0';
        strcpy(suffix, match + matchLength);

        int prefixWidth = 0;
        int highlightedWidth = 0;
        getTextWidth(fontToUse, prefix, &prefixWidth);
        getTextWidth(fontToUse, highlighted, &highlightedWidth);

        if (strlen(prefix) > 0) {
                drawTextOnScreen(fontToUse, NULL, x, y, prefix, baseColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
        }
        drawTextOnScreen(fontToUse, NULL, x + prefixWidth, y, highlighted, highlightColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
        if (strlen(suffix) > 0) {
                drawTextOnScreen(fontToUse, NULL, x + prefixWidth + highlightedWidth, y, suffix, baseColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
        }
}

static int getSearchResultsStartIndex(int maxVisible) {
        if (searchResultsCount <= maxVisible) {
                return 0;
        }
        int startIndex = searchSelectionIndex - (maxVisible / 2);
        if (startIndex < 0) {
                startIndex = 0;
        }
        int maxStart = searchResultsCount - maxVisible;
        if (startIndex > maxStart) {
                startIndex = maxStart;
        }
        return startIndex;
}

static void resetSearchState() {
        searchKeyboardRow = 0;
        searchKeyboardCol = 0;
        searchSelectionIndex = 0;
        searchFocusOnResults = 0;
        memset(searchQuery, 0, sizeof(searchQuery));
        searchResultsCount = 0;
        searchTotalMatches = 0;
}

static void rebuildSearchResults() {
        searchResultsCount = 0;
        searchTotalMatches = 0;
        if ((int)strlen(searchQuery) == 0) {
                searchSelectionIndex = 0;
                searchFocusOnResults = 0;
                return;
        }
        if (!ensureSearchIndexReady()) {
                return;
        }
        int maxResults = sizeof(searchResults) / sizeof(searchResults[0]);
        for (int i = 0; i < searchIndexCount; i++) {
                        if (strcasestr(searchIndexEntries[i].displayName, searchQuery) != NULL) {
                                searchTotalMatches++;
                                if (searchResultsCount < maxResults) {
                                        searchResults[searchResultsCount].sectionIndex = searchIndexEntries[i].sectionIndex;
                                        searchResults[searchResultsCount].romIndex = searchIndexEntries[i].romIndex;
                                        searchResults[searchResultsCount].displayName = searchIndexEntries[i].displayName;
                                        searchResultsCount++;
                                }
                        }
                }
        clampSearchSelection();
        if (searchResultsCount > 0) {
                if (searchSelectionIndex >= searchResultsCount) {
                        searchSelectionIndex = searchResultsCount - 1;
                }
        } else {
                searchSelectionIndex = 0;
                searchFocusOnResults = 0;
        }
}

static void appendCharacterToQuery(char character) {
        if ((int)strlen(searchQuery) >= (int)sizeof(searchQuery) - 1) {
                return;
        }
        int currentLength = strlen(searchQuery);
        searchQuery[currentLength] = toupper(character);
        searchQuery[currentLength + 1] = '\0';
        rebuildSearchResults();
        searchSelectionIndex = 0;
}

static void deleteCharacterFromQuery() {
        int len = strlen(searchQuery);
        if (len > 0) {
                searchQuery[len - 1] = '\0';
                rebuildSearchResults();
                searchSelectionIndex = 0;
                if (strlen(searchQuery) == 0) {
                        searchFocusOnResults = 0;
                }
        }
}

static void clearSearchQuery() {
        memset(searchQuery, 0, sizeof(searchQuery));
        rebuildSearchResults();
        searchSelectionIndex = 0;
        searchFocusOnResults = 0;
}

static void applyKeyboardSelection() {
        clampKeyboardPosition();
        char selectedChar = searchKeyboardRows[searchKeyboardRow][searchKeyboardCol];
        if (selectedChar == '<') {
                deleteCharacterFromQuery();
                return;
        }
        if (selectedChar == ' ') {
                appendCharacterToQuery(' ');
                return;
        }
        appendCharacterToQuery(selectedChar);
}

static void moveSelectionDown() {
        if (searchFocusOnResults) {
                if (searchSelectionIndex < searchResultsCount - 1) {
                        searchSelectionIndex++;
                }
        } else {
                searchKeyboardRow++;
                clampKeyboardPosition();
        }
}

static void moveSelectionUp() {
        if (searchFocusOnResults) {
                if (searchSelectionIndex > 0) {
                        searchSelectionIndex--;
                }
        } else {
                searchKeyboardRow--;
                clampKeyboardPosition();
        }
}

static void moveSelectionLeft() {
        if (searchFocusOnResults) {
                searchFocusOnResults = 0;
        } else {
                searchKeyboardCol--;
                clampKeyboardPosition();
        }
}

static void moveSelectionRight() {
        if (searchFocusOnResults) {
                searchFocusOnResults = 0;
        } else {
                searchKeyboardCol++;
                clampKeyboardPosition();
        }
}

static void toggleSearchFocus() {
        if (searchResultsCount > 0) {
                searchFocusOnResults = 1 - searchFocusOnResults;
        }
}

static void jumpToSearchSelection() {
        if (searchResultsCount == 0) {
                return;
        }
        SearchResult result = searchResults[searchSelectionIndex];

        closeSearchWindow();
        refreshScreen();
        SDL_Delay(40);

        if (ensureSectionInitialized(result.sectionIndex)) {
                if (currentSectionNumber != result.sectionIndex) {
                        currentSectionNumber = result.sectionIndex;
                        struct MenuSection *sec = &menuSections[currentSectionNumber];
                        if (sec->systemLogoSurface == NULL && sec->systemLogo[0] != '\0') {
                                sec->systemLogoSurface = IMG_Load(sec->systemLogo);
                        }
                        if (sec->backgroundSurface == NULL && sec->background[0] != '\0') {
                                sec->backgroundSurface = IMG_Load(sec->background);
                        }
                        if (sec->systemPictureSurface == NULL && sec->systemPicture[0] != '\0') {
                                sec->systemPictureSurface = IMG_Load(sec->systemPicture);
                        }
                } else {
                        currentSectionNumber = result.sectionIndex;
                }

                scrollToGame(result.romIndex);
                currentState = BROWSING_GAME_LIST;
                previousState = currentState;
                refreshRequest = 1;
                refreshScreen();
                SDL_Delay(20);
        }
}

static void drawBoxWithBorder(int x, int y, int width, int height, int *fillColor, int *borderColor, int thickness) {
        drawRectangleToScreen(width, height, x, y, fillColor);
        drawRectangleToScreen(width, thickness, x, y, borderColor);
        drawRectangleToScreen(width, thickness, x, y + height - thickness, borderColor);
        drawRectangleToScreen(thickness, height, x, y, borderColor);
        drawRectangleToScreen(thickness, height, x + width - thickness, y, borderColor);
}

static void drawSearchKeyboard(int startX, int startY, int availableWidth, int cellHeight) {
        int maxRowLength = 0;
        for (int row = 0; row < getKeyboardRowCount(); row++) {
                int length = getKeyboardRowLength(row);
                if (length > maxRowLength) {
                        maxRowLength = length;
                }
        }

        int cellWidth = availableWidth / (maxRowLength > 0 ? maxRowLength : 1);
        int height = getKeyboardRowCount() * cellHeight;
        int strokeColor[3] = {96, 80, 64};
        int darkFill[3] = {18, 15, 15};
        int gridFill[3] = {38, 38, 38};
        int selectedFill[3] = {240, 190, 80};
        int selectedText[3] = {15, 12, 8};
        int textColor[3] = {220, 220, 220};

        drawBoxWithBorder(startX, startY, availableWidth, height, darkFill, strokeColor, 2);

        for (int row = 0; row < getKeyboardRowCount(); row++) {
                int rowLength = getKeyboardRowLength(row);
                for (int col = 0; col < rowLength; col++) {
                        char character = searchKeyboardRows[row][col];
                        char label[7] = {0};
                        if (character == ' ') {
                                strcpy(label, "SPC");
                        } else if (character == '<') {
                                strcpy(label, "DEL");
                        } else {
                                snprintf(label, sizeof(label), "%c", character);
                        }

                        int blockX = startX + (col * cellWidth);
                        int blockY = startY + (row * cellHeight);
                        int isSelected = (!searchFocusOnResults && row == searchKeyboardRow && col == searchKeyboardCol);
                        int *cellFill = isSelected ? selectedFill : gridFill;
                        int *fontColor = isSelected ? selectedText : textColor;

                        drawRectangleToScreen(cellWidth, cellHeight, blockX, blockY, cellFill);
                        drawRectangleToScreen(cellWidth, 1, blockX, blockY, strokeColor);
                        drawRectangleToScreen(cellWidth, 1, blockX, blockY + cellHeight - 1, strokeColor);
                        drawRectangleToScreen(1, cellHeight, blockX, blockY, strokeColor);
                        drawRectangleToScreen(1, cellHeight, blockX + cellWidth - 1, blockY, strokeColor);

                        drawTextOnScreen(searchFont, NULL, blockX + cellWidth/2, blockY + cellHeight/2, label, fontColor, VAlignMiddle | HAlignCenter, (int[]){}, 0);
                }
        }
}

static void drawSearchResults(int startX, int startY, int listWidth, int listHeight, int maxVisible, int rowHeight) {
        if (maxVisible < 1) {
                maxVisible = 1;
        }
        if (strlen(searchQuery) == 0) {
                return;
        }
        int startIndex = getSearchResultsStartIndex(maxVisible);
        int endIndex = searchResultsCount < startIndex + maxVisible ? searchResultsCount : startIndex + maxVisible;
        int rowWidth = listWidth - calculateProportionalSizeOrDistance1(10);
        int leftPadding = calculateProportionalSizeOrDistance1(4);
        int rightPadding = calculateProportionalSizeOrDistance1(4);

        for (int i = startIndex; i < endIndex; i++) {
                SearchResult result = searchResults[i];
                const char *displayName = result.displayName != NULL ? result.displayName : "";
                int y = startY + ((i - startIndex) * rowHeight);
                int isSelected = (searchFocusOnResults && i == searchSelectionIndex);
                int yTop = y;
                int *textColor = isSelected ? (int[]){255, 220, 80} : (int[]){235, 235, 235};
                int *highlightColor = isSelected ? (int[]){12, 8, 6} : (int[]){245, 180, 68};
                int *bulletColor = isSelected ? (int[]){255, 140, 0} : (int[]){70, 62, 54};
                int *lineColor = isSelected ? (int[]){245, 180, 68} : (int[]){70, 62, 54};
                int *rowFill = isSelected ? (int[]){180, 90, 30} : (int[]){24, 22, 20};
                
                drawRectangleToScreen(rowWidth, rowHeight - 1, startX, yTop, rowFill);
                drawRectangleToScreen(rowWidth, 1, startX, yTop, lineColor);
                drawRectangleToScreen(rowWidth, 1, startX, yTop + rowHeight - 2, lineColor);
                drawRectangleToScreen(2, rowHeight - 1, startX, yTop, lineColor);
                int bulletWidth = 0;
                getTextWidth(searchFont, "■ ", &bulletWidth);
                int rowCenterY = yTop + (rowHeight / 2);
                int bulletX = startX + leftPadding;
                int textX = startX + bulletWidth + calculateProportionalSizeOrDistance1(2) + leftPadding;
                const char *sectionName = menuSections[result.sectionIndex].sectionName;
                const char *fantasyName = menuSections[result.sectionIndex].fantasyName;
                const char *abbreviation = getSectionAbbreviation(sectionName, fantasyName);
                char systemLabel[16];
                snprintf(systemLabel, sizeof(systemLabel), "[%s]", abbreviation);
                int systemLabelWidth = 0;
                getTextWidth(searchFont, systemLabel, &systemLabelWidth);
                int labelX = startX + rowWidth - rightPadding;
                int labelGap = calculateProportionalSizeOrDistance1(6);
                drawTextOnScreen(searchFont, NULL, bulletX, rowCenterY, "■", bulletColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
                int availableWidth = rowWidth - (textX - startX) - rightPadding - systemLabelWidth - labelGap;
                if (availableWidth < 0) {
                        availableWidth = 0;
                }
                char trimmedName[600];
                ellipsizeTextForWidth(searchFont, displayName, availableWidth, trimmedName, sizeof(trimmedName));
                drawHighlightedText(searchFont, textX, rowCenterY, trimmedName, searchQuery, textColor, highlightColor);
                drawTextOnScreen(searchFont, NULL, labelX, rowCenterY, systemLabel, textColor, VAlignMiddle | HAlignRight, (int[]){}, 0);
        }

        if (searchResultsCount == 0) {
                drawTextOnScreen(searchFont, NULL, startX + calculateProportionalSizeOrDistance1(4), startY + rowHeight / 2, "No matches", (int[]){200,200,200}, VAlignMiddle | HAlignLeft, (int[]){}, 0);
        }

        int remaining = searchTotalMatches - (endIndex);
        if (remaining > 0) {
                char moreLabel[80];
                snprintf(moreLabel, sizeof(moreLabel), "+%d more...", remaining);
                int footerY = startY + listHeight - calculateProportionalSizeOrDistance1(4);
                drawTextOnScreen(searchFont, NULL, startX + calculateProportionalSizeOrDistance1(4), footerY, moreLabel, (int[]){180,180,180}, VAlignMiddle | HAlignLeft, (int[]){}, 0);
        }
}

static void drawSearchOverlay() {
        drawTransparentRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, (int[]){0,0,0}, 185);

        int padding = calculateProportionalSizeOrDistance1(6);
        int margin = calculateProportionalSizeOrDistance1(5);
        int panelX = margin;
        int panelY = margin;
        int panelWidth = SCREEN_WIDTH - (margin * 2);
        int panelHeight = SCREEN_HEIGHT - (margin * 2);

        int frameColor[3] = {115, 95, 80};
        int fillColor[3] = {14, 13, 13};
        int accentColor[3] = {245, 180, 68};
        int labelColor[3] = {220, 220, 220};

        drawBoxWithBorder(panelX, panelY, panelWidth, panelHeight, fillColor, frameColor, 2);

        int textHeight = TTF_FontHeight(searchFont);
        if (textHeight <= 0) {
                textHeight = calculateProportionalSizeOrDistance1(10);
        }
        int headerHeight = textHeight + calculateProportionalSizeOrDistance1(4);
        int headerY = panelY + padding;
        int titleX = panelX + padding + calculateProportionalSizeOrDistance1(2);
        int titleWidth = 0;
        getTextWidth(searchFont, "SEARCH", &titleWidth);
        drawTextOnScreen(searchFont, NULL, titleX, headerY + headerHeight / 2, "SEARCH", labelColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);

        int querySpacing = calculateProportionalSizeOrDistance1(8);
        int queryX = titleX + titleWidth + querySpacing;
        int queryWidth = panelWidth - padding - queryX;
        int queryHeight = headerHeight;
        drawBoxWithBorder(queryX, headerY, queryWidth, queryHeight, (int[]){22, 20, 18}, frameColor, 1);
        char queryLabel[220];
        snprintf(queryLabel, sizeof(queryLabel), "> %s", strlen(searchQuery)>0?searchQuery:"_");
        drawTextOnScreen(searchFont, NULL, queryX + calculateProportionalSizeOrDistance1(4), headerY + queryHeight/2, queryLabel, accentColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);

        int contentTop = headerY + headerHeight + calculateProportionalSizeOrDistance1(6);
        int helpHeight = textHeight + calculateProportionalSizeOrDistance1(4);
        int keyboardCellHeight = textHeight + calculateProportionalSizeOrDistance1(1);
        int keyboardHeight = getKeyboardRowCount() * keyboardCellHeight + 2;
        int keyboardGap = calculateProportionalSizeOrDistance1(4);
        int helpY = panelY + panelHeight - padding - helpHeight;
        int keyboardY = helpY - keyboardGap - keyboardHeight;

        int resultsWidth = panelWidth - (padding * 2);
        int resultsHeight = keyboardY - calculateProportionalSizeOrDistance1(8) - contentTop;
        int resultsRowHeight = textHeight + calculateProportionalSizeOrDistance1(2);
        int maxVisible = resultsRowHeight > 0 ? resultsHeight / resultsRowHeight : 0;
        if (resultsHeight < resultsRowHeight) {
                resultsHeight = resultsRowHeight;
        }
        if (maxVisible > 22) {
                maxVisible = 22;
        }
        if (maxVisible < 1) {
                maxVisible = 1;
        }

        drawBoxWithBorder(panelX + padding, contentTop, resultsWidth, resultsHeight, (int[]){20, 18, 18}, frameColor, 1);
        drawSearchResults(panelX + padding, contentTop + calculateProportionalSizeOrDistance1(2), resultsWidth, resultsHeight - calculateProportionalSizeOrDistance1(4), maxVisible, resultsRowHeight);

        drawSearchKeyboard(panelX + padding, keyboardY, resultsWidth, keyboardCellHeight);
        drawRectangleToScreen(resultsWidth, 1, panelX + padding, keyboardY - calculateProportionalSizeOrDistance1(3), frameColor);

        drawTextOnScreen(searchFont, NULL, panelX + padding, helpY + helpHeight/2, "[A] select [X] switch kbd/list [B] del [Y] clear [L1] close", labelColor, VAlignMiddle | HAlignLeft, (int[]){}, 0);
}

static void drawBrowsingState(struct Rom *rom) {
        if (fullscreenMode) {
                if (currentSectionNumber == favoritesSectionNumber || CURRENT_SECTION.gameCount>0) {
                        logMessage("INFO","updateScreen","Fullscreen mode");
                        displayGamePicture(rom);
                        drawGameList();
                        displayHeart(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
                }
        } else {
                logMessage("INFO","updateScreen","Menu mode");
                displaySurface(CURRENT_SECTION.backgroundSurface, 0, 0);
                drawGameList();
                if (battX>-1 && surfaceBatt1!= NULL) {
                        switch (lastChargeLevel) {
                                case 1:
                                        displaySurface(surfaceBatt1, battX, battY);
                                        break;
                                case 2:
                                        displaySurface(surfaceBatt2, battX, battY);
                                        break;
                                case 3:
                                        displaySurface(surfaceBatt3, battX, battY);
                                        break;
                                case 4:
                                        displaySurface(surfaceBatt4, battX, battY);
                                        break;
                                case 5:
                                        displaySurface(surfaceBatt5, battX, battY);
                                        break;
                                case 6:
                                        displaySurface(surfaceBatt6, battX, battY);
                                        break;
                                case 7:
                                        displaySurface(surfaceBatt7, battX, battY);
                                        break;
                                case 8:
                                        displaySurface(surfaceBatt8, battX, battY);
                                        break;
                                case 9:
                                        displaySurface(surfaceBatt9, battX, battY);
                                        break;
                                case 10:
                                        displaySurface(surfaceBatt10, battX, battY);
                                        break;
                                case 11:
                                        displaySurface(surfaceBatt11, battX, battY);
                                        break;
                                case 12:
                                        displaySurface(surfaceBatt12, battX, battY);
                                        break;
                                case 13:
                                        displaySurface(surfaceBatt13, battX, battY);
                                        break;
                                case 14:
                                        displaySurface(surfaceBatt14, battX, battY);
                                        break;
                                case 15:
                                        displaySurface(surfaceBatt15, battX, battY);
                                        break;
                                case 16:
                                        displaySurface(surfaceBatt16, battX, battY);
                                        break;
                                case 17:
                                        displaySurface(surfaceBatt17, battX, battY);
                                        break;
                                case 18:
                                        displaySurface(surfaceBatt18, battX, battY);
                                        break;
                                case 19:
                                        displaySurface(surfaceBatt19, battX, battY);
                                        break;
                                case 20:
                                        displaySurface(surfaceBatt20, battX, battY);
                                        break;
                                default:
                                        displaySurface(surfaceBattCharging, battX, battY);
                                        break;
                        }
                }
                if (wifiX>-1 && surfaceWifiOff!= NULL) {
                        switch (lastWifiMode) {
                                case 0:
                                        displaySurface(surfaceWifiOff, wifiX, wifiY);
                                        break;
                                case 1:
                                        displaySurface(surfaceWifiOn, wifiX, wifiY);
                                        break;
                                case 2:
                                        displaySurface(surfaceNoWifi, wifiX, wifiY);
                                        break;
                                default:
                                        displaySurface(surfaceWifiOff, wifiX, wifiY);
                                        break;
                        }
                }
                setupDecorations(rom);
        }
        if (CURRENT_SECTION.alphabeticalPaging) {
                logMessage("INFO","updateScreen","Show alphabetical navigation bar");
                if (rom->name!=NULL) {
                        showLetter(rom);
                }
        }
        if (CURRENT_SECTION.gameCount==0) {
                if(fullscreenMode) {
                        drawRectangleToScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, (int[]){0,0,0});
                }
                generateError("NO GAMES FOUND-FOR THIS SECTION GROUP", 0);
                showErrorMessage(errorMessage);
        }
}

void openSearchWindow() {
	searchPreviousState = currentState;
	resetSearchState();
	if (!ensureSearchIndexReady()) {
		return;
	}
	rebuildSearchResults();
	currentState = SEARCHING_ROMS;
	refreshRequest = 1;
}

void closeSearchWindow() {
        searchFocusOnResults = 0;
        searchSelectionIndex = 0;
        aKeyComboWasPressed = 0;
        hotKeyPressed = 0;
        resetSearchState();
        CURRENT_SECTION.alphabeticalPaging = 0;
        currentState = isSettingsState(searchPreviousState) ? BROWSING_GAME_LIST : searchPreviousState;
        previousState = currentState;
        if (currentState != BROWSING_GAME_LIST && currentState != SELECTING_SECTION) {
                currentState = BROWSING_GAME_LIST;
	}
	refreshRequest = 1;
}

void handleSearchInput(int key) {
	if (key == BTN_L1) {
		closeSearchWindow();
		return;
	}
	if (key == BTN_UP) {
			moveSelectionUp();
        } else if (key == BTN_DOWN) {
                        moveSelectionDown();
        } else if (key == BTN_LEFT) {
                        if (searchFocusOnResults) {
                               for (int i = 0; i < 10; i++) {
				                    moveSelectionUp();
			                        }
                        } else {
                                moveSelectionLeft();
                        }
        } else if (key == BTN_RIGHT) {
                        if (searchFocusOnResults) {
                               for (int i = 0; i < 10; i++) {
				                    moveSelectionDown();
			                        }
                        } else {
                                moveSelectionRight();
                        }
        } else if (key == BTN_Y) {
                        clearSearchQuery();
        } else if (key == BTN_X) {
                        toggleSearchFocus();
        } else if (key == BTN_B) {
                        deleteCharacterFromQuery();
        } else if (key == BTN_A) {
                        if (searchFocusOnResults) {
                                jumpToSearchSelection();
                        } else {
                                applyKeyboardSelection();
                        }
        } else if (key == BTN_START) {
                        jumpToSearchSelection();
        }
        refreshRequest = 1;
}

void updateScreen(struct Node *node) {
	struct Rom *rom;
	if (node==NULL) {
		rom = NULL;
	} else {
		rom = node->data;
	}
	if (!itsStoppedBecauseOfAnError) {
                switch(currentState) {
                        case BROWSING_GAME_LIST:
                                drawBrowsingState(rom);
                                break;
                        case SEARCHING_ROMS:
                                drawBrowsingState(rom);
                                drawSearchOverlay();
                                break;
                        case SETTINGS_SCREEN:
                                clearOptionsValuesAndHints();
                                setupSettingsScreen();
                                drawSpecialScreen("SETTINGS", options, values, hints, 1);
				break;
			case HELP_SCREEN_1:
				clearOptionsValuesAndHints();
				setupHelpScreen(1);
				drawSpecialScreen("HELP", options, values, hints, 0);
				break;
			case HELP_SCREEN_2:
				clearOptionsValuesAndHints();
				setupHelpScreen(2);
				drawSpecialScreen("HELP", options, values, hints, 0);
				break;
			case APPEARANCE_SETTINGS:
				clearOptionsValuesAndHints();
				setupAppearanceSettings();
				drawSpecialScreen("APPEARANCE", options, values, hints, 1);
				break;
			case SYSTEM_SETTINGS:
				clearOptionsValuesAndHints();
				setupSystemSettings();
				drawSpecialScreen("SYSTEM", options, values, hints, 1);
				break;
#if defined MIYOOMINI
            case SCREEN_SETTINGS:
				clearOptionsValuesAndHints();
				setupScreenSettings();
				drawSpecialScreen("SCREEN SETTINGS", options, values, hints, 1);
				break;
#endif
			case CHOOSING_GROUP:
				showCurrentGroup();
				if (battX>-1 && surfaceBatt1!= NULL) {
						switch (lastChargeLevel) {
							case 1:
								displaySurface(surfaceBatt1, battX, battY);
								break;
							case 2:
								displaySurface(surfaceBatt2, battX, battY);
								break;
							case 3:
								displaySurface(surfaceBatt3, battX, battY);
								break;
							case 4:
								displaySurface(surfaceBatt4, battX, battY);
								break;
							case 5:
								displaySurface(surfaceBatt5, battX, battY);
								break;
							case 6:
								displaySurface(surfaceBatt6, battX, battY);
								break;
							case 7:
								displaySurface(surfaceBatt7, battX, battY);
								break;
							case 8:
								displaySurface(surfaceBatt8, battX, battY);
								break;
							case 9:
								displaySurface(surfaceBatt9, battX, battY);
								break;
							case 10:
								displaySurface(surfaceBatt10, battX, battY);
								break;
							case 11:
								displaySurface(surfaceBatt11, battX, battY);
								break;
							case 12:
								displaySurface(surfaceBatt12, battX, battY);
								break;
							case 13:
								displaySurface(surfaceBatt13, battX, battY);
								break;
							case 14:
								displaySurface(surfaceBatt14, battX, battY);
								break;
							case 15:
								displaySurface(surfaceBatt15, battX, battY);
								break;
							case 16:
								displaySurface(surfaceBatt16, battX, battY);
								break;
							case 17:
								displaySurface(surfaceBatt17, battX, battY);
								break;
							case 18:
								displaySurface(surfaceBatt18, battX, battY);
								break;
							case 19:
								displaySurface(surfaceBatt19, battX, battY);
								break;
							case 20:
								displaySurface(surfaceBatt20, battX, battY);
								break;							
							default:
								displaySurface(surfaceBattCharging, battX, battY);
								break;
						}
					}
				if (wifiX>-1 && surfaceWifiOff!= NULL) {
						switch (lastWifiMode) {
							case 0:
								displaySurface(surfaceWifiOff, wifiX, wifiY);
								break;
							case 1:
								displaySurface(surfaceWifiOn, wifiX, wifiY);
								break;
							case 2:
								displaySurface(surfaceNoWifi, wifiX, wifiY);
								break;
							default:
								displaySurface(surfaceWifiOff, wifiX, wifiY);
								break;
						}
					}
				break;
			case SELECTING_EMULATOR:
				showRomPreferences();
				break;
			case SELECTING_SECTION:
				showConsole();
				if (battX>-1 && surfaceBatt1!= NULL) {
						switch (lastChargeLevel) {
							case 1:
								displaySurface(surfaceBatt1, battX, battY);
								break;
							case 2:
								displaySurface(surfaceBatt2, battX, battY);
								break;
							case 3:
								displaySurface(surfaceBatt3, battX, battY);
								break;
							case 4:
								displaySurface(surfaceBatt4, battX, battY);
								break;
							case 5:
								displaySurface(surfaceBatt5, battX, battY);
								break;
							case 6:
								displaySurface(surfaceBatt6, battX, battY);
								break;
							case 7:
								displaySurface(surfaceBatt7, battX, battY);
								break;
							case 8:
								displaySurface(surfaceBatt8, battX, battY);
								break;
							case 9:
								displaySurface(surfaceBatt9, battX, battY);
								break;
							case 10:
								displaySurface(surfaceBatt10, battX, battY);
								break;
							case 11:
								displaySurface(surfaceBatt11, battX, battY);
								break;
							case 12:
								displaySurface(surfaceBatt12, battX, battY);
								break;
							case 13:
								displaySurface(surfaceBatt13, battX, battY);
								break;
							case 14:
								displaySurface(surfaceBatt14, battX, battY);
								break;
							case 15:
								displaySurface(surfaceBatt15, battX, battY);
								break;
							case 16:
								displaySurface(surfaceBatt16, battX, battY);
								break;
							case 17:
								displaySurface(surfaceBatt17, battX, battY);
								break;
							case 18:
								displaySurface(surfaceBatt18, battX, battY);
								break;
							case 19:
								displaySurface(surfaceBatt19, battX, battY);
								break;
							case 20:
								displaySurface(surfaceBatt20, battX, battY);
								break;								
							default:
								displaySurface(surfaceBattCharging, battX, battY);
								break;
						}
					}
				if (wifiX>-1 && surfaceWifiOff!= NULL) {
						switch (lastWifiMode) {
							case 0:
								displaySurface(surfaceWifiOff, wifiX, wifiY);
								break;
							case 1:
								displaySurface(surfaceWifiOn, wifiX, wifiY);
								break;
							case 2:
								displaySurface(surfaceNoWifi, wifiX, wifiY);
								break;
							default:
								displaySurface(surfaceWifiOff, wifiX, wifiY);
								break;
						}
					}
				break;
			case SHUTTING_DOWN:
				drawShutDownScreen();
				break;
			case AFTER_RUNNING_LAUNCH_AT_BOOT:
				drawTimedShutDownScreen();
				break;
		}
	} else if (itsStoppedBecauseOfAnError) {
		showErrorMessage(errorMessage);
	}
}

void setupKeys() {
	initializeKeys();
	logMessage("INFO","setupKeys","Input successfully configured");
}

void clearShutdownTimer() {
	if (shutdownTimer != NULL) {
		SDL_RemoveTimer(shutdownTimer);
	}
	shutdownTimer = NULL;
}

uint32_t countDownToShutdown() {
	countDown--;
	if(countDown==0) {
		running=0;
		clearShutdownTimer();
		refreshRequest=1;
		pushEvent();
		return 0;
	}
	refreshRequest=1;
	return 1000;
}

void resetShutdownTimer() {
	countDown=3;
	clearShutdownTimer();
	shutdownTimer=SDL_AddTimer(1 * 1e3, countDownToShutdown, NULL);
	updateScreen(NULL);
	refreshScreen();
}

void clearPicModeHideMenuTimer() {
	if (picModeHideMenuTimer != NULL) {
		SDL_RemoveTimer(picModeHideMenuTimer);
	}
	picModeHideMenuTimer = NULL;
}

uint32_t hideFullScreenModeMenu() {
	if(!hotKeyPressed) {
		clearPicModeHideMenuTimer();
		isPicModeMenuHidden=1;
	}
	refreshRequest=1;
	pushEvent();
	return 0;
}

void resetPicModeHideMenuTimer() {
	if (menuVisibleInFullscreenMode) {
		isPicModeMenuHidden=0;
		clearPicModeHideMenuTimer();
		picModeHideMenuTimer=SDL_AddTimer(0.6 * 1e3, hideFullScreenModeMenu, NULL);
	}
}

void clearPicModeHideLogoTimer() {
	if (picModeHideLogoTimer != NULL) {
		SDL_RemoveTimer(picModeHideLogoTimer);
	}
	picModeHideLogoTimer = NULL;
}

uint32_t hidePicModeLogo() {
	clearPicModeHideLogoTimer();
	hotKeyPressed=0;
	aKeyComboWasPressed=0;
	if (CURRENT_SECTION.backgroundSurface==NULL) {
		logMessage("INFO","screen","Loading system background");
		CURRENT_SECTION.backgroundSurface = loadImage(CURRENT_SECTION.background);
		logMessage("INFO","screen","Loading system picture");
		CURRENT_SECTION.systemPictureSurface = loadImage(CURRENT_SECTION.systemPicture);
	}
	currentState=BROWSING_GAME_LIST_AFTER_TIMER;
	refreshRequest=1;
	pushEvent();
	return 0;
}

void resetPicModeHideLogoTimer() {
	clearPicModeHideLogoTimer();
	picModeHideLogoTimer=SDL_AddTimer(0.7 * 1e3, hidePicModeLogo, NULL);
}

void clearHideHeartTimer() {
	if (hideHeartTimer != NULL) {
		SDL_RemoveTimer(hideHeartTimer);
	}
	hideHeartTimer = NULL;
}

uint32_t hideHeart() {
	clearHideHeartTimer();
	if(!isPicModeMenuHidden) {
		resetPicModeHideMenuTimer();
	}
	refreshRequest=1;
	pushEvent();
	return 0;
}

void resetHideHeartTimer() {
	clearHideHeartTimer();
	hideHeartTimer=SDL_AddTimer(0.5 * 1e3, hideHeart, NULL);
}

void clearBatteryTimer() {
        if (batteryTimer != NULL) {
                SDL_RemoveTimer(batteryTimer);
        }
        batteryTimer = NULL;
}

uint32_t batteryCallBack() {
	lastChargeLevel = getBatteryLevel();
	refreshRequest=1;
	return 60000;
}


void startBatteryTimer() {
	batteryTimer=SDL_AddTimer(1 * 60e3, batteryCallBack, NULL);
}

void clearWifiTimer() {
	if (wifiTimer != NULL) {
		SDL_RemoveTimer(wifiTimer);
	}
	wifiTimer = NULL;
}

uint32_t wifiCallBack() {
	lastWifiMode=getCurrentWifi();
	refreshRequest=1;
	return 60000;
}

void startWifiTimer() {
        wifiTimer=SDL_AddTimer(1 * 60e3, wifiCallBack, NULL);
}

void clearActiveRefreshTimer() {
        if (activeRefreshTimer != NULL) {
                SDL_RemoveTimer(activeRefreshTimer);
        }
        activeRefreshTimer = NULL;
}

uint32_t activeRefreshCallBack() {
        refreshRequest=1;
        return 5 * 1e3;
}

void startActiveRefreshTimer() {
        activeRefreshTimer=SDL_AddTimer(5 * 1e3, activeRefreshCallBack, NULL);
}

void freeResources() {
	freeFonts();
	freeSettingsFonts();
	TTF_Quit();
#if defined TARGET_OD || defined TARGET_OD_BETA
	Shake_Stop(device, effect_id);
	Shake_EraseEffect(device, effect_id);
	Shake_Close(device);
	Shake_Quit();
#endif
#ifndef TARGET_PC
	closeLogFile();
#endif
	SDL_Quit();
}

