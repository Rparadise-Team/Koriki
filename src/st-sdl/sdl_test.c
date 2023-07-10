#include <SDL/SDL.h>

int main(int argc, char** argv) {
	SDL_Init(SDL_INIT_VIDEO);
	printf("Testing SDL\n");

	SDL_Surface* screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);

	int quit = 0;
	while( !quit ){
		SDL_Event event;
		while( SDL_PollEvent( &event ) ){

			switch( event.type ){
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					quit = 1;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				default:
					break;
			}

		}
		SDL_Rect rect;
		rect.x = rand() % screen->w;
		rect.y = rand() % screen->h;
		rect.w = rand() % (screen->w - rect.x);
		rect.h = rand() % (screen->h - rect.y);
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, rand() % 255, rand() % 255, rand() % 255));
		SDL_Flip(screen);
		SDL_Delay(1000 / 60);
	}

	SDL_Quit();
}
