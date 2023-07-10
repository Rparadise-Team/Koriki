#ifndef CONFIG_H_
#define CONFIG_H_

#include <cstdlib>
#include <string>

#include "config_def.h"
#include "sdl_backports.h"

struct Config {
    // Display settings
    int disp_width = SCREEN_WIDTH;
    int disp_height = SCREEN_HEIGHT;
    int disp_bpp = SCREEN_BPP;
    float disp_ppu_x = PPU_X;
    float disp_ppu_y = PPU_Y;
    bool disp_autoscale = static_cast<bool>(AUTOSCALE);
    bool disp_autoscale_dpi = static_cast<bool>(AUTOSCALE_DPI);

    // Default paths to the left and right panels
    std::string path_default = PATH_DEFAULT;
    std::string path_default_right = PATH_DEFAULT_RIGHT;

    // Used if `path_default_right` does not exist or left path == right path.
    std::string path_default_right_fallback;

    // Default filesystem for Disk info
    std::string file_system = FILE_SYSTEM;

    // Resources directory (e.g. icons).
    std::string res_dir { RES_DIR };

    // Keyboard key code mappings
    SDLC_Keycode key_down = CMDR_KEY_DOWN;
    SDLC_Keycode key_left = CMDR_KEY_LEFT;
    SDLC_Keycode key_open = CMDR_KEY_OPEN;
    SDLC_Keycode key_operation = CMDR_KEY_OPERATION;
    SDLC_Keycode key_pagedown = CMDR_KEY_PAGEDOWN;
    SDLC_Keycode key_pageup = CMDR_KEY_PAGEUP;
    SDLC_Keycode key_parent = CMDR_KEY_PARENT;
    SDLC_Keycode key_right = CMDR_KEY_RIGHT;
    SDLC_Keycode key_select = CMDR_KEY_SELECT;
    SDLC_Keycode key_system = CMDR_KEY_SYSTEM;
    SDLC_Keycode key_transfer = CMDR_KEY_TRANSFER;
    SDLC_Keycode key_up = CMDR_KEY_UP;
    SDLC_Keycode key_menu = CMDR_KEY_MENU;	// added for TRIMUI

    // On-screen keyboard settings.
#ifdef OSK_KEY_SYSTEM_IS_BACKSPACE
    // `key_system` is backspace, `key_parent` is cancel
    bool osk_key_system_is_backspace = true;
#else
    // `key_system` is cancel, `key_parent` is backspace
    bool osk_key_system_is_backspace = false;
#endif

    void Load(const std::string &path);
};

Config &config();

#endif // CONFIG_H_
