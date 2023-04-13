#ifndef __FONT_H__
#define __FONT_H__

void draw_char(SDL_Surface* surface, unsigned char symbol, int x, int y, unsigned short color);
void draw_string(SDL_Surface* surface, const char* text, int x, int y, unsigned short color);

#endif
