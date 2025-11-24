#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_timer.h>
#include <unistd.h>


#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mouse.h>
#include <SDL/SDL_stdinc.h>
#include <SDL/SDL_video.h>
#include "../headers/screen.h"

#if defined TARGET_OD || defined TARGET_OD_BETA
#include <shake.h>
#endif

#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/SDL_rotozoom.h"
#include "../headers/logic.h"
#include "../headers/graphics.h"
#include "../headers/globals.h"
#include "../headers/utils.h"

SDL_Color make_color(Uint8 r, Uint8 g, Uint8 b) {
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.unused = 1;
	return c;
}

SDL_Surface * getBlendedText(TTF_Font *font, const char *text, int txtColor[]) {
	return TTF_RenderUTF8_Blended(font, text, make_color(txtColor[0], txtColor[1], txtColor[2]));
}

int calculateProportionalSizeOrDistance1(int number) {
	if(SCREEN_RATIO>=1.33&&SCREEN_RATIO<=1.34)
		return ((float)SCREEN_HEIGHT*(float)number)/240;
	else {
		return number;
	}
}

int drawTextOnScreenMaxWidth(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int backgroundColor[], int shaded, int textWidth) {
	SDL_Surface *msg;
	SDL_Surface *msg1 = malloc(sizeof(msg));
	char *bufCopy=malloc(strlen(buf)+1);
	strcpy(bufCopy,buf);

	int len=strlen(buf);
	int width = MAGIC_NUMBER;

	int retW = textWidth;
	int retH = 1;

	if(currentState == BROWSING_GAME_LIST
			&& newspaperMode
			&& y >= (currentH)) {
		width = SCREEN_WIDTH-(gameListX*2);
	}

	while (retW>width) {
		bufCopy[len]='\0';
		char *bufCopy1=strdup(bufCopy);
		len--;
		TTF_SizeUTF8(font, (const char *) bufCopy1, &retW, &retH);
		free(bufCopy1);
	}
	if(retH==1) {
		TTF_SizeUTF8(font, (const char *) bufCopy, &retW, &retH);
	}
	if (shaded) {
		drawRectangleToScreen(width+((SCREEN_WIDTH*3)/640)*2, retH, gameListX-((SCREEN_WIDTH*3)/640), y, backgroundColor);
		if (currentState==BROWSING_GAME_LIST  && outline != NULL && fontOutline > 0) {
			msg1 = TTF_RenderUTF8_Blended(outline, bufCopy, make_color(50, 50, 50));
			msg = TTF_RenderUTF8_Solid(font, bufCopy, make_color(txtColor[0], txtColor[1], txtColor[2]));
		} else {
			msg = TTF_RenderUTF8_Blended(font, bufCopy, make_color(txtColor[0], txtColor[1], txtColor[2]));
		}
	} else {
		if (currentState==BROWSING_GAME_LIST && outline != NULL && fontOutline > 0) {
			msg1 = TTF_RenderUTF8_Blended(outline, bufCopy, make_color(50, 50, 50));
			msg = TTF_RenderUTF8_Solid(font, bufCopy, make_color(txtColor[0], txtColor[1], txtColor[2]));
		} else {
			msg = TTF_RenderUTF8_Blended(font, bufCopy, make_color(txtColor[0], txtColor[1], txtColor[2]));
		}
	}

	if (align & HAlignCenter) {
		x -= msg->w / 2;
	} else if (align & HAlignRight) {
		x -= msg->w;
	}
	if (align & VAlignMiddle) {
		y -= msg->h / 2;
	} else if (align & VAlignTop) {
		y -= msg->h;
	}
	SDL_Rect rect2;
	rect2.x = x;
	rect2.y = y;
	rect2.w = msg->w;
	rect2.h = msg->h;

	if(currentState==BROWSING_GAME_LIST && outline != NULL && fontOutline > 0) {
		SDL_Rect rect = {fontOutline, fontOutline, msg1->w, msg1->h};
		SDL_BlitSurface(msg, NULL, msg1, &rect);
		SDL_BlitSurface(msg1, NULL, screen, &rect2);
		SDL_FreeSurface(msg1);
	} else {
		SDL_BlitSurface(msg, NULL, screen, &rect2);
	}
	SDL_FreeSurface(msg);
	free(bufCopy);
	return 1;
}

int drawTextOnScreen(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int backgroundColor[], int shaded) {
	SDL_Surface *msg;
	SDL_Surface *msg1 = malloc(sizeof(msg));

	int width;

	if(currentState == BROWSING_GAME_LIST
			&& newspaperMode
			&& y >= (currentH)) {
		width = SCREEN_WIDTH-(gameListX*2);
	} else {
		width = gameListWidth;
	}

	if (shaded) {
		int retW = 1;
		int retH = 1;
		TTF_SizeUTF8(font, (const char *) buf, &retW, &retH);
		drawRectangleToScreen(width+((SCREEN_WIDTH*3)/640)*2, retH, gameListX-((SCREEN_WIDTH*3)/640), y, backgroundColor);
		if (currentState==BROWSING_GAME_LIST  && outline != NULL && fontOutline > 0) {
			msg1 = TTF_RenderUTF8_Blended(outline, buf, make_color(50, 50, 50));
			msg = TTF_RenderUTF8_Solid(font, buf, make_color(txtColor[0], txtColor[1], txtColor[2]));
		} else {
			msg = TTF_RenderUTF8_Blended(font, buf, make_color(txtColor[0], txtColor[1], txtColor[2]));
		}
	} else {
		if (currentState==BROWSING_GAME_LIST && outline != NULL && fontOutline > 0) {
			msg1 = TTF_RenderUTF8_Blended(outline, buf, make_color(50, 50, 50));
			msg = TTF_RenderUTF8_Solid(font, buf, make_color(txtColor[0], txtColor[1], txtColor[2]));
		} else {
			msg = TTF_RenderUTF8_Blended(font, buf, make_color(txtColor[0], txtColor[1], txtColor[2]));
		}
	}

	if (align & HAlignCenter) {
		x -= msg->w / 2;
	} else if (align & HAlignRight) {
		x -= msg->w;
	}
	if (align & VAlignMiddle) {
		y -= msg->h / 2;
	} else if (align & VAlignTop) {
		y -= msg->h;
	}
	SDL_Rect rect2;
	rect2.x = x;
	rect2.y = y;
	rect2.w = msg->w;
	rect2.h = msg->h;

	if(currentState==BROWSING_GAME_LIST && outline != NULL && fontOutline > 0) {
		SDL_Rect rect = {fontOutline, fontOutline, msg1->w, msg1->h};
		SDL_BlitSurface(msg, NULL, msg1, &rect);
		SDL_BlitSurface(msg1, NULL, screen, &rect2);
		SDL_FreeSurface(msg1);
	} else {
		SDL_BlitSurface(msg, NULL, screen, &rect2);
	}
	SDL_FreeSurface(msg);
	return 1;
}


void genericDrawMultiLineTextOnScreen(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int maxWidth, int lineSeparation) {
	SDL_Surface *msg;
	char *bufCopy;
	char *wordsInBuf[500];
	char *ptr = NULL;

	bufCopy = strdup(buf);

	ptr = strtok(bufCopy," ");
	int wordCounter = -1;
	while(ptr!=NULL) {
		wordCounter++;
		wordsInBuf[wordCounter]=strdup(ptr);
		ptr = strtok(NULL," ");
	}
	free (bufCopy);
	int printCounter = 0;
	char *test=NULL;
	if(wordCounter>0) {
		while(printCounter<wordCounter) {
			test=malloc(500);
			strcpy(test,wordsInBuf[printCounter]);
			if (printCounter>0) {
				SDL_FreeSurface(msg);
			}
			msg = TTF_RenderUTF8_Blended(font, test, make_color(txtColor[0], txtColor[1], txtColor[2]));
			while (msg->w<=maxWidth&&printCounter<wordCounter) {
				printCounter++;
				if (strcmp(wordsInBuf[printCounter],"-")!=0) {
					strcat(test," ");
					strcat(test,wordsInBuf[printCounter]);
					SDL_FreeSurface(msg);
					msg = TTF_RenderUTF8_Blended(font, test, make_color(txtColor[0], txtColor[1], txtColor[2]));
				} else {
					printCounter++;
					break;
				}
			}
			if (msg->w>maxWidth) {
				test[strlen(test)-strlen(wordsInBuf[printCounter])]='\0';
			}
			drawTextOnScreen(font,outline,x,y,test,txtColor,align,NULL,0);
			free(test);
			y+=lineSeparation;
		}
		if (printCounter==wordCounter) {
			y-=lineSeparation;
			if (msg->w>maxWidth) {
				drawTextOnScreen(font,outline,x,y+lineSeparation,wordsInBuf[printCounter],txtColor,align,NULL,0);
			}
		}
		SDL_FreeSurface(msg);
		for (int i=0;i<=wordCounter;i++) {
			free(wordsInBuf[i]);
		}

	} else {
		drawTextOnScreen(font,outline,x,y,buf,txtColor,align,NULL,0);
		free(wordsInBuf[0]);
	}
}

SDL_Rect drawRectangleToScreen(int width, int height, int x, int y, int rgbColor[]) {
	char temp[500];
	snprintf(temp,sizeof(temp),"%d - %d - %d - %d - {%d,%d,%d}", width, height, x, y, rgbColor[0], rgbColor[1], rgbColor[2]);
	logMessage("INFO","drawRectangleToScreen",temp);
	SDL_Rect rectangle;
	rectangle.w = width;
	rectangle.h = height;
	rectangle.x = x;
	rectangle.y = y;
	logMessage("INFO","drawRectangleToScreen","Filling");
	SDL_FillRect(screen, &rectangle, SDL_MapRGB(screen->format, rgbColor[0], rgbColor[1], rgbColor[2]));
	return(rectangle);
}

SDL_Surface *loadImage (char *fileName) {
	SDL_Surface *img = NULL;
	SDL_Surface *_img = IMG_Load(fileName);
	if (_img!=NULL) {
		img = SDL_DisplayFormatAlpha(_img);
		SDL_FreeSurface(_img);
	}
	return(img);
}

void displayBackGroundImage(char *fileName, SDL_Surface *surface) {
	SDL_Surface *img = loadImage (fileName);
	int rgbColor[]={0, 0, 0};
	SDL_Rect bgrect = drawRectangleToScreen(img->w, img->h, (SCREEN_WIDTH/2)-(img->w/2),(SCREEN_HEIGHT/2)-(img->h/2), rgbColor);
	SDL_BlitSurface(img, NULL, surface, &bgrect);
	SDL_FreeSurface(img);
}

void drawTransparentRectangleToScreen(int w, int h, int x, int y, int rgbColor[], int opacity) {
	SDL_Surface *transparentrectangle;
	SDL_Rect rectangleOrig;
	SDL_Rect rectangleDest;
	rectangleOrig.w = w;
	rectangleOrig.h = h;
	rectangleOrig.x = 0;
	rectangleOrig.y = 0;
	rectangleDest.w = w;
	rectangleDest.h = h;
	rectangleDest.x = x;
	rectangleDest.y = y;
	transparentrectangle = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16, 0, 0, 0, 0);
	SDL_FillRect(transparentrectangle, &rectangleOrig, SDL_MapRGB(transparentrectangle->format, rgbColor[0], rgbColor[1], rgbColor[2]));
	SDL_SetAlpha(transparentrectangle, SDL_SRCALPHA, opacity);
	SDL_BlitSurface(transparentrectangle, &rectangleOrig, screen, &rectangleDest);
	SDL_FreeSurface(transparentrectangle);
}

int drawImage(SDL_Surface* display, SDL_Surface *image, int x, int y, int xx, int yy , const double newwidth, const double newheight, int transparent, int smoothing) {
	double zoomx = newwidth  / (float)image->w;
	double zoomy = newheight / (float)image->h;
	SDL_Surface* sized = NULL;
	if (((int)newwidth<(int)(image->w/2))&&(int)(image->w/2)%(int)newwidth==0) {
		zoomx = (float)image->w/newwidth;
		zoomy = (float)image->h/newheight;
		sized = shrinkSurface(image, zoomx, zoomy);
	} else {
		zoomx = newwidth  / (float)image->w;
		zoomy = newheight / (float)image->h;
		sized = zoomSurface(image, zoomx, zoomy, smoothing);
	}
	if( image->flags & SDL_SRCCOLORKEY ) {
		Uint32 colorkey = image->format->colorkey;
		SDL_SetColorKey( sized, SDL_SRCCOLORKEY, colorkey );
	}
	SDL_FreeSurface(image);
	image =  sized;
	SDL_Rect src, dest;
	src.x = xx; src.y = yy; src.w = image->w; src.h = image->h;
	dest.x =  x; dest.y = y; dest.w = image->w; dest.h = image->h;
	if(transparent == 1 ) {
		SDL_SetColorKey(image,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(image->format,0x0,0x0,0x0));
	}
	SDL_BlitSurface(image, &src, display, &dest);
	SDL_FreeSurface(image);
	return 1;
}

SDL_Surface *resizeSurfaceToScreenSize(SDL_Surface *surface) {
	if (surface==NULL) {
		logMessage("WARN","resizeSurface","Image not found, surface can't be resized");
		return NULL;
	}
	int newW = SCREEN_WIDTH;
	int newH = SCREEN_HEIGHT;
	if (newW==surface->w&&newH==surface->h) {
		return surface;
	}
	int smoothing = 0;
	double zoomx = (double)(newW / (double)surface->w);
	double zoomy = (double)(newH / (double)surface->h);

	if ((surface->w!=SCREEN_WIDTH || surface->h!=SCREEN_HEIGHT) && !(SCREEN_WIDTH%surface->w==0 && SCREEN_HEIGHT%surface->h==0)) {
		if(SCREEN_WIDTH==320) {
			smoothing=1;
		}
	}

	SDL_Surface *sized = NULL;
	sized = zoomSurface(surface, zoomx, zoomy, smoothing);
	if(surface->flags & SDL_SRCCOLORKEY ) {
		Uint32 colorkey = surface->format->colorkey;
		SDL_SetColorKey(sized, SDL_SRCCOLORKEY, colorkey);
	}
	SDL_FreeSurface(surface);
	surface=NULL;
	return sized;
}

SDL_Surface *resizeSurface(SDL_Surface *surface, int w, int h) {
	if (surface==NULL) {
		logMessage("WARN","resizeSurface","Image not found, surface can't be resized");
		return NULL;
	}
	int newW = (float)w;
	int newH = (float)h;
	if (newW==surface->w&&newH==surface->h) {
		return surface;
	}
	int smoothing = 0;
	if ((surface->w!=w || surface->h!=h) && !(w%surface->w==0 && h%surface->h==0)) {
		if(SCREEN_WIDTH==320) {
			smoothing=1;
		}
	}
	double zoomx = (float)(newW / (float)surface->w);
	double zoomy = (float)(newH / (float)surface->h);

	SDL_Surface *sized = NULL;
	sized = zoomSurface(surface, zoomx, zoomy, smoothing);
	if(surface->flags & SDL_SRCCOLORKEY ) {
		Uint32 colorkey = surface->format->colorkey;
		SDL_SetColorKey(sized, SDL_SRCCOLORKEY, colorkey);
	}
	SDL_FreeSurface(surface);
	surface=NULL;
	return sized;
}

void displayCenteredSurface(SDL_Surface *surface) {
	if(surface==NULL) {
		logMessage("WARN","displayCenteredSurface","Image not found, surface can't be displayed");
		return;
	}
	SDL_Rect rectangleDest;
	rectangleDest.w = 0;
	rectangleDest.h = 0;
	rectangleDest.x = SCREEN_WIDTH/2-surface->w/2;
	rectangleDest.y = ((SCREEN_HEIGHT)/2-surface->h/2);
	SDL_BlitSurface(surface, NULL, screen, &rectangleDest);
	logMessage("INFO","displayCenteredSurface","Displayed surface");
}

void displaySurface(SDL_Surface *surface, int x, int y) {
	if(surface==NULL) {
		logMessage("WARN","displaySurface","Image not found, surface can't be displayed");
		return;
	}
	SDL_Rect rectangleDest;
	rectangleDest.w = 0;
	rectangleDest.h = 0;
	rectangleDest.x = x;
	rectangleDest.y = y;
	SDL_BlitSurface(surface, NULL, screen, &rectangleDest);
}

int displayCenteredImageOnScreen(char *fileName, char *fallBackText, int scaleToFullScreen, int keepRatio) {
	SDL_Surface *img = IMG_Load(fileName);
	if (img==NULL) {
		if (strlen(fallBackText)>1) {
			return -1;
		}
	} else {
		if (!scaleToFullScreen&&(img->h==SCREEN_HEIGHT || img->w==SCREEN_WIDTH)) {
			SDL_Rect rectangleDest;
			rectangleDest.w = 0;
			rectangleDest.h = 0;
			rectangleDest.x = SCREEN_WIDTH/2-img->w/2;
			rectangleDest.y = ((SCREEN_HEIGHT)/2-img->h/2);
			SDL_BlitSurface(img, NULL, screen, &rectangleDest);
			SDL_FreeSurface(img);
		} else {
			if(img->h==SCREEN_HEIGHT && img->w==SCREEN_WIDTH) {
				SDL_Rect rectangleDest;
				rectangleDest.w = 0;
				rectangleDest.h = 0;
				rectangleDest.x = SCREEN_WIDTH/2-img->w/2;
				rectangleDest.y = ((SCREEN_HEIGHT)/2-img->h/2);
				SDL_BlitSurface(img, NULL, screen, &rectangleDest);
				SDL_FreeSurface(img);
			} else {
				double w = img->w;
				double h = img->h;
				double ratio = 0;
				int smoothing = 0;
				ratio = w / h;
				h = SCREEN_HEIGHT;
				w = h*ratio;
				if (w>SCREEN_WIDTH) {
					ratio = h / w;
					w = SCREEN_WIDTH;
					h = w*ratio;
				}
				if (!keepRatio) {
					w=SCREEN_WIDTH;
				}
				if ((int)h!=(int)img->h) {
					if(SCREEN_WIDTH==320) {
						smoothing=1;
					}
				}
				drawImage(screen, img, SCREEN_WIDTH/2-w/2, SCREEN_HEIGHT/2-h/2, 0, 0, w, h, 0, smoothing);
			}
		}
	}
	return 0;
}

void initializeDisplay(int w, int h) {
	int depth=16;
#if defined(TARGET_PC) || defined(MIYOOMINI)
	Uint32 pcflags = SDL_HWSURFACE|SDL_NOFRAME;
#else
	Uint32 flags = SDL_SWSURFACE|SDL_NOFRAME;
#endif
	SDL_ShowCursor(0);
	logMessage("INFO","initializeDisplay","well...");
	setenv("SDL_FBCON_DONT_CLEAR", "1", 0);
	logMessage("INFO","initializeDisplay","maybe...");
#ifdef TARGET_OD
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	screen = SDL_SetVideoMode(320, 240, depth, flags);
	SDL_FreeSurface(screen);
	SDL_Quit();
#endif
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
#ifndef MIYOOMINI
	char * line = NULL;
	size_t len = 0;
	FILE *fpHDMI = fopen("/sys/class/hdmi/hdmi","r");
	ssize_t read;
	if (fpHDMI!=NULL) {
		read = getline(&line, &len, fpHDMI);
		hdmiEnabled = atoi(line);
		fclose(fpHDMI);
		if (read!=-1) {
			free(line);
		}
	}

	hdmiChanged = hdmiEnabled;
	if (hdmiEnabled) {
		SCREEN_WIDTH = HDMI_WIDTH;
		SCREEN_HEIGHT = HDMI_HEIGHT;
	}
	SCREEN_RATIO = (double)SCREEN_WIDTH/SCREEN_HEIGHT;
#endif


#ifdef TARGET_RFW
	//	ipu modes (/proc/jz/ipu):
	//	0: stretch
	//	1: aspect
	//	2: original (fallback to aspect when downscale is needed)
	//	3: 4:3
	FILE *fp;
	fp = fopen("/proc/jz/ipu","w");
	fprintf(fp,"0");
	fclose(fp);
#endif
#if defined(TARGET_PC) || defined(MIYOOMINI)
	SCREEN_HEIGHT = h;
	SCREEN_WIDTH = w;
	char msg[1000];
	snprintf(msg,1000,"%dx%d",SCREEN_WIDTH,SCREEN_HEIGHT);
	logMessage("INFO", "initializeDisplay", msg);
	SCREEN_RATIO = (double)SCREEN_WIDTH/SCREEN_HEIGHT;
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, depth, pcflags);
	#if defined MIYOOMINI
	if (loadingScreenEnabled) {
    char tempString[1000];
    snprintf(tempString, sizeof(tempString), "%s/.simplemenu/resources/loading.png", getenv("HOME"));
    SDL_Surface* image;
    image = IMG_Load(tempString);
    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_Flip(screen);
    SDL_FreeSurface(image);
    image = NULL;
	SDL_Delay(1000);
	} else {
	SDL_Flip(screen);
	}
	#else
	char tempString[1000];
    snprintf(tempString, sizeof(tempString), "%s/.simplemenu/resources/loading.png", getenv("HOME"));
    SDL_Surface* image;
    image = IMG_Load(tempString);
    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_Flip(screen);
    SDL_FreeSurface(image);
    image = NULL;
	#endif
#else
	char res[20];
	sprintf(res, "resolution: %dx%d", w, h);
	logMessage("INFO" , "initializeDisplay", res);
	FILE *fp1;
	SDL_ShowCursor(0);

	SCREEN_WIDTH=320;
	SCREEN_HEIGHT=240;

	SDL_Rect** modes = SDL_ListModes(NULL,SDL_NOFRAME|SDL_SWSURFACE);

	fp1 = fopen("/sys/class/graphics/fb0/device/allow_downscaling","w");
	if (fp1!=NULL) {
		fprintf(fp1, "%d" , 0);
		fclose(fp1);
	}

#if defined TARGET_OD || defined TARGET_OD_BETA
	if(modes==(SDL_Rect **)0) {
		logMessage("INFO", "initializeDisplay", "No available modes");
	} else if(modes==(SDL_Rect **)-1) {
		logMessage("INFO", "initializeDisplay", "All modes available");
	} else {
		logMessage("INFO", "initializeDisplay", "Some Available modes");
		for(int i=0; modes[i]; i++) {
			if (modes[i]->w==640 && modes[i]->h ==480) {
				SCREEN_WIDTH=640;
				SCREEN_HEIGHT=480;
				break;
			}
		}
	}
#endif
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, depth, flags);
	if (screen==NULL) {
		SCREEN_WIDTH=320;
		SCREEN_HEIGHT=240;
		screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, depth, flags);
	}

#endif
	MAGIC_NUMBER = SCREEN_WIDTH-calculateProportionalSizeOrDistance1(2);
	logMessage("INFO","initializeDisplay","Initialized Display");
	SCREEN_RATIO = (double)SCREEN_WIDTH/SCREEN_HEIGHT;
}

void getTextWidth(TTF_Font *font, const char *text, int *widthToBeSet){
        TTF_SizeUTF8(font, text, widthToBeSet, NULL);
}

void refreshScreen() {
	SDL_Flip(screen);
}
