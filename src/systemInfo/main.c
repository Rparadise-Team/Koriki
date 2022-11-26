#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>


char* load_file(char const* path) {
  char* buffer = 0;
  long length = 0;
  FILE * f = fopen(path, "rb"); //was "rb"

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char*) malloc((length+1)*sizeof(char));
    if (buffer) fread(buffer, sizeof(char), length, f);
    fclose(f);
  }
  buffer[length] = '\0';

  return buffer;
}

int main(int argc , char* argv[]) {
  SDL_Surface* video;
  SDL_Surface* screen;
  char* kor_version_str;
  SDL_Color color_white = {255, 255, 255, 0};
  int running = 1;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(SDL_DISABLE);
  TTF_Init();
  TTF_Font* font = TTF_OpenFont("resources/Pmc.ttf", 48);
  video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
  screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

  SDL_Surface* background = IMG_Load("resources/background.png");
  SDL_BlitSurface(background, NULL, screen, NULL);
  SDL_FreeSurface(background);

  kor_version_str = load_file("version.txt");
  SDL_Surface* korVersion = TTF_RenderUTF8_Blended(font, kor_version_str, color_white);
  SDL_Rect rectKorVersion = {35, 395, 565, 51};
  SDL_BlitSurface(korVersion, NULL, screen, &rectKorVersion);
  SDL_FreeSurface(korVersion);
  free(kor_version_str);

  SDL_BlitSurface(screen, NULL, video, NULL);
  SDL_Flip(video);
  
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      running = !(event.type == SDL_KEYDOWN || event.type == SDL_QUIT);
    }
  }

  SDL_FreeSurface(screen);
  SDL_FreeSurface(video);
  SDL_Quit();

  return EXIT_SUCCESS;
}
