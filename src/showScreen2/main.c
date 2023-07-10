#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


#define FONT_OUTLINE 2


int main(int argc , char* argv[]) {
  SDL_Surface* video;
  SDL_Surface* screen;
  TTF_Font* font40;
  TTF_Font* font40_outline;
  SDL_Color color_white = {255, 255, 255, 0};
  SDL_Color color_black = {0, 0, 0, 0};

  if (argc<2 || argc>3) {
    puts("Usage: showScreen /path/image.png [version]");
    return EXIT_SUCCESS;
  }

  if (access(argv[1], F_OK) != 0) return EXIT_FAILURE;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(SDL_DISABLE);
  TTF_Init();
  video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
  screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

  SDL_Surface* img = IMG_Load(argv[1]);
  SDL_BlitSurface(img, NULL, screen, NULL);
  SDL_FreeSurface(img);

  if (argc==3) {
    font40 = TTF_OpenFont("resources/Exo-2-Bold-Italic.ttf", 40);
    font40_outline = TTF_OpenFont("resources/Exo-2-Bold-Italic.ttf", 40);
    TTF_SetFontOutline(font40_outline, FONT_OUTLINE);
    SDL_Surface* imageVersion1 = TTF_RenderUTF8_Blended(font40_outline, argv[2], color_black);
    SDL_Surface* imageVersion = TTF_RenderUTF8_Blended(font40, argv[2], color_white);
    SDL_Rect rect = {FONT_OUTLINE, FONT_OUTLINE, imageVersion->w, imageVersion->h};
    SDL_BlitSurface(imageVersion, NULL, imageVersion1, &rect);
    SDL_FreeSurface(imageVersion);
    SDL_Rect rectVersion = {520, 415, 85, 54};
    SDL_BlitSurface(imageVersion1, NULL, screen, &rectVersion);
    SDL_FreeSurface(imageVersion1);
  }

  SDL_BlitSurface(screen, NULL, video, NULL);
  SDL_Flip(video);

  SDL_FreeSurface(screen);
  SDL_FreeSurface(video);
  SDL_Quit();

  return EXIT_SUCCESS;
}
