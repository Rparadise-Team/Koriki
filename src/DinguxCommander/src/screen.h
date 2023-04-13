#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <SDL.h>

#include "config.h"
#ifdef MIYOOMINI
#include "gfx.h"
#endif

struct Screen
{
    // Logical width and height.
    decltype(SDL_Rect().w) w;
    decltype(SDL_Rect().h) h;

    // Scaling factors.
    float ppu_x;
    float ppu_y;

    // Actual width and height.
    decltype(SDL_Rect().w) actual_w;
    decltype(SDL_Rect().h) actual_h;

    // We target 25 FPS because currently the key repeat timer is tied into the
    // frame limit. :(
    int refreshRate = 25;

    SDL_Surface *surface;

#ifdef USE_SDL2
    SDL_Window *window;
#endif

    void flip() {
#ifdef USE_SDL2
		if (SDL_UpdateWindowSurface(window) <= -1) {
			SDL_Log("%s", SDL_GetError());
		}
		surface = SDL_GetWindowSurface(window);
#else
#ifdef MIYOOMINI
	GFX_Flip(surface);
#else
      SDL_Flip(surface);
      surface = SDL_GetVideoSurface();
#endif
#endif
    }

    // Called once at startup.
    int init();

    // Called on every SDL_RESIZE event.
    int onResize(int w, int h);

    void setPhysicalResolution(int actual_w, int actual_h);

    void zoom(float factor);
};

extern Screen screen;

#endif // _SCREEN_H_
