#ifndef SDL_BACKPORTS_H_
#define SDL_BACKPORTS_H_

#include <SDL.h>

#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDLC_Keycode SDL_Keycode
#else
#define SDLC_Keycode SDLKey
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 0)
typedef struct SDL_Point {
	int x;
	int y;
} SDL_Point;
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 4)
inline SDL_bool SDL_PointInRect(const SDL_Point *p, const SDL_Rect *r)
{
    return ( (p->x >= r->x) && (p->x < (r->x + r->w)) &&
             (p->y >= r->y) && (p->y < (r->y + r->h)) ) ? SDL_TRUE : SDL_FALSE;
}
#endif

#endif // SDL_BACKPORTS_H_
