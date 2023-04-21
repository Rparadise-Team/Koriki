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

#define	BUTTON_A	SDLK_SPACE
#define	BUTTON_B	SDLK_LCTRL
#define	BUTTON_X	SDLK_LSHIFT
#define	BUTTON_Y	SDLK_LALT
#define	BUTTON_START	SDLK_RETURN
#define	BUTTON_SELECT	SDLK_RCTRL
#define	BUTTON_MENU	SDLK_ESCAPE
#define	BUTTON_L2	SDLK_TAB
#define	BUTTON_R2	SDLK_BACKSPACE
#define BUTTON_RIGHT SDLK_RIGHT
#define BUTTON_LEFT SDLK_LEFT

#define CONSOLA "gb" //a cambiar por consola
#define CORE "Gambatte" //a cambiar por emulador
#define TEXTO1 "video_dingux_ipu_keep_aspect"
#define TEXTO2 "video_scale_integer"
#define TEXTO3 "custom_viewport_height"
#define TEXTO4 "input_overlay"

#define VALOR1 "False"
#define VALOR2 "True"
#define VALOR3 ":/.retroarch/overlay/ATC/ATC-GB.cfg" //a cambiar por consola
#define VALOR4 "" // sin overlay

SDL_Surface* screen = NULL;
SDL_Surface* overlayImage = NULL;
char img_path[256] = "./overlay/";
const char* img_name = "1.png";
const char* text_values[4] = {TEXTO1, TEXTO2, TEXTO3, TEXTO4};
const char* text_bools[4] = {VALOR1, VALOR2, VALOR3, VALOR4};
const char* text_selected_values[3][4] = {
    {VALOR2, VALOR1, "576", VALOR4},// aspect ration
    {VALOR2, VALOR2, "576", VALOR3},// overlay
    {VALOR1, VALOR1, "576", VALOR4}// fullscreen
};

void update_overlay(char* img_name) {
    char opt_path[256] = "/mnt/SDCARD/RetroArch/.retroarch/config/" CORE "/" CONSOLA ".cfg";
    char opt_new_path[256] = CONSOLA".cfg.new";
    FILE *opt_file, *opt_new_file;
    opt_file = fopen(opt_path, "r");
    opt_new_file = fopen(opt_new_path, "w");

    if (opt_file == NULL || opt_new_file == NULL) {
        printf("Error al abrir el archivo de configuracion\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), opt_file)) {
        int i;
        for (i = 0; i < 5; i++) {
            const char* text_value = text_selected_values[atoi(img_name)-1][i];
            if (strstr(line, text_values[i]) != NULL) {
                fprintf(opt_new_file, "%s=%s\n", text_values[i], text_value);
                break;
            }
        }
        if (i == 5) {
            fprintf(opt_new_file, "%s", line);
        }
    }
    fclose(opt_file);
    fclose(opt_new_file);
    remove(opt_path);
    rename(opt_new_path, opt_path);
}
						   
int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        printf("Error initializing SDL.\n");
        return 1;
    }

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    if (screen == NULL) {
        printf("Error setting SDL video mode.\n");
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == -1) {
        printf("Error initializing SDL_image.\n");
        return 1;
    }

    int quit = 0;
    SDL_Event event;
    int selected_image_index = 0;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_LEFT) {
                        selected_image_index--;
                        if (selected_image_index < 0) {
                            selected_image_index = 2;
                        }
                        char new_img_name[16];
                        sprintf(new_img_name, "%d.png", selected_image_index+1);
                        update_overlay(new_img_name);
                        strcpy(img_path, "");
                        strcat(img_path, "./overlay/");
                        strcat(img_path, new_img_name);
                    } else if (event.key.keysym.sym == SDLK_RIGHT) {
                        selected_image_index++;
                        if (selected_image_index > 2) {
                            selected_image_index = 0;
                        }
                        char new_img_name[16];
                        sprintf(new_img_name, "%d.png", selected_image_index+1);
                        update_overlay(new_img_name);
                        strcpy(img_path, "");
                        strcat(img_path, "./overlay/");
                        strcat(img_path, new_img_name);
                    } else if (event.key.keysym.sym == SDLK_SPACE) {
                        char new_img_name[16];
                        sprintf(new_img_name, "%d.png", selected_image_index+1);
                        update_overlay(new_img_name);
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
						quit = 1;
					} else if (event.key.keysym.sym == SDLK_LCTRL) {
						quit = 1;
					}
                    break;
            }
        }

        overlayImage = IMG_Load(img_path);
        if (overlayImage == NULL) {
            printf("Error loading overlay image.\n");
            return 1;
        }

        SDL_Rect overlayRect;
        overlayRect.x = 0;
        overlayRect.y = 0;
        SDL_BlitSurface(overlayImage, NULL, screen, &overlayRect);
        SDL_FreeSurface(overlayImage);

        SDL_Flip(screen);
    }

    SDL_Quit();

    return 0;
}