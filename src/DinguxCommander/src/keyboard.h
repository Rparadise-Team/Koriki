#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <string>
#include <vector>
#include <utility>

#include <SDL.h>
#include <SDL_ttf.h>

#include "sdl_backports.h"
#include "sdl_ptrs.h"
#include "sdl_ttf_multifont.h"
#include "text_edit.h"
#include "window.h"

struct KeyboardLayout
{
    using Rows = std::vector<std::vector<std::string>>;
    std::vector<Rows> layers;
    std::size_t max_keys_per_row;
    std::size_t max_rows;
};

class CKeyboard : public CWindow
{
    public:
    // Constructor
    CKeyboard(const std::string &p_inputText, bool support_tabs = false);

    // Destructor
    ~CKeyboard() override = default;

    // Get input text
    const std::string &getInputText(void) const;

    bool handlesTextInput() const override { return true; }

    private:

    struct Keyboard
    {
        KeyboardLayout layout;

        const KeyboardLayout::Rows &current_keys() const
        {
            return layout.layers[current_keyset];
        }

        std::size_t num_rows() const
        {
            return layout.layers[current_keyset].size();
        }

        std::size_t num_row_keys(std::size_t row_index) const
        {
            return layout.layers[current_keyset][row_index].size();
        }

        const std::string &keycap(std::size_t x, std::size_t y) const;
        bool isBackspace(std::size_t x, std::size_t y) const;
        const std::string &text(std::size_t x, std::size_t y) const;

        // keycap dimensions: includes the border but not the gap.
        std::size_t key_w, key_h;

        std::size_t key_gap, border_w;

        bool collapse_borders;

        // Total width and height.
        std::size_t width, height;

        // Index of the selected keyset.
        std::size_t current_keyset;
        const std::size_t num_keysets() const { return layout.layers.size(); }
    };

    // Forbidden
    CKeyboard(void);
    CKeyboard(const CKeyboard &p_source);
    const CKeyboard &operator=(const CKeyboard &p_source);

    void init();

    void loadKeyboard();
    void calculateKeyboardDimensions(std::size_t max_w);

    SDL_Point getKeyCoordinates(int x, int y) const;
    std::pair<int, int> getButtonAt(SDL_Point p) const;

    void renderKeys(std::vector<SDLSurfaceUniquePtr> &out_surfaces, Sint16 x0, Sint16 y0,
        std::uint32_t key_bg_color, SDL_Color sdl_key_bg_color, std::uint32_t key_border_color) const;

    void renderButton(
        SDL_Surface &out, SDL_Rect rect, const std::string &text) const;
    void renderButtonHighlighted(
        SDL_Surface &out, SDL_Rect rect, const std::string &text) const;
    void renderButton(SDL_Surface &out, SDL_Rect rect, const std::string &text,
        std::uint32_t border_color, std::uint32_t bg_color,
        SDL_Color sdl_bg_color) const;

    // Window resized.
    void onResize() override;

    // Key press management
    const bool keyPress(const SDL_Event &p_event) override;

    // Key hold management
    const bool keyHold(void) override;

    bool mouseDown(int button, int x, int y) override;

    bool textInput(const SDL_Event &event) override;

    // Draw
    void render(const bool p_focus) const override;

    // Move cursor
    const bool moveCursorUp(const bool p_loop);
    const bool moveCursorDown(const bool p_loop);
    const bool moveCursorLeft(const bool p_loop);
    const bool moveCursorRight(const bool p_loop);

    bool pressFocusedKey();

    void focusOnTextEdit();
    bool isFocusOnTextEdit() const;
    bool isFocusOnButtonsRow() const;
    bool isFocusOnOk() const;
    bool isFocusOnCancel() const;

    const bool support_tabs_;

    // Colors:
    std::uint32_t border_color_;
    std::uint32_t bg_color_;
    SDL_Color sdl_bg_color_;
    std::uint32_t bg2_color_;
    std::uint32_t highlight_color_;
    SDL_Color sdl_highlight_color_;

    TextEdit text_edit_;

    // The cursor index
    std::size_t focus_x_;
    std::size_t focus_y_;

    // Pointers to resources
    const Fonts &m_fonts;

    // Layout:
    std::size_t frame_padding_x_, frame_padding_y_;
    std::size_t x_, y_, width_, height_;
    SDL_Rect text_field_rect_, kb_buttons_rect_, cancel_rect_, ok_rect_;

    Keyboard keyboard_;
    std::size_t keycap_text_offset_y_;

    // Background surfaces:
    std::vector<SDLSurfaceUniquePtr> surfaces_;
    SDLSurfaceUniquePtr footer_;

    // Highlighted keyboard keys
    std::vector<SDLSurfaceUniquePtr> kb_highlighted_surfaces_;

    // Foreground layer surfaces:
    SDLSurfaceUniquePtr cancel_highlighted_;
    SDLSurfaceUniquePtr ok_highlighted_;

    // Input config:
    SDLC_Keycode osk_backspace_;
    SDLC_Keycode osk_cancel_;
};

#endif
