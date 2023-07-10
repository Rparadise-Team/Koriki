#include <iostream>

#include <SDL_image.h>
#ifdef USE_SDL2
#include <SDL2_rotozoom.h>
#else
#include "SDL_rotozoom.h"
#endif
#include "resourceManager.h"
#include "def.h"
#include "screen.h"
#include "sdlutils.h"

namespace {

SDLSurfaceUniquePtr LoadIcon(const std::string &path) {
    SDL_Surface *img = IMG_Load(path.c_str());
    if(img == nullptr)
    {
        std::cerr << "LoadIcon(\"" << path << "\"): " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDLSurfaceUniquePtr scaled;
    if ((screen.ppu_x == 1 || screen.ppu_x == 2) && (screen.ppu_y == 1 || screen.ppu_y == 2)) {
        scaled = SDLSurfaceUniquePtr { shrinkSurface(img, 2 / screen.ppu_x, 2 / screen.ppu_y) };
    } else {
        scaled = SDLSurfaceUniquePtr { zoomSurface(img, screen.ppu_x / 2, screen.ppu_y / 2, SMOOTHING_ON) };
    }
    SDL_FreeSurface(img);
#ifdef USE_SDL2
    return scaled;
#else
    SDLSurfaceUniquePtr display { SDL_DisplayFormatAlpha(scaled.get()) };
    return display;
#endif
}

struct FontSpec {
    const char *const path;
    int size;
};
static constexpr FontSpec kFonts[] = {FONTS};
static constexpr std::size_t kFontsLen = sizeof(kFonts) / sizeof(kFonts[0]);
static constexpr FontSpec kLowDpiFonts[] = {LOW_DPI_FONTS};
static constexpr std::size_t kLowDpiFontsLen = sizeof(kLowDpiFonts) / sizeof(kLowDpiFonts[0]);

std::string ResDir = RES_DIR "";

std::string ResPath(const char *path) { return ResDir + path; }
std::string ResPath(const std::string &path) { return ResDir + path; }

std::vector<TTF_Font *> LoadFonts(bool low_dpi) {
    const FontSpec *specs = low_dpi ? kLowDpiFonts : kFonts;
    const std::size_t len = low_dpi ? kLowDpiFontsLen : kFontsLen;

    std::vector<TTF_Font *> fonts;
    fonts.reserve(len);
    for (std::size_t i = 0; i < len; ++i) {
        const std::string &path = specs[i].path;
        auto *font = SDL_utils::loadFont(path.front() == '/' ? path : ResPath(path), specs[i].size);
        if (font != nullptr) fonts.push_back(font);
    }
    if (fonts.empty()) {
        std::cerr << "No fonts found!" << std::endl;
        exit(1);
    }
    return fonts;
}

bool ShouldUseLowDpiFonts() {
    return screen.ppu_x <= 1.0 && kFonts[0].size < 12;
}

} // namespace

void CResourceManager::SetResDir(const char *res_dir)
{
    ResDir = res_dir;
    if (!ResDir.empty() && ResDir.back() != '/') ResDir += '/';
    std::fprintf(stderr, "Set resource directory to %s\n", ResDir.c_str());
}

CResourceManager& CResourceManager::instance()
{
    static CResourceManager l_singleton;
    return l_singleton;
}

CResourceManager::CResourceManager()
    : m_low_dpi_fonts(ShouldUseLowDpiFonts())
    , m_fonts(LoadFonts(m_low_dpi_fonts))
    , m_ppu_x(0)
    , m_ppu_y(0)
{
    onResize();
}

void CResourceManager::onResize()
{
    if (screen.ppu_x != m_ppu_x || screen.ppu_y != m_ppu_y) {
        m_surfaces[T_SURFACE_FOLDER] = LoadIcon(ResPath("folder.png"));
        m_surfaces[T_SURFACE_FILE] = LoadIcon(ResPath("file-text.png"));
        m_surfaces[T_SURFACE_FILE_IMAGE] = LoadIcon(ResPath("file-image.png"));
        m_surfaces[T_SURFACE_FILE_INSTALLABLE_PACKAGE]
            = LoadIcon(ResPath("file-ipk.png"));
        m_surfaces[T_SURFACE_FILE_PACKAGE] = LoadIcon(ResPath("file-opk.png"));
        m_surfaces[T_SURFACE_FILE_IS_SYMLINK]
            = LoadIcon(ResPath("file-is-symlink.png"));
        m_surfaces[T_SURFACE_UP] = LoadIcon(ResPath("up.png"));
    }

    m_surfaces[T_SURFACE_CURSOR1] = SDLSurfaceUniquePtr {
        SDL_utils::createImage(screen.actual_w / 2, LINE_HEIGHT_PHYS,
            SDL_MapRGB(screen.surface->format, COLOR_CURSOR_1))
    };
    m_surfaces[T_SURFACE_CURSOR2] = SDLSurfaceUniquePtr {
        SDL_utils::createImage(screen.actual_w / 2, LINE_HEIGHT_PHYS,
            SDL_MapRGB(screen.surface->format, COLOR_CURSOR_2))
    };

    const bool low_dpi_fonts = ShouldUseLowDpiFonts();
    if (m_low_dpi_fonts != low_dpi_fonts || screen.ppu_x != m_ppu_x
        || screen.ppu_y != m_ppu_y) {
        m_low_dpi_fonts = low_dpi_fonts;
        closeFonts();
        m_fonts = Fonts { LoadFonts(m_low_dpi_fonts) };
    }
    m_ppu_x = screen.ppu_x;
    m_ppu_y = screen.ppu_y;
}

void CResourceManager::sdlCleanup() {
    for (auto &surface : m_surfaces) surface = nullptr;
    closeFonts();
}

void CResourceManager::closeFonts() {
    for (auto &font : m_fonts.fonts()) {
        if (font != nullptr) {
            TTF_CloseFont(font);
            font = nullptr;
        }
    }
}

SDL_Surface *CResourceManager::getSurface(const T_SURFACE p_surface) const
{
    return m_surfaces[p_surface].get();
}

const Fonts &CResourceManager::getFonts() const
{
    return m_fonts;
}
