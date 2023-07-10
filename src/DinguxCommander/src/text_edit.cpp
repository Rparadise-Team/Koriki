#include "text_edit.h"

#include <cmath>
#include <cstdio>

#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"
#include "utf8.h"

namespace {
using SDL_utils::makeRect;
using SDL_utils::mapRGB;
using SDL_utils::renderBorder;
using SDL_utils::renderRectWithBorder;

bool is_ascii(unsigned char c) { return c <= 0x7f; }
} // namespace

void TextEdit::setDimensions(int width, int height)
{
    width_ = width;
    height_ = height;

    border_width_x_ = 2;
    border_width_y_ = border_width_x_ * std::round(screen.ppu_y / screen.ppu_x);

    padding_x_ = 5 * screen.ppu_x;
    padding_y_ = 3 * screen.ppu_y;

    foreground_rect_.x = border_width_x_;
    foreground_rect_.y = border_width_y_;
    foreground_rect_.w = width_ - 2 * border_width_x_;
    foreground_rect_.h = height_ - 2 * border_width_y_;

    prepareSurfaces();
    prepareColors();
    updateBackground();
    update_foreground_ = true;
}

void TextEdit::prepareSurfaces()
{
    background_.reset(SDL_utils::createSurface(width_, height_));
    foreground_.reset(
        SDL_utils::createSurface(foreground_rect_.w, foreground_rect_.h));
}

void TextEdit::prepareColors()
{
    const auto *pixel_format = background_->format;
    cursor_color_ = mapRGB(pixel_format, sdl_cursor_color_);
    border_color_ = mapRGB(pixel_format, sdl_border_color_);
    focus_border_color_ = mapRGB(pixel_format, sdl_focus_border_color_);
    bg_color_ = mapRGB(pixel_format, sdl_bg_color_);
}

void TextEdit::updateBackground()
{
    SDL_Rect bg_rect;
    bg_rect.x = 0;
    bg_rect.y = 0;
    bg_rect.w = width_;
    bg_rect.h = height_;
    renderRectWithBorder(background_.get(), bg_rect, border_width_x_,
        border_width_y_, border_color_, bg_color_);
}

void TextEdit::updateForeground() const
{
    update_foreground_ = false;
    SDL_FillRect(foreground_.get(), nullptr, bg_color_);
    if (text_.empty()) {
        text_x_ = cursor_x_ = 0;
        return;
    }
    const int max_w = foreground_rect_.w - 2 * padding_x_;

    std::string text_buf;
    const auto text_for_render
        = [this, &text_buf](const std::string &str) -> const std::string & {
        if (!support_tabs_) return str;
        text_buf = str;
        utf8::replaceTabsWithSpaces(&text_buf);
        return text_buf;
    };

    const auto &fonts = CResourceManager::instance().getFonts();
    SDLSurfaceUniquePtr tmp_surface { SDL_utils::renderText(fonts,
        text_for_render(text_), Globals::g_colorTextNormal, { COLOR_BG_1 }) };
    const int cursor_x = cursor_pos_ == text_.size()
        ? tmp_surface->w
        : SDL_utils::measureText(
            fonts, text_for_render(text_.substr(0, cursor_pos_)))
              .first;
    if (cursor_x < text_x_) text_x_ = cursor_x;
    if (cursor_x > text_x_ + max_w) text_x_ = cursor_x - max_w;

    cursor_x_ = cursor_x - text_x_;

    SDL_Rect rect;
    rect.x = text_x_;
    rect.y = 0;
    rect.w = max_w;
    rect.h = tmp_surface->h;
    SDL_utils::applyPpuScaledSurface(
        padding_x_, padding_y_, tmp_surface.get(), foreground_.get(), &rect);
}

void TextEdit::blitBackground(SDL_Surface &out, int x, int y) const
{
    SDL_utils::applyPpuScaledSurface(x, y, background_.get(), &out);
}

void TextEdit::blitForeground(SDL_Surface &out, int x, int y) const
{
    if (update_foreground_) updateForeground();

    SDL_utils::applyPpuScaledSurface(
        x + border_width_x_, y + border_width_y_, foreground_.get(), &out);

    blitFocus(out, x, y);

    SDL_Rect cursor_rect;
    cursor_rect.x = x + padding_x_ + border_width_x_ + cursor_x_;
    cursor_rect.y = y + padding_y_ + border_width_y_;
    cursor_rect.w = 1;
    cursor_rect.h = foreground_rect_.h - 2 * padding_y_;
    SDL_FillRect(&out, &cursor_rect, cursor_color_);
}

void TextEdit::blitFocus(SDL_Surface &out, int x, int y) const
{
    if (!focused_) return;
    renderBorder(&out, makeRect(x, y, width_, height_), border_width_x_,
        border_width_y_, focus_border_color_);
}

bool TextEdit::moveCursorPrev()
{
    if (cursor_pos_ == 0) return false;
    while (cursor_pos_ > 0 && utf8::isTrailByte(text_[cursor_pos_ - 1]))
        --cursor_pos_;
    if (cursor_pos_ > 0) --cursor_pos_;
    update_foreground_ = true;
    return true;
}

bool TextEdit::moveCursorNext()
{
    if (cursor_pos_ == text_.size()) return false;
    cursor_pos_ += utf8::codePointLen(text_.data() + cursor_pos_);
    update_foreground_ = true;
    return true;
}

bool TextEdit::setCursorToStart()
{
    if (cursor_pos_ == 0) return false;
    cursor_pos_ = 0;
    update_foreground_ = true;
    return true;
}

bool TextEdit::setCursorToEnd()
{
    if (cursor_pos_ == text_.size()) return false;
    cursor_pos_ = text_.size();
    update_foreground_ = true;
    return true;
}

void TextEdit::typeText(const std::string &text)
{
    if (text.empty()) return;
    text_.insert(cursor_pos_, text);
    cursor_pos_ += text.size();
    update_foreground_ = true;
}

void TextEdit::typeText(char c)
{
    text_.insert(cursor_pos_, 1, c);
    ++cursor_pos_;
    update_foreground_ = true;
}

bool TextEdit::backspace()
{
    if (cursor_pos_ == 0) return false;
    std::size_t left = cursor_pos_ - 1;
    while (left > 0 && utf8::isTrailByte(text_[left])) --left;
    text_.erase(left, cursor_pos_ - left);
    cursor_pos_ = left;
    update_foreground_ = true;
    return true;
}

bool TextEdit::del()
{
    if (cursor_pos_ == text_.size()) return false;
    text_.erase(cursor_pos_, utf8::codePointLen(text_.data() + cursor_pos_));
    update_foreground_ = true;
    return true;
}
