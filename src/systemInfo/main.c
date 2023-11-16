#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>


#define BUFFER_LEN 1024
#define LINE_SPACING 40

char* load_file(char const* path) {
  char* buffer = NULL;
  long length = 0;
  FILE * f = fopen(path, "rb"); //was "rb"

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char*) malloc((length+1)*sizeof(char));
    if (buffer) fread(buffer, sizeof(char), length, f);
    fclose(f);
    buffer[length] = '\0';
  }

  return buffer;
}

void load_file2(char* buffer, char const* path) {
  FILE * f = fopen(path, "rb");
  if (f) {
    fgets(buffer, BUFFER_LEN, f);
    fclose(f);
  }
}

int main(int argc , char* argv[]) {
  SDL_Surface* video;
  SDL_Surface* screen;
  char sys_version_str[BUFFER_LEN];
  char* kor_version_str;
  SDL_Color color_white = {255, 255, 255, 0};
  int running = 1;
  long unsigned int i;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(SDL_DISABLE);
  TTF_Init();
  TTF_Font* font = TTF_OpenFont("resources/Pmc.ttf", 48);
  video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
  screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);

  SDL_Surface* background = IMG_Load("resources/background.png");
  SDL_BlitSurface(background, NULL, screen, NULL);
  SDL_FreeSurface(background);

  // System version
  FILE* cmd_output = popen("/etc/fw_printenv miyoo_version", "r");
  fgets(sys_version_str, BUFFER_LEN, cmd_output);
  pclose(cmd_output);
	
  char* version_start = strchr(sys_version_str, '=');
  if (version_start != NULL) {
      version_start++;
      sscanf(version_start, "%s", sys_version_str);
  } 
  SDL_Surface* sysVersion;
  SDL_Rect rectSysVersion;
  char prevLine[BUFFER_LEN];
  prevLine[0] = 0;
  char line[BUFFER_LEN];
  line[0] = 0;
  int w, h;
  short int y = 84;
  char delim[] = " ";
  char *word = strtok(sys_version_str, delim);
  while(word != NULL) {
    strncat(line, word, 256);
    strncat(line, " ", 1);
    TTF_SizeText(font, line, &w, &h);
    if (w > 565) {
      sysVersion = TTF_RenderUTF8_Blended(font, prevLine, color_white);
      rectSysVersion = {35, y, 565, 50};
      SDL_BlitSurface(sysVersion, NULL, screen, &rectSysVersion);
      SDL_FreeSurface(sysVersion);
      prevLine[0] = 0;
      strncpy(line, word, 256);
      strncat(line, " ", 1);
      y += LINE_SPACING;
    }
    strncpy(prevLine, line, BUFFER_LEN);
    word = strtok(NULL, delim);
  }
  for (i = 0; i <= strlen(prevLine); i++) {
    if(prevLine[i] == 10) prevLine[i] = 32;
  }
  sysVersion = TTF_RenderUTF8_Blended(font, prevLine, color_white);
  rectSysVersion = {35, y, 565, 50};
  SDL_BlitSurface(sysVersion, NULL, screen, &rectSysVersion);
  SDL_FreeSurface(sysVersion);

  // Koriki version
#if defined(PLATFORM_PC)
  kor_version_str = load_file("version.txt");
#else
  kor_version_str = load_file("/mnt/SDCARD/Koriki/version.txt");
#endif
  if (kor_version_str && strlen(kor_version_str)) {
    for (i = 0; i <= strlen(kor_version_str); i++) {
      if(kor_version_str[i] == 10) kor_version_str[i] = 32;
    }
    SDL_Surface* korVersion = TTF_RenderUTF8_Blended(font, kor_version_str, color_white);
    SDL_Rect rectKorVersion = {35, 395, 565, 51};
    SDL_BlitSurface(korVersion, NULL, screen, &rectKorVersion);
    SDL_FreeSurface(korVersion);
    free(kor_version_str);
  }

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