#include "screen.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "sdlutils.h"

Screen screen;

namespace {

#ifndef USE_SDL2
SDL_Surface *SetVideoMode(int width, int height, int bpp, std::uint32_t flags)
{
    std::fprintf(stderr, "Setting video mode %dx%d bpp=%u flags=0x%08X\n",
        width, height, bpp, flags);
    std::fflush(stderr);
#ifdef MIYOOMINI
    auto *result = GFX_CreateRGBSurface(SDL_SWSURFACE, width, height, bpp, 0,0,0,0);
#else
    auto *result = SDL_SetVideoMode(width, height, bpp, flags);
#endif
    const auto &current = *SDL_GetVideoInfo();
    std::fprintf(stderr, "Video mode is now %dx%d bpp=%u flags=0x%08X\n",
        current.current_w, current.current_h, current.vfmt->BitsPerPixel,
        SDL_GetVideoSurface()->flags);
    std::fflush(stderr);
    return result;
}
#endif

}

int Screen::init()
{
    const auto &cfg = config();
    screen.w = cfg.disp_width;
    screen.h = cfg.disp_height;
    screen.ppu_x = cfg.disp_ppu_x;
    screen.ppu_y = cfg.disp_ppu_y;
    screen.actual_w = cfg.disp_width * cfg.disp_ppu_x;
    screen.actual_h = cfg.disp_height * cfg.disp_ppu_y;

#ifndef USE_SDL2
#ifdef MIYOOMINI
    GFX_Init();
#endif
    const auto &best = *SDL_GetVideoInfo();
    std::fprintf(stderr,
        "Best video mode reported as: %dx%d bpp=%d hw_available=%u\n",
        best.current_w, best.current_h, best.vfmt->BitsPerPixel,
        best.hw_available);

    // If the display resolution doesn't match the configured one:
    // 1. Increase the resolution.
    // 2. Use 2x DPI if the horizontal resolution is >2x the configured one.
    if (cfg.disp_autoscale
        && (best.current_w != screen.actual_w
            || best.current_h != screen.actual_h)) {
        if (cfg.disp_autoscale_dpi) {
            if (best.current_w >= screen.w * 2) {
                // E.g. 640x480. Upscale to the smaller of the two.
                const float scale
                    = std::min(best.current_w / static_cast<float>(screen.w),
                        best.current_h / static_cast<float>(screen.h));
                screen.ppu_x = screen.ppu_y = std::min(scale, 2.0f);
            } else if (best.current_w != screen.w) {
                // E.g. RS07 with 480x272 screen.
                screen.ppu_x = screen.ppu_y = 1;
            }
        }
        setPhysicalResolution(best.current_w, best.current_h);
    }
#endif

#ifdef USE_SDL2
    int window_flags = SDL_WINDOW_RESIZABLE;
    if (cfg.disp_autoscale) window_flags |= SDL_WINDOW_MAXIMIZED;

    int window_w = screen.actual_w;
    int window_h = screen.actual_h;
    window = SDL_CreateWindow("Commander", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, window_h, window_h, window_flags);
    if (window == nullptr) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }
    if (cfg.disp_autoscale && cfg.disp_autoscale_dpi) {
        const int disp_index = SDL_GetWindowDisplayIndex(window);
        if (disp_index != -1) {
            float hdpi, vdpi;
            if (SDL_GetDisplayDPI(disp_index, /*ddpi=*/nullptr, &hdpi, &vdpi)
                != -1) {
                if (hdpi != 0 && vdpi != 0) {
                    screen.ppu_x = hdpi / 72.0;
                    screen.ppu_y = vdpi / 72.0;
                }
                SDL_Log("Display DPI: %f %f. Scaling factors: %f %f", hdpi,
                    vdpi, screen.ppu_x, screen.ppu_y);
            } else {
                SDL_Log("SDL_GetDisplayDPI failed: %s", SDL_GetError());
            }
        } else {
            SDL_Log("SDL_GetWindowDisplayIndex failed: %s", SDL_GetError());
        }
    }
    SDL_GetWindowSize(window, &window_w, &window_h);
    setPhysicalResolution(window_w, window_h);
    screen.surface = SDL_GetWindowSurface(window);
#else
    surface = SetVideoMode(screen.actual_w, screen.actual_h, SCREEN_BPP,
        SDL_SWSURFACE | SDL_RESIZABLE);
    if (surface == nullptr) {
        std::fprintf(stderr, "SDL_SetVideoMode failed: %s\n", SDL_GetError());
        return 1;
    }
#endif
    return 0;
}

int Screen::onResize(int w, int h)
{
#ifdef USE_SDL2
    this->surface = SDL_GetWindowSurface(this->window);
    setPhysicalResolution(surface->w, surface->h);
#else
#ifndef MIYOOMINI
    this->surface = SDL_GetVideoSurface();
    if (this->surface->w < w || this->surface->h < h) {
        this->surface = SDL_SetVideoMode(
            w, h, this->surface->format->BitsPerPixel, this->surface->flags);
    }
#endif
    setPhysicalResolution(w, h);
#endif
    return 0;
}

void Screen::setPhysicalResolution(int actual_w, int actual_h)
{
    this->actual_w = actual_w;
    this->actual_h = actual_h;
    this->w = static_cast<int>(actual_w / ppu_x);
    this->h = static_cast<int>(actual_h / ppu_y);
}

void Screen::zoom(float factor) {
    ppu_x *= factor;
    ppu_y *= factor;
    this->w = static_cast<int>(actual_w / ppu_x);
    this->h = static_cast<int>(actual_h / ppu_y);
}
