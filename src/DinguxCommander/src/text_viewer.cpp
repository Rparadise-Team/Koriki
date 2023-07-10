#include "text_viewer.h"

#include <algorithm>
#include <cstring>
#include <fstream>

#include "config.h"
#include "def.h"
#include "error_dialog.h"
#include "keyboard.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"
#include "utf8.h"

#define VIEWER_PADDING_X 1
#define VIEWER_PADDING_X_PHYS static_cast<int>(VIEWER_PADDING_X * screen.ppu_x)
#define VIEWER_LINE_HEIGHT 13
#define VIEWER_LINE_HEIGHT_PHYS                                                \
    static_cast<int>(VIEWER_LINE_HEIGHT * screen.ppu_y)
#define VIEWER_Y_LIST 17
#define VIEWER_Y_LIST_PHYS static_cast<int>(VIEWER_Y_LIST * screen.ppu_y)
#define VIEWER_X_STEP 32
#define VIEWER_X_STEP_PHYS static_cast<int>(VIEWER_X_STEP * screen.ppu_x)

namespace {

// The number of lines that fully fit into the viewport.
int numFullViewportLines()
{
    return (screen.actual_h - VIEWER_Y_LIST_PHYS) / VIEWER_LINE_HEIGHT_PHYS;
}

// The number of lines that are visible (even if partially) in the viewport.
int numTotalViewportLines()
{
    return (screen.actual_h - VIEWER_Y_LIST_PHYS - 1) / VIEWER_LINE_HEIGHT_PHYS
        + 1;
}

void adjustLineForDisplay(std::string *line)
{
    utf8::replaceTabsWithSpaces(line);
}

} // namespace

TextViewer::TextViewer(std::string filename)
    : fonts_(CResourceManager::instance().getFonts())
    , filename_(std::move(filename))
    , first_line_(0)
    , current_line_(0)
{
    std::ifstream input_file(filename_.c_str());
    if (!input_file.is_open()) {
        ErrorDialog(
            "Unable to open file", filename_ + "\n" + std::strerror(errno));
        m_retVal = -1;
        return;
    }
    while (!input_file.eof()) {
        lines_.emplace_back();
        auto &line = lines_.back();
        std::getline(input_file, line);
        if (!line.empty() && line.back() == '\r') line.pop_back();
    }

    if (!lines_.empty()) {
        utf8::removeBom(&lines_[0]);

        // Remove EOL at the end of the file.
        // We'll add it back when saving.
        if (lines_.back().empty()) lines_.pop_back();
    }
    if (lines_.empty()) lines_.emplace_back();

    input_file.close();
    lines_for_display_ = lines_;
    for (auto &line : lines_for_display_) { adjustLineForDisplay(&line); }
    clip_.x = clip_.y = 0;
    init();
}

void TextViewer::init()
{
    const auto &fonts = CResourceManager::instance().getFonts();

    const SDL_PixelFormat *pixel_format = screen.surface->format;
    border_color_ = SDL_MapRGB(pixel_format, COLOR_BORDER);
    sdl_bg_color_ = SDL_Color { COLOR_BG_1 };
    bg_color_ = SDL_MapRGB(pixel_format, COLOR_BG_1);
    sdl_highlight_color_ = SDL_Color { COLOR_CURSOR_1 };
    highlight_color_ = SDL_MapRGB(pixel_format, COLOR_CURSOR_1);

    // Create background image
    background_ = SDLSurfaceUniquePtr { SDL_utils::createImage(
        screen.actual_w, screen.actual_h, bg_color_) };
    {
        SDL_Rect rect = SDL_utils::Rect(0, 0, screen.actual_w, HEADER_H_PHYS);
        SDL_FillRect(background_.get(), &rect, border_color_);
    }
    // Print title
    {
        SDLSurfaceUniquePtr tmp { SDL_utils::renderText(
            fonts, filename_, Globals::g_colorTextTitle, { COLOR_TITLE_BG }) };
        SDL_Rect rect;
        SDL_Rect *clip_rect = nullptr;
        if (tmp->w > background_->w - 2 * VIEWER_PADDING_X_PHYS) {
            rect.x = tmp->w - (background_->w - 2 * VIEWER_PADDING_X_PHYS);
            rect.y = 0;
            rect.w = background_->w - 2 * VIEWER_PADDING_X_PHYS;
            rect.h = tmp->h;
            clip_rect = &rect;
        }
        SDL_utils::applyPpuScaledSurface(VIEWER_PADDING_X_PHYS,
            HEADER_PADDING_TOP_PHYS, tmp.get(), background_.get(), clip_rect);
    }
    clip_.w = screen.actual_w - 2 * VIEWER_PADDING_X_PHYS;
}

void TextViewer::onResize()
{
    const int max_first_line = maxFirstLine();
    if (current_line_ > first_line_ + numFullViewportLines() - 1) {
        first_line_ = std::min(
            static_cast<int>(current_line_ - numFullViewportLines() + 1),
            max_first_line);
    }
    init();
}

void TextViewer::render(const bool focused) const
{
    SDL_utils::applyPpuScaledSurface(0, 0, background_.get(), screen.surface);

    std::size_t i = std::min(
        first_line_ + numTotalViewportLines() + 1, lines_for_display_.size());
    SDL_Rect clip = clip_;
    const int y0 = VIEWER_Y_LIST_PHYS;
    const int line_height = VIEWER_LINE_HEIGHT_PHYS;
    while (i-- > first_line_) {
        const std::string &line = lines_for_display_[i];
        const int viewport_line_i = static_cast<int>(i - first_line_);
        SDLSurfaceUniquePtr tmp;
        if (!line.empty()) {
            tmp = SDLSurfaceUniquePtr { SDL_utils::renderText(fonts_, line,
                Globals::g_colorTextNormal,
                i == current_line_ ? sdl_highlight_color_ : sdl_bg_color_) };
        }
        const int y = y0 + viewport_line_i * line_height;
        if (i == current_line_) {
            SDL_Rect hl_rect
                = SDL_utils::makeRect(0, y, screen.actual_w, line_height);
            SDL_FillRect(screen.surface, &hl_rect, highlight_color_);
        }
        if (tmp == nullptr) continue;
        clip.h = tmp->h;
        SDL_utils::applyPpuScaledSurface(
            VIEWER_PADDING_X_PHYS, y, tmp.get(), screen.surface, &clip);
    }
}

const bool TextViewer::keyPress(const SDL_Event &event)
{
    CWindow::keyPress(event);
    const auto &c = config();
    const auto sym = event.key.keysym.sym;
    if (sym == c.key_system || sym == c.key_parent) {
        m_retVal = -1;
        return true;
    }
    if (sym == c.key_open || sym == c.key_operation) return editLine();
    if (sym == c.key_up) return moveUp(1);
    if (sym == c.key_down) return moveDown(1);
    if (sym == c.key_pageup) return moveUp(numFullViewportLines() - 1);
    if (sym == c.key_pagedown) return moveDown(numFullViewportLines() - 1);
    if (sym == c.key_left) return moveLeft();
    if (sym == c.key_right) return moveRight();
    return false;
}

const bool TextViewer::keyHold()
{
    const auto &c = config();
    if (m_lastPressed == c.key_up) return tick(c.key_up) && moveUp(1);
    if (m_lastPressed == c.key_down) return tick(c.key_down) && moveDown(1);
    if (m_lastPressed == c.key_pageup)
        return tick(c.key_pageup) && moveUp(numFullViewportLines() - 1);
    if (m_lastPressed == c.key_pagedown)
        return tick(c.key_pagedown) && moveDown(numFullViewportLines() - 1);
    if (m_lastPressed == c.key_left) return tick(c.key_left) && moveLeft();
    if (m_lastPressed == c.key_right) return tick(c.key_right) && moveRight();
    return false;
}

int TextViewer::getLineAt(int x, int y) const
{
    const int y0 = VIEWER_Y_LIST_PHYS;
    if (y < y0) return -1;
    const int line_height = VIEWER_LINE_HEIGHT_PHYS;
    const int line = (y - y0) / line_height;
    if (first_line_ + line >= lines_.size()) return -1;
    return line;
}

int TextViewer::maxFirstLine() const
{
    const int viewport_lines = numFullViewportLines();
    return lines_.size() >= viewport_lines ? lines_.size() - viewport_lines : 0;
}

bool TextViewer::mouseWheel(int dx, int dy)
{
    bool changed = false;
    if (dy > 0) {
        changed = moveUp(/*step=*/1) || changed;
    } else if (dy < 0) {
        changed = moveDown(/*step=*/1) || changed;
    }
    if (dx < 0) {
        changed = moveLeft() || changed;
    } else if (dx > 0) {
        changed = moveRight() || changed;
    }
    return changed;
}

bool TextViewer::mouseDown(int button, int x, int y)
{
    switch (button) {
        case SDL_BUTTON_LEFT: {
            const int line = getLineAt(x, y);
            if (line != -1) {
                const std::size_t new_current_line = first_line_ + line;
                if (current_line_ == new_current_line) {
                    editLine();
                    return true;
                }
                current_line_ = new_current_line;
                if (first_line_ + numFullViewportLines() < current_line_)
                    ++first_line_;
                return true;
            }
            return false;
        }
        case SDL_BUTTON_RIGHT:
        case SDL_BUTTON_X1: m_retVal = -1; return true;
    }
    return false;
}

bool TextViewer::moveUp(unsigned step)
{
    bool changed = false;
    if (current_line_ > 0) {
        current_line_ = current_line_ >= step ? current_line_ - step : 0;
        changed = true;
    }
    if (current_line_ < first_line_) { first_line_ = current_line_; }
    return changed;
}

bool TextViewer::moveDown(unsigned step)
{
    if (lines_.empty()) return false;
    bool changed = false;
    if (current_line_ + 1 < lines_.size()) {
        current_line_ = std::min(current_line_ + step, lines_.size() - 1);
        changed = true;
    }
    const int max_first_line = maxFirstLine();
    if (current_line_ > first_line_ + numFullViewportLines() - 1) {
        first_line_ = std::min(
            static_cast<int>(current_line_ - numFullViewportLines() + 1),
            max_first_line);
    }
    return changed;
}

bool TextViewer::moveLeft()
{
    if (clip_.x > 0) {
        if (clip_.x > VIEWER_X_STEP_PHYS) {
            clip_.x -= VIEWER_X_STEP_PHYS;
        } else {
            clip_.x = 0;
        }
        return true;
    }
    return false;
}

bool TextViewer::moveRight()
{
    clip_.x += VIEWER_X_STEP_PHYS;
    return true;
}

bool TextViewer::editLine()
{
    std::string title = lines_for_display_[current_line_];
    constexpr std::size_t kMaxTitleLen = 60;
    if (title.size() > kMaxTitleLen) {
        std::size_t len = kMaxTitleLen - 3;
        if (!utf8::isTrailByte(title[len])) {
            while (len > 0 && utf8::isTrailByte(title[len - 1])) --len;
        }
        title.resize(len);
        title.append("...");
    }
    title = "Line " + std::to_string(current_line_ + 1) + ": " + title;
    CDialog dialog { title };
    dialog.addLabel("Saved automatically");
    std::vector<std::function<bool()>> handlers;

    dialog.addOption("Edit line");
    handlers.push_back([&]() {
        CKeyboard keyboard(lines_[current_line_], /*support_tabs=*/true);
        if (keyboard.execute() == 1
            && keyboard.getInputText() != lines_[current_line_]) {
            lines_for_display_[current_line_] = lines_[current_line_]
                = keyboard.getInputText();
            adjustLineForDisplay(&lines_for_display_[current_line_]);
            saveFile();
        }
        return true;
    });

    dialog.addOption("Duplicate line");
    handlers.push_back([&]() {
        lines_.emplace(
            lines_.begin() + current_line_ + 1, lines_[current_line_]);
        lines_for_display_.emplace(
            lines_for_display_.begin() + current_line_ + 1,
            lines_for_display_[current_line_]);
        if (first_line_ < maxFirstLine()) ++first_line_;
        saveFile();
        return true;
    });

    dialog.addOption("Insert line before");
    handlers.push_back([&]() {
        lines_.emplace(lines_.begin() + current_line_);
        lines_for_display_.emplace(lines_for_display_.begin() + current_line_);
        ++current_line_;
        if (current_line_ == first_line_) --first_line_;
        saveFile();
        return true;
    });

    dialog.addOption("Insert line after");
    handlers.push_back([&]() {
        lines_.emplace(lines_.begin() + current_line_ + 1);
        lines_for_display_.emplace(
            lines_for_display_.begin() + current_line_ + 1);
        if (first_line_ < maxFirstLine()) ++first_line_;
        saveFile();
        return true;
    });

    dialog.addOption("Remove line");
    handlers.push_back([&]() {
        lines_.erase(lines_.begin() + current_line_);
        lines_for_display_.erase(lines_for_display_.begin() + current_line_);
        if (lines_.empty()) {
            lines_.emplace_back();
            lines_for_display_.emplace_back();
        }
        if (current_line_ == lines_.size()) --current_line_;
        saveFile();
        return true;
    });

    dialog.init();
    int dialog_result = dialog.execute();
    if (dialog_result > 0 && dialog_result <= handlers.size())
        handlers[dialog_result - 1]();
    return true;
}

void TextViewer::saveFile()
{
    std::ofstream out(filename_, std::ofstream::trunc);
    for (const auto &line : lines_) out << line << "\n";
}
