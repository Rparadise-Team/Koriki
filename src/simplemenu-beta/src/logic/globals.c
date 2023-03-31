#ifndef GLOBALS_DEFINED
#define GLOBALS_DEFINED

#include <pthread.h>
#include <SDL/SDL_timer.h>
#include <SDL/SDL_image.h>
#include "../headers/hashtable.h"
#include "../headers/constants.h"

#if defined TARGET_OD || defined TARGET_OD_BETA
#include <shake.h>
#endif
SDL_Surface *screen;
/* STATUS */
int nullUpdate;
int updateScreenFlag;
int launchAtBoot;
int alternateControls;
char *errorMessage;
int running;
int itsStoppedBecauseOfAnError;
int thereIsACriticalError;
int favoritesChanged;
int returnTo;
int currentSectionNumber;
int currentCPU;
int fullscreenMode;
int hotKeyPressed;
int aKeyComboWasPressed;
int currentState;
int previousState;
int loading;
int isPicModeMenuHidden;
int isSuspended;
int activeGroup;
int beforeTryingToSwitchGroup;
int chosenSetting;
int previouslyChosenSetting;
int chosenChoosingOption;
char currentGameNameBeingDisplayed [3000];
SDL_TimerID shutdownTimer;
SDL_TimerID timeoutTimer;
SDL_TimerID picModeHideMenuTimer;
SDL_TimerID picModeHideLogoTimer;
SDL_TimerID hideHeartTimer;
SDL_TimerID batteryTimer;

typedef struct thread_picture {
	  SDL_Surface* display;
	  SDL_Surface *image;
	  int x;
	  int y;
	  int xx;
	  int yy;
	  double newwidth;
	  double newheight;
	  int transparent;
	  int smoothing;
} threadPicture;

/* QUANTITIES */
int MAX_GAMES_IN_SECTION;
int ITEMS_PER_PAGE;
int FULLSCREEN_ITEMS_PER_PAGE;
int MENU_ITEMS_PER_PAGE;
int favoritesSectionNumber;
int favoritesSize;
int menuSectionCounter;
int sectionGroupCounter;
int gamesInPage;

/* SETTINGS */
int TIDY_ROMS_OPTION;
int FULL_SCREEN_FOOTER_OPTION;
int FULL_SCREEN_MENU_OPTION;
int THEME_OPTION;
int SCREEN_TIMEOUT_OPTION;
int DEFAULT_OPTION;
int USB_OPTION;
#if defined MIYOOMINI
int AUDIOFIX_OPTION;
int SCREEN_OPTION;
int LUMINATION_OPTION;
int HUE_OPTION;
int SATURATION_OPTION;
int CONTRAST_OPTION;
int NUM_SCREEN_OPTIONS;
int COLOR_MAX_VALUE;
#endif
int VOLUME_OPTION;
int BRIGHTNESS_OPTION;
int SHARPNESS_OPTION;
int OC_OPTION;
int SHUTDOWN_OPTION;
int HELP_OPTION;
int APPEARANCE_OPTION;
int SYSTEM_OPTION;
int NUM_SYSTEM_OPTIONS;

char mediaFolder[1000];
int stripGames;
int useCache;
int shutDownEnabled;
int selectedShutDownOption;
int footerVisibleInFullscreenMode;
int menuVisibleInFullscreenMode;
int timeoutValue;
int OCValue;
int brightnessValue;
int maxBrightnessValue;
int sharpnessValue;
int volumeValue;
int OC_NO;
int OC_OC_LOW;
int OC_OC_HIGH;
int OC_SLEEP;
int backlightValue;
int hdmiChanged;
#if defined MIYOOMINI
int audioFix;
int luminationValue;
int volValue;
int hueValue;
int saturationValue;
int contrastValue;
int mmModel;
#endif
pthread_t myThread;

/* THEME */
char *themes[100];
int activeTheme;
int themeChanged;
int themeCounter;
int currentMode;
char menuFont[1000];
int baseFont;
int transparentShading;
int footerOnTop;
char simpleBackground[1000];
char fullscreenBackground[1000];
char favoriteIndicator[1000];
char sectionGroupsFolder[1000];
int itemsPerPage;
int itemsPerPageFullscreen;
int itemsSeparation;
char textXFont[1000];
char batt1[1000];
SDL_Surface* surfaceBatt1;
char batt2[1000];
SDL_Surface* surfaceBatt2;
char batt3[1000];
SDL_Surface* surfaceBatt3;
char batt4[1000];
SDL_Surface* surfaceBatt4;
char batt5[1000];
SDL_Surface* surfaceBatt5;
char battCharging[1000];
SDL_Surface* surfaceBattCharging;
int battX;
int battY;
int text1FontSize;
int newspaperMode;
int text1X;
int text1Y;
int text1Alignment;
int text2FontSize;
int text2X;
int text2Y;
int text2Alignment;
int text3FontSize;
int text3X;
int text3Y;
int text3Alignment;
int gameListAlignment;
int gameListX;
int gameListY;
int gameListWidth;
int gameListPositionFullScreen;
int artWidth;
int artHeight;
int currentW;
int currentH;
int artX;
int artY;
int artTextDistanceFromPicture;
int artTextLineSeparation;
int artTextFontSize;
int systemWidth;
int systemHeight;
int systemX;
int systemY;
int fontSize;
int colorfulFullscreenMenu;
int fontOutline;
int displaySectionGroupName;
int showArt;
int refreshRequest;

int displayGameCount;
char gameCountFont[1000];
int gameCountFontSize;
int gameCountX;
int gameCountY;
int gameCountAlignment;
int gameCountFontColor[3];
char gameCountText[100];

/* STRUCTS */
struct OPKDesktopFile {
	char parentOPK[200];
	char name[200];
	char displayName[200];
	char category[200];
	int isConsoleApp;
};

struct StolenGMenuFile {
	char title[200];
	char exec[200];
	char params[600];
	int isConsoleApp;
};

struct Favorite {
	char section[300];
	char sectionAlias[300];
	char name[300];
	char alias[300];
	char emulatorFolder[200];
	char executable[200];
	char filesDirectory[400];
	int isConsoleApp;
	int frequency;
};

struct AutostartRom {
	struct Rom *rom;
	char *emulator;
	char *emulatorDir;
};

struct RomPreferences {
	int emulator;
	int emulatorDir;
	int frequency;
};

struct Rom {
	char *name;
	char *alias;
	char *directory;
	struct RomPreferences preferences;
	int isConsoleApp;
};

struct Node  {
	struct Rom  *data;
	struct Node *next;
	struct Node *prev;
};

struct SectionGroup {
	char groupPath[1000];
	char groupName[25];
	char groupBackground[1000];
	SDL_Surface *groupBackgroundSurface;
};

struct MenuSection {
	int counted;
	char sectionName[250];
	char fantasyName[250];
	char *emulatorDirectories[10];
	char *executables[10];
	char filesDirectories[400];
	char fileExtensions[150];
	char systemLogo[300];
	SDL_Surface *systemLogoSurface;
	char systemPicture[300];
	SDL_Surface *systemPictureSurface;
	char aliasFileName[300];
	int hidden;
	int currentPage;
	int currentGameInPage;
	int realCurrentGameNumber;
	int alphabeticalPaging;
	int totalPages;
	int gameCount;
	int initialized;
	int onlyFileNamesNoExtension;
	int fullScreenMenuBackgroundColor[3];
	int fullscreenMenuItemsColor[3];
	int fullscreenSelectedMenuItemsColor[3];
	int menuItemsFontColor[3];
	int bodySelectedTextBackgroundColor[3];
	int bodySelectedTextTextColor[3];
	int pictureTextColor[3];
	struct Node* currentGameNode;
	struct Node *head;
	struct Node *tail;
	hashtable_t *aliasHashTable;
	int activeExecutable;
	int activeEmulatorDirectory;
	char category[100];
	char scaling[2];
	char background[1000];
	SDL_Surface *backgroundSurface;
	int hasDirs;
	char noArtPicture[4000];
};

struct SectionGroup sectionGroups[100];
int sectionGroupStates[100][100][5];
struct MenuSection menuSections[100];
struct Favorite favorites[2000];

/* CONTROL */
uint8_t *keys;
SDL_Joystick *joystick;
int BTN_Y;
int BTN_B;
int BTN_A;
int BTN_X;
int BTN_START;
int BTN_SELECT;
int BTN_R;
int BTN_UP;
int BTN_DOWN;
int BTN_LEFT;
int BTN_RIGHT;
int BTN_L1;
int BTN_R1;
int BTN_L2;
int BTN_R2;

#if defined TARGET_OD || defined TARGET_OD_BETA
 Shake_Device *device;
 Shake_Effect effect;
 int effect_id;
 Shake_Effect effect1;
 int effect_id1;
#endif

/* SCREEN */
int MAGIC_NUMBER;
int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int HDMI_WIDTH;
int HDMI_HEIGHT;
double SCREEN_RATIO;
int hdmiEnabled;

/* MISC */
time_t currRawtime;
struct tm * currTime;
int lastSec;
int lastMin;
int lastChargeLevel;
pthread_t clockThread;
pthread_mutex_t lock;

#endif
