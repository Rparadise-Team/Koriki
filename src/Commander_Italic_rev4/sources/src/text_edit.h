#ifndef TEXT_EDIT_H_
#define TEXT_EDIT_H_

#include <string>

#include <SDL.h>

#include "def.h"
#include "sdl_ptrs.h"

class TextEdit {
  public:
    explicit TextEdit(bool support_tabs = false)
        : support_tabs_(support_tabs)
    {
    }

    void setDimensions(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }

    void blitBackground(SDL_Surface &out, int x, int y) const;
    void blitForeground(SDL_Surface &out, int x, int y) const;

    const std::string &text() const { return text_; }
    void typeText(const std::string &text);
    void typeText(char c);
    bool backspace();
    bool del();

    bool isFocused() const { return focused_; }
    void setFocused(bool val) { focused_ = val; }

    bool moveCursorPrev();
    bool moveCursorNext();
    bool setCursorToStart();
    bool setCursorToEnd();

  private:
    void blitFocus(SDL_Surface &out, int x, int y) const;

    void prepareSurfaces();
    void prepareColors();

    void updateBackground();
    void updateForeground() const;

    std::string text_;
    std::size_t cursor_pos_ = 0;
    bool focused_ = false;

    // Rendering composes background, foreground, and cursor.
    //
    // Cached surfaces:
    mutable SDLSurfaceUniquePtr background_, foreground_;
    mutable int cursor_x_, text_x_ = 0;
    mutable bool update_foreground_ = false;

    SDL_Rect foreground_rect_;

    int width_, height_;

    int border_width_x_, border_width_y_, padding_x_, padding_y_;

    SDL_Color sdl_border_color_ = SDL_Color { COLOR_BORDER };
    std::uint32_t border_color_;

    SDL_Color sdl_focus_border_color_ = SDL_Color { COLOR_CURSOR_1 };
    std::uint32_t focus_border_color_;

    SDL_Color sdl_bg_color_ = SDL_Color { COLOR_BG_1 };
    std::uint32_t bg_color_;

    const bool support_tabs_;
};

#endif // TEXT_EDIT_H_
