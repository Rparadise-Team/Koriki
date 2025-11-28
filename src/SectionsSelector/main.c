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
#include <sys/ioctl.h>
#include <dirent.h>
#include <limits.h>

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

#define NUM_IMAGES 2

#define SECTIONS "/mnt/SDCARD/.simplemenu/section_groups"
#define SCRIPTS "/mnt/SDCARD/.simplemenu/scripts"
#define DESTINATION "/mnt/SDCARD/.simplemenu"
#define ALPHABETIC "/mnt/SDCARD/App/SectionsSelector/sections/alphabetic/section_groups"
#define DEFAULT "/mnt/SDCARD/App/SectionsSelector/sections/default/section_groups"
#define SYSTEMS "/mnt/SDCARD/App/SectionsSelector/sections/systems/section_groups"
#define ALPHABETIC_SCRIPTS "/mnt/SDCARD/App/SectionsSelector/scripts/alphabetic/scripts"
#define DEFAULT_SCRIPTS "/mnt/SDCARD/App/SectionsSelector/scripts/default/scripts"
#define SYSTEMS_SCRIPTS "/mnt/SDCARD/App/SectionsSelector/scripts/systems/scripts"

SDL_Surface* screen = NULL;
SDL_Surface* image[NUM_IMAGES];
SDL_Surface* text_surface = NULL;
TTF_Font* font = NULL;
SDL_Rect image_rect;
SDL_Event event;
bool running = true;
int current_image = 0;

void load_image(int index) {
	char filename[16];
	sprintf(filename, "%d.png", index + 1);
	image[index] = IMG_Load(filename);
}

void copy_directory(const char* source, const char* destination) {
    char removeCommand[256];
    snprintf(removeCommand, sizeof(removeCommand), "rm -rf %s", destination);
    system(removeCommand);
	
	destination = DESTINATION;

    char copyCommand[256];
    snprintf(copyCommand, sizeof(copyCommand), "cp -R %s %s", source, destination);
    system(copyCommand);
}

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if (screen == NULL) {
		printf("Error setting video mode: %s\n", SDL_GetError());
		return -1;
	}
	for (int i = 0; i < NUM_IMAGES; i++) {
		load_image(i);
	}

	image_rect.x = (SCREEN_WIDTH - image[current_image]->w) / 2;
	image_rect.y = (SCREEN_HEIGHT - image[current_image]->h) / 2;
	image_rect.w = image[current_image]->w;
	image_rect.h = image[current_image]->h;
	
	TTF_Init();
	TTF_Font *font = TTF_OpenFont("Exo-2-Bold-Italic.ttf", 32);
	
	SDL_Color white = {255, 255, 255};
	SDL_Surface *text_surface = TTF_RenderText_Blended(font, "Done! Press B to exit!", white);
	
	int text_width = text_surface->w;
	int text_height = text_surface->h;
	
	int padding = 10;
	SDL_Surface *rect_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, text_width + padding*2, text_height + padding*2, 32, 0, 0, 0, 0);
	SDL_FillRect(rect_surface, NULL, SDL_MapRGB(screen->format, 50, 50, 50));
	SDL_Rect rect_pos;
	rect_pos.x = (SCREEN_WIDTH - rect_surface->w) / 2;
	rect_pos.y = (SCREEN_HEIGHT - rect_surface->h) / 2;
	
	SDL_Rect text_pos;
	text_pos.x = rect_pos.x + padding;
	text_pos.y = rect_pos.y + padding;
	
	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == BUTTON_RIGHT) {
						current_image = (current_image + 1) % NUM_IMAGES;
						image_rect.x = (SCREEN_WIDTH - image[current_image]->w) / 2;
						image_rect.y = (SCREEN_HEIGHT - image[current_image]->h) / 2;
						break;
					} else if (event.key.keysym.sym == BUTTON_LEFT) {
						current_image = (current_image - 1 + NUM_IMAGES) % NUM_IMAGES;
						image_rect.x = (SCREEN_WIDTH - image[current_image]->w) / 2;
						image_rect.y = (SCREEN_HEIGHT - image[current_image]->h) / 2;
						break;
					} else if (event.key.keysym.sym == BUTTON_A) {
						switch (current_image) {
							case 0:
								 //ALPHABETIC
								copy_directory(ALPHABETIC, SECTIONS);
								copy_directory(ALPHABETIC_SCRIPTS, SCRIPTS);
								system("rm /mnt/SDCARD/.simplemenu/last_state.sav");
								system("rm -f /mnt/SDCARD/.simplemenu/*_section");
								system("touch /mnt/SDCARD/.simplemenu/alphabetic_section");
								sync();
								SDL_BlitSurface(rect_surface, NULL, screen, &rect_pos);
								SDL_BlitSurface(text_surface, NULL, screen, &text_pos);
								SDL_Flip(screen);
								SDL_Delay(3000);
								break;
							case 1:
								 //SYSTEMS
								copy_directory(SYSTEMS, SECTIONS);
								copy_directory(SYSTEMS_SCRIPTS, SCRIPTS);
								system("rm /mnt/SDCARD/.simplemenu/last_state.sav");
								system("rm -f /mnt/SDCARD/.simplemenu/*_section");
								system("touch /mnt/SDCARD/.simplemenu/systems_section");
								sync();
								SDL_BlitSurface(rect_surface, NULL, screen, &rect_pos);
								SDL_BlitSurface(text_surface, NULL, screen, &text_pos);
								SDL_Flip(screen);
								SDL_Delay(3000);
								break;
						}
						break;
					} else if (event.key.keysym.sym == BUTTON_B || event.key.keysym.sym == BUTTON_MENU) {
						running = false;
						break;
					}
					break;
			}
		}
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		SDL_BlitSurface(image[current_image], NULL, screen, &image_rect);
		SDL_Flip(screen);
	}
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
