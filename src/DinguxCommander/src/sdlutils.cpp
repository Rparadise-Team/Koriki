#include "sdlutils.h"

#include <algorithm>
#include <iostream>

#include <SDL_image.h>
#ifdef USE_SDL2
#include <SDL2_rotozoom.h>
#else
#include "SDL_rotozoom.h"
#endif
#include "def.h"
#include "fileutils.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdl_ttf_multifont.h"
#include "sdl_ptrs.h"

namespace SDL_utils {

void setMouseCursorEnabled(bool enabled) {
    SDL_ShowCursor(enabled ? 1 : 0);
}

bool isSupportedImageExt(const std::string &ext) {
    return ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "ico" || ext == "bmp" || ext == "xcf";
}

SDLSurfaceUniquePtr loadImageToFit(
    const std::string &p_filename, int fit_w, int fit_h)
{
    // Load image
    SDL_Surface *l_img = IMG_Load(p_filename.c_str());
    if (IMG_GetError() != nullptr && *IMG_GetError() != '\0') {
        if (!strcmp(IMG_GetError(), "Unsupported image format") == 0)
            std::cerr << "loadImageToFit: " << IMG_GetError() << std::endl;
        SDL_ClearError();
        return nullptr;
    }
    const double aspect_ratio = static_cast<double>(l_img->w) / l_img->h;
    int target_w, target_h;
    if (fit_w * l_img->h <= fit_h * l_img->w) {
        target_w = std::min(l_img->w, fit_w);
        target_h = target_w / aspect_ratio;
    } else {
        target_h = std::min(l_img->h, fit_h);
        target_w = target_h * aspect_ratio;
    }
    target_w *= screen.ppu_x;
    target_h *= screen.ppu_y;
    SDLSurfaceUniquePtr l_img2 { zoomSurface(l_img,
        static_cast<double>(target_w) / l_img->w,
        static_cast<double>(target_h) / l_img->h, SMOOTHING_ON) };
    SDL_FreeSurface(l_img);

    const std::string ext = File_utils::getLowercaseFileExtension(p_filename);
    const bool supports_alpha = ext != "xcf" && ext != "jpg" && ext != "jpeg";
#ifdef USE_SDL2
    auto l_img3 = supports_alpha
        ? std::move(l_img2)
        : SDLSurfaceUniquePtr { SDL_ConvertSurface(
            l_img2.get(), screen.surface->format, SDL_SWSURFACE) };
#else
    SDLSurfaceUniquePtr l_img3 { supports_alpha
            ? SDL_DisplayFormatAlpha(l_img2.get())
            : SDL_DisplayFormat(l_img2.get()) };
#endif
    return l_img3;
}

void applyPpuScaledSurface(const Sint16 p_x, const Sint16 p_y, SDL_Surface* p_source, SDL_Surface* p_destination, SDL_Rect *p_clip)
{
    // Rectangle to hold the offsets
    SDL_Rect l_offset;
    // Set offsets
    l_offset.x = p_x;
    l_offset.y = p_y;
    //Blit the surface
    SDL_BlitSurface(p_source, p_clip, p_destination, &l_offset);
}

void applySurface(const Sint16 p_x, const Sint16 p_y, SDL_Surface* p_source, SDL_Surface* p_destination, SDL_Rect *p_clip)
{
    return applyPpuScaledSurface(p_x * screen.ppu_x, p_y * screen.ppu_y, p_source, p_destination, p_clip);
}

TTF_Font *loadFont(const std::string &p_font, const int p_size)
{
    INHIBIT(std::cout << "loadFont(" << p_font << ", " << p_size << ")" << std::endl;)
#ifdef USE_TTF_OPENFONT_DPI
    TTF_Font *l_font = TTF_OpenFontDPI(p_font.c_str(), p_size, 72 * screen.ppu_x, 72 * screen.ppu_y);
#else
    TTF_Font *l_font = TTF_OpenFont(p_font.c_str(), p_size);
#endif
    if (l_font == NULL) {
        std::cerr << "loadFont: " << SDL_GetError() << std::endl;
        SDL_ClearError();
    }
    return l_font;
}

SDL_Surface *renderText(const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg, bool use_second)
{
    SDL_Surface *result = TTFMultiFont_RenderUTF8_Shaded(p_fonts, p_text, p_fg, p_bg, use_second);
    if (result == nullptr && SDL_GetError() != nullptr && SDL_GetError()[0] != '\0') {
        std::cerr << "TTFMultiFont_RenderUTF8_Shaded: " << SDL_GetError() << std::endl;
        SDL_ClearError();
    }
    return result;
}
SDL_Surface *renderText(const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg)
{
    return renderText(p_fonts, p_text, p_fg, p_bg, false);
}

std::pair<int, int> measureText(const Fonts &fonts, const std::string &text) {
    if (text.empty()) return {0, 0};
    SDLSurfaceUniquePtr surface { TTFMultiFont_RenderUTF8_Shaded(
        fonts, text, SDL_Color { 0, 0, 0, 0 }, SDL_Color { 0, 0, 0, 0 }) };
    if (surface == nullptr && SDL_GetError() != nullptr && SDL_GetError()[0] != '\0') {
        std::cerr << "TTFMultiFont_RenderUTF8_Shaded: " << SDL_GetError() << std::endl;
        SDL_ClearError();
        return {0, 0};
    }
    return {surface->w, surface->h};
}

void applyPpuScaledText(Sint16 p_x, Sint16 p_y, SDL_Surface* p_destination, const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg, const T_TEXT_ALIGN p_align)
{
    SDL_Surface *l_text = renderText(p_fonts, p_text, p_fg, p_bg);
    switch (p_align)
    {
        case T_TEXT_ALIGN_LEFT:
            applyPpuScaledSurface(p_x, p_y, l_text, p_destination);
            break;
        case T_TEXT_ALIGN_RIGHT:
            applyPpuScaledSurface(p_x - l_text->w, p_y, l_text, p_destination);
            break;
        case T_TEXT_ALIGN_CENTER:
            applyPpuScaledSurface(p_x - l_text->w / 2, p_y, l_text, p_destination);
            break;
        default:
            break;
    }
    SDL_FreeSurface(l_text);
}

void applyText(Sint16 p_x, Sint16 p_y, SDL_Surface* p_destination, const Fonts &p_fonts, const std::string &p_text, const SDL_Color &p_fg, const SDL_Color &p_bg, const T_TEXT_ALIGN p_align)
{
    SDL_Surface *l_text = renderText(p_fonts, p_text, p_fg, p_bg);
    switch (p_align)
    {
        case T_TEXT_ALIGN_LEFT:
            applySurface(p_x, p_y, l_text, p_destination);
            break;
        case T_TEXT_ALIGN_RIGHT:
            applySurface(p_x - l_text->w / screen.ppu_x, p_y, l_text, p_destination);
            break;
        case T_TEXT_ALIGN_CENTER:
            applySurface(p_x - l_text->w / 2 / screen.ppu_x, p_y, l_text, p_destination);
            break;
        default:
            break;
    }
    SDL_FreeSurface(l_text);
}

void removeBorder(SDL_Rect *rect, int border_width_x, int border_width_y)
{
    rect->x += border_width_x;
    rect->y += border_width_y;
    rect->w -= 2 * border_width_x;
    rect->h -= 2 * border_width_y;
}

void renderBorder(SDL_Surface *out, SDL_Rect rect,
    int border_width_x, int border_width_y, Uint32 border_color) {
    SDL_Rect line = rect;
    line.w = border_width_x;
    SDL_FillRect(out, &line, border_color);
    line.x = rect.x + rect.w - border_width_x;
    SDL_FillRect(out, &line, border_color);
    line.x = rect.x;
    line.w = rect.w;
    line.h = border_width_y;
    SDL_FillRect(out, &line, border_color);
    line.y = rect.y + rect.h - border_width_y;
    SDL_FillRect(out, &line, border_color);
}

void renderRectWithBorder(SDL_Surface *out, SDL_Rect rect,
    int border_width_x, int border_width_y, Uint32 border_color, Uint32 bg_color)
{
    renderBorder(out, rect, border_width_x, border_width_y, border_color);
    removeBorder(&rect, border_width_x, border_width_y);
    SDL_FillRect(out, &rect, bg_color);
}

SDL_Surface *createSurface(int width, int height)
{
    return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, screen.surface->format->BitsPerPixel, screen.surface->format->Rmask, screen.surface->format->Gmask, screen.surface->format->Bmask, screen.surface->format->Amask);
}

SDL_Surface *createImage(const int p_width, const int p_height, const Uint32 p_color)
{
    SDL_Surface *l_ret = createSurface(p_width, p_height);
    if (l_ret == NULL)
        std::cerr << "createImage: " << SDL_GetError() << std::endl;
    // Fill image with the given color
    SDL_FillRect(l_ret, NULL, p_color);
    return l_ret;
}

void renderAll(void)
{
    if (Globals::g_windows.empty())
        return;
    // First window to draw is the last fullscreen
    unsigned int l_i = Globals::g_windows.size() - 1;
    while (l_i && !Globals::g_windows[l_i]->isFullScreen())
        --l_i;
    // Draw windows
    for (std::vector<CWindow *>::iterator l_it = Globals::g_windows.begin() + l_i; l_it != Globals::g_windows.end(); ++l_it)
        (*l_it)->render(l_it + 1 == Globals::g_windows.end());
}

void hastalavista(void)
{
    // Destroy all dialogs except the first one (the commander)
    while (Globals::g_windows.size() > 1)
        delete Globals::g_windows.back();
    // Free resources
    CResourceManager::instance().sdlCleanup();
#ifdef MIYOOMINI
	GFX_FreeSurface(screen.surface);
	GFX_Quit();
#endif
    // Quit SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void pleaseWait(void)
{
    SDLSurfaceUniquePtr text_surface { renderText(
        CResourceManager::instance().getFonts(), "Please wait...",
        Globals::g_colorTextNormal, { COLOR_BG_2 }) };
    const int border_x = static_cast<int>(DIALOG_BORDER * screen.ppu_x);
    const int border_y = static_cast<int>(DIALOG_BORDER * screen.ppu_y);
    const int padding_x = static_cast<int>(DIALOG_PADDING * screen.ppu_y);
    const int padding_y = static_cast<int>(4 * screen.ppu_y);
    const int dialog_w = text_surface->w + 2 * (border_x + padding_x);
    const int dialog_h = text_surface->h + 2 * (border_y + padding_y);
    SDL_Rect l_rect = Rect((screen.actual_w - dialog_w) / 2,
        (screen.actual_h - dialog_h) / 2, dialog_w, dialog_h);
    SDL_FillRect(screen.surface, &l_rect, SDL_MapRGB(screen.surface->format, COLOR_BG_2));
    renderBorder(screen.surface, l_rect, border_x, border_y,
        SDL_MapRGB(screen.surface->format, COLOR_BORDER));
    applyPpuScaledSurface(l_rect.x + border_x + padding_x,
        l_rect.y + border_y + padding_y, text_surface.get(), screen.surface);
    screen.flip();
}

} // namespace SDL_utils
