#ifndef _SDLUTILS_H_
#define _SDLUTILS_H_

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#include <SDL.h>
#include <SDL_ttf.h>

#include "sdl_ptrs.h"
#include "sdl_ttf_multifont.h"
#include "window.h"

namespace SDL_utils
{
    void setMouseCursorEnabled(bool enabled);

    // Text alignment
    typedef enum
    {
        T_TEXT_ALIGN_LEFT = 0,
        T_TEXT_ALIGN_RIGHT,
        T_TEXT_ALIGN_CENTER
    }
    T_TEXT_ALIGN;

    inline SDL_Rect Rect(Sint16 x, Sint16 y, Uint16 w, Uint16 h)
    {
        return SDL_Rect{x, y, w, h};
    }

    // Load an image to fit the given viewport size.
    SDLSurfaceUniquePtr loadImageToFit(const std::string &p_filename, int fit_w, int fit_h);

    bool isSupportedImageExt(const std::string &filename);

    // Load a TTF font
    TTF_Font *loadFont(const std::string &p_font, const int p_size);

    // Apply a surface on another surface (logical coordinates)
    void applySurface(const Sint16 p_x, const Sint16 p_y, SDL_Surface* p_source, SDL_Surface* p_destination, SDL_Rect *p_clip = NULL);

    // Apply a surface on another surface (actual coordinates)
    void applyPpuScaledSurface(const Sint16 p_x, const Sint16 p_y, SDL_Surface* p_source, SDL_Surface* p_destination, SDL_Rect *p_clip = NULL);

    // Render a text
    SDL_Surface *renderText(const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg, bool use_second);
    SDL_Surface *renderText(const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg);

    // Render a text and apply on a given surface (logical coordinates)
    void applyText(Sint16 p_x, Sint16 p_y, SDL_Surface* p_destination, const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg, const T_TEXT_ALIGN p_align = T_TEXT_ALIGN_LEFT);

    // Render a text and apply on a given surface (actual coordinates)
    void applyPpuScaledText(Sint16 p_x, Sint16 p_y, SDL_Surface* p_destination, const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg, const T_TEXT_ALIGN p_align = T_TEXT_ALIGN_LEFT);

    // Get text dimensions (expensive).
    std::pair<int, int> measureText(const Fonts &fonts, const std::string &text);

    // Equivalent to SDL_Rect { ... } but avoids -Wnarrowing.
    inline SDL_Rect makeRect(int x, int y, int w, int h)
    {
        using Pos = decltype(SDL_Rect {}.x);
        using Len = decltype(SDL_Rect {}.w);
        return SDL_Rect { static_cast<Pos>(x), static_cast<Pos>(y),
            static_cast<Len>(w), static_cast<Len>(h) };
    }

    inline std::uint32_t mapRGB(const SDL_PixelFormat *fmt, SDL_Color c)
    {
        return SDL_MapRGB(fmt, c.r, c.g, c.b);
    }

    void removeBorder(SDL_Rect *rect, int border_width_x, int border_width_y);
    inline void removeBorder(SDL_Rect *rect, int border_width) {
        return removeBorder(rect, border_width, border_width);
    }

    void renderBorder(SDL_Surface *out, SDL_Rect rect,
        int border_width_x, int border_width_y, Uint32 border_color);

    void renderRectWithBorder(SDL_Surface *out, SDL_Rect rect,
        int border_width_x, int border_width_y, Uint32 border_color,
        Uint32 bg_color);
    inline void renderRectWithBorder(SDL_Surface *out, SDL_Rect rect,
        int border_width, Uint32 border_color, Uint32 bg_color)
    {
        return renderRectWithBorder(
            out, rect, border_width, border_width, border_color, bg_color);
    }

    // Create a surface in the same format as the screen
    SDL_Surface *createSurface(int width, int height);

    // Create an image filled with the given color
    SDL_Surface *createImage(const int p_width, const int p_height, const Uint32 p_color);

    // Render all opened windows
    void renderAll(void);

    // Cleanup and quit
    void hastalavista(void);

    // Display a waiting window
    void pleaseWait(void);
}

// Globals
namespace Globals
{
    // Screen
    extern SDL_Surface *g_screen;
    // Colors
    extern const SDL_Color g_colorTextNormal;
    extern const SDL_Color g_colorTextTitle;
    extern const SDL_Color g_colorTextDir;
    extern const SDL_Color g_colorTextSelected;
    // The list of opened windows
    extern std::vector<CWindow *> g_windows;
}

#endif
