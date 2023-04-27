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

#define CONSOLA	"GBA"
#define CORE	"Meteor GBA"
#define BORDER	"ON"

#define NUM_IMAGES 3

#define TEXTO0	""
#define TEXTO1	"video_dingux_ipu_keep_aspect"
#define TEXTO2	"video_scale_integer"
#define TEXTO3	"custom_viewport_height"
#define TEXTO4	"input_overlay"
#define TEXTO5	"video_filter"
#define TEXTO6	"aspect_ratio_index"
#define TEXTO7	"custom_viewport_width"

#define VALOR0	""
#define VALOR1	"false"
#define VALOR2	"true"
#define VALOR3	"576"
#define VALOR4	"608"
#define VALOR5	"672"
#define VALOR6	"768"
#define VALOR7	"0"
#define VALOR8	":/.retroarch/overlay/ATC/ATC-GB.cfg"
#define VALOR9	":/.retroarch/overlay/ATC/ATC-LYNX.cfg"
#define VALOR10	":/.retroarch/overlay/ATC/ATC-POKEMINI.cfg"
#define VALOR11	":/.retroarch/overlay/ATC/ATC-GG.cfg"
#define VALOR12	":/.retroarch/overlay/ATC/ATC-GBA.cfg"
#define VALOR13	":/.retroarch/overlay/ATC/ATC-WS.cfg"
#define VALOR14	":/.retroarch/overlay/ATC/ATC-GBC.cfg"
#define VALOR15	":/.retroarch/overlay/ATC/ATC-NGP.cfg"
#define VALOR16	":/.retroarch/overlay/ATC/ATC-SUPERVISION.cfg"
#define VALOR17	":/.retroarch/overlay/ATC/ATC-SGB.cfg"
#define VALOR18	":/.retroarch/filters/video/Grid3x.filt"
#define VALOR19	":/.retroarch/filters/video/Scanline2x.filt"
#define VALOR20	"23"

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

void update_config(const char* filename, const char* texto1, const char* valor1, const char* texto2, const char* valor2, const char* texto3, const char* valor3, const char* texto4, const char* valor4, const char* texto5, const char* valor5, const char* texto6, const char* valor6) {
	char buffer[100];
	sprintf(buffer, "/mnt/SDCARD/RetroArch/.retroarch/config/%s/%s.cfg", CORE, CONSOLA);
	if (access(buffer, F_OK) != -1) {
		remove(buffer);
	}
	FILE* file = fopen(buffer, "w");
	if (file != NULL) {
		if (strlen(texto1) > 0 && strlen(valor1) > 0) {
			fprintf(file, "%s = \"%s\"\n", texto1, valor1);
		}
		if (strlen(texto2) > 0 && strlen(valor2) > 0) {
			fprintf(file, "%s = \"%s\"\n", texto2, valor2);
		}
		if (strlen(texto3) > 0 && strlen(valor3) > 0) {
			fprintf(file, "%s = \"%s\"\n", texto3, valor3);
		}
		if (strlen(texto4) > 0 && strlen(valor4) > 0) {
			fprintf(file, "%s = \"%s\"\n", texto4, valor4);
		}
		if (strlen(texto5) > 0 && strlen(valor5) > 0) {
			fprintf(file, "%s = \"%s\"\n", texto5, valor5);
		}
		if (strlen(texto6) > 0 && strlen(valor6) > 0) {
			fprintf(file, "%s = \"%s\"\n", texto6, valor6);
		}
		fclose(file);
    }
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
	
	char FILECONFIG[100];
	sprintf(FILECONFIG, "/mnt/SDCARD/RetroArch/.retroarch/config/%s/%s.cfg", CORE, CONSOLA);
	
	if (strcmp(BORDER, "OFF") == 0 && strcmp(CONSOLA, "SGB") == 0) {
		FILE *fp = fopen("/mnt/SDCARD/RetroArch/.retroarch/config/mSGB/mSGB.opt", "r+");
		char line[256];
		
		while (fgets(line, sizeof(line), fp)) {
			if (strstr(line, "mgba_sgb_borders =")) {
				fseek(fp, -1 * strlen(line), SEEK_CUR);
				fputs("mgba_sgb_borders = \"OFF\"\n", fp);
				break;
			}
		}
		
		fclose(fp);
	} else if (strcmp(BORDER, "ON") == 0 && strcmp(CONSOLA, "SGB") == 0) {
		FILE *fp = fopen("/mnt/SDCARD/RetroArch/.retroarch/config/mSGB/mSGB.opt", "r+");
		char line[256];
		
		while (fgets(line, sizeof(line), fp)) {
			if (strstr(line, "mgba_sgb_borders =")) {
				fseek(fp, -1 * strlen(line), SEEK_CUR);
				fputs("mgba_sgb_borders = \"ON\"\n", fp);
				break;
			}
		}
		
		fclose(fp);
	}
	
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
								update_config(FILECONFIG, TEXTO1, VALOR2, TEXTO2, VALOR1, TEXTO5, VALOR18, TEXTO0, VALOR0, TEXTO0, VALOR0, TEXTO0, VALOR0); //aspect ratio
								SDL_BlitSurface(rect_surface, NULL, screen, &rect_pos);
								SDL_BlitSurface(text_surface, NULL, screen, &text_pos);
								SDL_Flip(screen);
								SDL_Delay(3000);
								break;
							case 1:
								update_config(FILECONFIG, TEXTO4, VALOR12, TEXTO1, VALOR2, TEXTO5, VALOR18, TEXTO0, VALOR0, TEXTO0, VALOR0, TEXTO0, VALOR0); //overlay
								SDL_BlitSurface(rect_surface, NULL, screen, &rect_pos);
								SDL_BlitSurface(text_surface, NULL, screen, &text_pos);
								SDL_Flip(screen);
								SDL_Delay(3000);
								break;
							case 2:
								update_config(FILECONFIG, TEXTO1, VALOR1, TEXTO2, VALOR1, TEXTO5, VALOR18, TEXTO0, VALOR0, TEXTO0, VALOR0, TEXTO0, VALOR0); //fullscreen
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
