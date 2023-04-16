#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "error_dialog.h"
#include "commander.h"
#include "def.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"

// Globals
const SDL_Color Globals::g_colorTextNormal = {COLOR_TEXT_NORMAL};
const SDL_Color Globals::g_colorTextTitle = {COLOR_TEXT_TITLE};
const SDL_Color Globals::g_colorTextDir = {COLOR_TEXT_DIR};
const SDL_Color Globals::g_colorTextSelected = {COLOR_TEXT_SELECTED};
std::vector<CWindow *> Globals::g_windows;

namespace {

bool fileExists(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

constexpr char kUsage[] =
    R"(commander [--config <path>] [--config-prelude <path>] [--res-dir <path>]

    --config <path>             Config file path. Default: ~/.config/commander.cfg.
    --config-prelude <path>     If provided, this config is loaded before the main config.
    --res-dir <path>            Resource directory. Overrides the configured one.
)";

} // namespace

int main(int argc, char *argv[])
{
    std::string config_prelude_path;
    std::string config_path;
    std::string res_dir;
    std::string exec_error;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0) {
            std::cout << kUsage;
            return 0;
        }
        if (std::strcmp(argv[i], "--config") == 0) {
            if (i == argc - 1) {
                std::cerr << "--config requires an argument\n";
                return 1;
            }
            config_path = argv[++i];
        } else if (std::strcmp(argv[i], "--config-prelude") == 0) {
            if (i == argc - 1) {
                std::cerr << "--config-prelude requires an argument\n";
                return 1;
            }
            config_prelude_path = argv[++i];
        } else if (std::strcmp(argv[i], "--res-dir") == 0) {
            if (i == argc - 1) {
                std::cerr << "--res-dir requires an argument\n";
                return 1;
            }
            res_dir = argv[++i];
        } else if (std::strcmp(argv[i], "--show_exec_error") == 0) {
            if (i == argc - 1) {
                std::cerr << "--show_exec_error requires an argument\n";
                return 1;
            }
            exec_error = argv[++i];
        }
    }

    auto &cfg = config();
    if (config_path.empty()) {
        std::string home_cfg_path
            = std::getenv("HOME") + std::string("/.config/commander.cfg");
        if (fileExists(home_cfg_path)) config_path = std::move(home_cfg_path);
    }
    if (!config_prelude_path.empty()) cfg.Load(config_prelude_path);
    if (!config_path.empty()) cfg.Load(config_path);
    if (!res_dir.empty()) cfg.res_dir = res_dir;

    CResourceManager::SetResDir(cfg.res_dir.c_str());

    // Avoid crash due to the absence of mouse
    char l_s[]="SDL_NOMOUSE=1";
    putenv(l_s);

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP) == 0) {
        std::cerr << "IMG_Init failed" << std::endl;
    } else {
        // Clear the errors for image libraries that did not initialize.
        SDL_ClearError();
    }

    // Hide cursor before creating the output surface.
    SDL_ShowCursor(SDL_DISABLE);

    if (screen.init() != 0) return 1;

    // Init font
    if (TTF_Init() == -1)
    {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

#ifndef USE_SDL2
    SDL_EnableUNICODE(1);
#endif

    // Create instances
    CResourceManager::instance();

    std::string l_path = cfg.path_default;
    std::string r_path = cfg.path_default_right;
    if (!fileExists(l_path)) l_path = "/";
    if (!cfg.path_default_right_fallback.empty()
        && (l_path == r_path || !fileExists(r_path)))
        r_path = cfg.path_default_right_fallback;
    if (!fileExists(r_path)) r_path = "/";
    CCommander l_commander(l_path, r_path);

    if (!exec_error.empty())
        ErrorDialog("Exec error", exec_error);

    // Main loop
    l_commander.execute();

    //Quit
    SDL_utils::hastalavista();

    return 0;
}
