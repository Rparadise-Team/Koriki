#ifndef _RESOURCEMANAGER_H_
#define _RESOURCEMANAGER_H_

#include <array>
#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>

#include "sdl_ptrs.h"
#include "sdl_ttf_multifont.h"

class CResourceManager
{
    public:

    static void SetResDir(const char *res_dir);

    typedef enum
    {
        T_SURFACE_FOLDER = 0,
        T_SURFACE_FILE,
        T_SURFACE_FILE_IMAGE,
        T_SURFACE_FILE_INSTALLABLE_PACKAGE,
        T_SURFACE_FILE_PACKAGE,
        T_SURFACE_FILE_IS_SYMLINK,
        T_SURFACE_UP,
        T_SURFACE_CURSOR1,
        T_SURFACE_CURSOR2
    }
    T_SURFACE;

    // Method to get the instance
    static CResourceManager& instance(void);

    // Called on window resize.
    void onResize();

    // Cleanup all resources
    void sdlCleanup(void);

    void closeFonts();

    // Get a loaded surface
    SDL_Surface *getSurface(const T_SURFACE p_surface) const;

    // Get the loaded fonts
    const Fonts &getFonts(void) const;

    private:

    // Forbidden
    CResourceManager();
    CResourceManager(const CResourceManager &p_source) = delete;
    const CResourceManager &operator =(const CResourceManager &p_source) = delete;

    // Images
    std::array<SDLSurfaceUniquePtr, 9> m_surfaces {};

    // Fonts
    bool m_low_dpi_fonts;
    Fonts m_fonts;

    // PPU values when the resources were loaded.
    float m_ppu_x, m_ppu_y;
};

#endif
