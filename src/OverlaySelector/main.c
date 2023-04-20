#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "sys/ioctl.h"
#include <dirent.h>

#define CONSOLE			"GBA"
#define IMAGE1			"gba1"
#define IMAGE2			"gba2"
#define IMAGE3			"gba3"
#define CONFIG_FILE		"gba.opt"

// Max number of records in data structure
#define NUMBER_OF_BS 200
#define MAX_BS_NAME_SIZE 256
#define FONT_OUTLINE 2

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

#define OV_PATH "overlays"
//#ifdef PLATFORM_PC
//#define BOOTSCREEN_PATH "."
//#else
//#define BOOTSCREEN_PATH "/mnt/SDCARD/.tmp_update/res/bootScreen.png"    //Onion
//#define BOOTSCREEN_PATH "/mnt/SDCARD/Koriki/images/bootScreen.png"    //Koriki
//#endif


// Global vars
char bss[NUMBER_OF_BS][MAX_BS_NAME_SIZE];
SDL_Surface* video;
SDL_Surface* screen;
TTF_Font* font40;
TTF_Font* font40_outline;
SDL_Surface* surfaceBSS;
SDL_Surface* surfaceName;
SDL_Surface* surfaceName1;
SDL_Surface* imagePages;
SDL_Surface* imagePages1;
SDL_Surface* surfaceArrowLeft;
SDL_Surface* surfaceArrowRight;
SDL_Rect rectArrowLeft = {22, 222, 32, 36};
SDL_Rect rectArrowRight = {586, 222, 32, 36};
SDL_Rect rectName;
SDL_Rect rectPages = {500, 425, 85, 54};
SDL_Color color_white = {255, 255, 255, 0};
SDL_Color color_black = {0, 0, 0, 0};
int nCurrentPage = 0;
int levelPage = 0;
int bsCount = 0;

// Función para obtener la imagen seleccionada
const char* get_selected_image() {
    if (strcmp(IMAGE1, "overlay.png") == 0) {
        return IMAGE1;
    } else if (strcmp(IMAGE2 , "overlay.png") == 0) {
        return IMAGE2;
    } else if (strcmp(IMAGE3, "overlay.png") == 0) {
        return IMAGE3;
    } else {
        return NULL;
    }
}

void update_opt_file(const char* filename, const char* key, const char* value) {
    char line[1000];
    char key_value[100];

    // Abrir el archivo de configuración
    FILE* fp = fopen(filename, "r+");

    // Leer línea por línea y buscar la clave a modificar
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, key) != NULL) {
            // Encontrado, modificar el valor y escribirlo en el archivo
            snprintf(key_value, sizeof(key_value), "%s=%s\n", key, value);
            fseek(fp, -strlen(line), SEEK_CUR);
            fputs(key_value, fp);
            break;
        }
    }

    // Cerrar el archivo
    fclose(fp);
}

bool file_exists (char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int alphasort_no_case(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

// Draw the overlay #nbs
void showBS(int nbs) {
    char cOVPath[250];
    char cName[250];
    char cPages[10];

    sprintf(cOVPath, OV_PATH"/%s/overlay.png", bss[nbs]);
    surfaceBSS = IMG_Load(cOVPath);
    SDL_BlitSurface(surfaceBSS, NULL, screen, NULL);
    SDL_FreeSurface(surfaceBSS);
    if (nbs != 0) {
        SDL_BlitSurface(surfaceArrowLeft, NULL, screen, &rectArrowLeft);
    }
    if (nbs != (bsCount-1)) {
        SDL_BlitSurface(surfaceArrowRight, NULL, screen, &rectArrowRight);
    }
    sprintf(cName, "%s", bss[nbs]);
    surfaceName1 = TTF_RenderUTF8_Blended(font40_outline, cName, color_black);
    surfaceName = TTF_RenderUTF8_Blended(font40, cName, color_white);
    SDL_Rect rect = {FONT_OUTLINE, FONT_OUTLINE, surfaceName->w, surfaceName->h};
    SDL_BlitSurface(surfaceName, NULL, surfaceName1, &rect);
    SDL_FreeSurface(surfaceName);
    rectName = {320 - surfaceName1->w/2, 5, surfaceName1->w, surfaceName1->h};
    SDL_BlitSurface(surfaceName1, NULL, screen, &rectName);
    SDL_FreeSurface(surfaceName1);

    sprintf(cPages, "%d/%d", (nbs+1), bsCount);
    imagePages1 = TTF_RenderUTF8_Blended(font40_outline, cPages, color_black);
    imagePages = TTF_RenderUTF8_Blended(font40, cPages, color_white);
    rect = {FONT_OUTLINE, FONT_OUTLINE, imagePages->w, imagePages->h};
    SDL_BlitSurface(imagePages, NULL, imagePages1, &rect);
    SDL_FreeSurface(imagePages);
    SDL_BlitSurface(imagePages1, NULL, screen, &rectPages);
    SDL_FreeSurface(imagePages1);
}


int main(void) {
    int running = 1;
    char cOVPath[250];

    struct dirent **files;
    int n = scandir("overlays", &files, NULL, alphasort_no_case);
    if (n < 0) {
        perror("Couldn't open the directory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        struct dirent *ent = files[i];
        if (ent->d_type == DT_DIR)  {
            sprintf(cOVPath, "overlays/%s/overlay.png", ent->d_name);
            if (file_exists(cOVPath) == 1) {
                strcpy(bss[bsCount], ent->d_name);
                bsCount ++;
            }
        }
        free(files[i]);
    }
    free(files);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();
    video = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    surfaceArrowLeft = IMG_Load("resources/arrowLeft.png");
    surfaceArrowRight = IMG_Load("resources/arrowRight.png");
    font40 = TTF_OpenFont("resources/Exo-2-Bold-Italic.ttf", 40);
    font40_outline = TTF_OpenFont("resources/Exo-2-Bold-Italic.ttf", 40);
    TTF_SetFontOutline(font40_outline, FONT_OUTLINE);

    showBS(nCurrentPage);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (((int)event.key.keysym.sym) == BUTTON_B) {
                    if (levelPage==0) {
                        //exit program
                        running = 0;
                    } else {
                        levelPage = 0;
                    }
                } else if (((int)event.key.keysym.sym) == BUTTON_A) {
                    if (levelPage == 1) {
                        // Install overlay
                        const char* selected_image = get_selected_image();
						if (selected_image != NULL) {
							switch (selected_image) {
								case IMAGE1:
									update_opt_file(CONFIG_FILE, "video_filtre", "False");
									break;
								case IMAGE2:
									update_opt_file(CONFIG_FILE, "video_filtre", "True");
									break;
								case IMAGE3:
									update_opt_file(CONFIG_FILE, "video_filtre", "Null");
									break;
								default:
									printf("Imagen no válida.\n");
									break;
							}
						} else {
							printf("Imagen no seleccionada.\n");
						}
                        running = 0;
                    } else {
                        levelPage = 1;
                    }
                } else if (((int)event.key.keysym.sym) == BUTTON_RIGHT) {
                    if (nCurrentPage < (bsCount-1)){
                        nCurrentPage ++;
                    }
                } else if (((int)event.key.keysym.sym) == BUTTON_LEFT) {
                    if (nCurrentPage > 0){
                        nCurrentPage --;
                    }
                }
            } else if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Show BS #nCurrentPage
            showBS(nCurrentPage);
            if (levelPage == 1) {
                // Show BS #nCurrentPage with confirmation alert
                surfaceBSS = IMG_Load("resources/confirm.png");
                SDL_BlitSurface(surfaceBSS, NULL, screen, NULL);
                SDL_FreeSurface(surfaceBSS);
            }

        }
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }

    SDL_FreeSurface(surfaceArrowLeft);
    SDL_FreeSurface(surfaceArrowRight);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(video);
    SDL_Quit();

    return EXIT_SUCCESS;
}
