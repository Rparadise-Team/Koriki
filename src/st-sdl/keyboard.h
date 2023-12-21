#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <SDL/SDL.h>

void init_keyboard();
void draw_keyboard(SDL_Surface* surface);
int handle_keyboard_event(SDL_Event* event);
extern int active;
extern int show_help;

#endif
