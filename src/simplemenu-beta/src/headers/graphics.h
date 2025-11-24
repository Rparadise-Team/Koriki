#ifndef GRAPHICS_DEFINED
#define GRAPHICS_DEFINED

#include <SDL/SDL_ttf.h>
#include <SDL/SDL_video.h>

static const int
HAlignLeft = 1,
HAlignRight = 2,
HAlignCenter = 4,
VAlignTop = 8,
VAlignBottom = 16,
VAlignMiddle = 32;

int drawShadedTextOnScreen(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int backgroundColor[]);
int drawTextOnScreen(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int backgroundColor[], int shaded);
int drawTextOnScreenMaxWidth(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int backgroundColor[], int shaded, int textWidth);
void drawTransparentRectangleToScreen(int w, int h, int x, int y, int rgbColor[], int transparency);
void displayBackGroundImage(char *fileName, SDL_Surface *surface);
int displayCenteredImageOnScreen(char *fileName, char *fallBackText, int scaleToFullScreen, int keepRatio);
void initializeDisplay(int w, int h);
void refreshScreen();
void genericDrawMultiLineTextOnScreen(TTF_Font *font, TTF_Font *outline, int x, int y, char *buf, int txtColor[], int align, int maxWidth, int lineSeparation);
void displayCenteredSurface(SDL_Surface *surface);
void drawGameNumber(char *buf, int x, int y);
void displaySurface(SDL_Surface *surface, int x, int y);
int drawImage(SDL_Surface* display, SDL_Surface *image, int x, int y, int xx, int yy , const double newwidth, const double newheight, int transparent, int smoothing);
int calculateProportionalSizeOrDistance1(int number);
SDL_Surface *resizeSurfaceToScreenSize(SDL_Surface *surface);
SDL_Surface *resizeSurface(SDL_Surface *surface, int w, int h);
SDL_Surface *loadImage (char *fileName);
SDL_Rect drawRectangleToScreen(int width, int height, int x, int y, int rgbColor[]);
SDL_Color make_color(Uint8 r, Uint8 g, Uint8 b);
SDL_Surface * getBlendedText(TTF_Font *font, const char *text, int color[]);
void getTextWidth(TTF_Font *font, const char *text, int *widthToBeSet);
#endif
