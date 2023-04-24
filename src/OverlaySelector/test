#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <dirent.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP 32
#define BUTTON_A SDLK_SPACE
#define BUTTON_B SDLK_LCTRL
#define BUTTON_X SDLK_LSHIFT
#define BUTTON_Y SDLK_LALT
#define BUTTON_START SDLK_RETURN
#define BUTTON_SELECT SDLK_RCTRL
#define BUTTON_MENU SDLK_ESCAPE
#define BUTTON_L2 SDLK_TAB
#define BUTTON_R2 SDLK_BACKSPACE
#define BUTTON_RIGHT SDLK_RIGHT
#define BUTTON_LEFT SDLK_LEFT
#define CONSOLA "GB" //a cambiar por consola
#define CORE "Gambatte" //a cambiar por emulador
#define TEXTO1 "video_dingux_ipu_keep_aspect"
#define TEXTO2 "video_scale_integer"
#define TEXTO3 "custom_viewport_height"
#define TEXTO4 "input_overlay"
#define VALOR1 "False"
#define VALOR2 "True"
#define VALOR3 ":/.retroarch/overlay/ATC/ATC-GB.cfg" //a cambiar por consola
#define VALOR4 "" // sin overlay

void create_config_file(char *file_path, char *texto1, char *valor1, char *texto2, char *valor2, char *texto3, char *valor3, char *texto4, char *valor4) {
    FILE *file = fopen(file_path, "w");
    fprintf(file, "%s=\"%s\"\n", texto1, valor1);
    fprintf(file, "%s=\"%s\"\n", texto2, valor2);
    fprintf(file, "%s=\"%s\"\n", texto3, valor3);
    fprintf(file, "%s=\"%s\"\n", texto4, valor4);
    fclose(file);
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Surface *screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    SDL_Event event;
    SDL_Surface *images[3];
    images[0] = IMG_Load("1.png");
    images[1] = IMG_Load("2.png");
    images[2] = IMG_Load("3.png");
    int current_image = 0;
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = SCREEN_WIDTH;
    rect.h = SCREEN_HEIGHT;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case BUTTON_LEFT:
                        current_image = (current_image + 2) % 3;
                        break;
                    case BUTTON_RIGHT:
                        current_image = (current_image + 1) % 3;
                        break;
                    case BUTTON_A:
                        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Changes applied", "Press Button B to exit", screen);
                        char *file_path = malloc(strlen("/mnt/SDCARD/RetroArch/.retroarch/config/") + strlen(CORE) + strlen("/") + strlen(CONSOLA) + strlen(".cfg") + 1);
                        sprintf(file_path, "/mnt/SDCARD/RetroArch/.retroarch/config/%s/%s.cfg", CORE, CONSOLA);
                        char *texto1 = TEXTO1;
                        char *texto2 = TEXTO2;
                        char *texto3 = TEXTO3;
                        char *texto4 = TEXTO4;
                        char *valor1;
                        char *valor2;
                        char *valor3;
                        char *valor4;
                        switch (current_image) {
                            case 0:
                                valor1 = VALOR2;
                                valor2 = VALOR1;
                                valor3 = "576";
                                valor4 = VALOR4;
                                break;
                            case 1:
                                valor1 = VALOR2;
                                valor2 = VALOR2;
                                valor3 = "576";
                                valor4 = VALOR3;
                                break;
                            case 2:
                                valor1 = VALOR1;
                                valor2 = VALOR1;
                                valor3 = "576";
                                valor4 = VALOR4;
                                break;
                        }
                        create_config_file(file_path, texto1, valor1, texto2, valor2, texto3, valor3, texto4, valor4);
                        free(file_path);
                        break;
                    case BUTTON_B:
                    case BUTTON_MENU:
                        running = false;
                        break;
                }
            }
        }
        SDL_BlitSurface(images[current_image], NULL, screen, &rect);
        TTF_Init();
        TTF_Font *font = TTF_OpenFont("arial.ttf", 16);
        SDL_Color black = {0, 0, 0};
        SDL_Surface *text_surface = TTF_RenderText_Blended(font, "Press Button A for select or Button B for exit.", black);
        SDL_Rect text_rect;
        text_rect.x = 0;
        text_rect.y = SCREEN_HEIGHT - text_surface->h;
        text_rect.w = SCREEN_WIDTH;
        text_rect.h = text_surface->h;
