#include <iostream>

#include "config.h"
#include "dialog.h"
#include "screen.h"
#include "sdlutils.h"
#include "resourceManager.h"
#include "def.h"

CDialog::CDialog(const std::string &p_title, std::function<Sint16()> x_fn,
        std::function<Sint16()> y_fn):
    CWindow(),
    m_borderColor({COLOR_BORDER}),
    m_nbTitle(false),
    m_nbLabels(0),
    m_nbOptions(0),
    m_titleImg(NULL),
    m_highlightedLine(0),
    m_image(NULL),
    m_cursor1(NULL),
    m_cursor2(NULL),
    m_x_fn(std::move(x_fn)),
    m_y_fn(std::move(y_fn)),
    m_cursorX(0),
    m_cursorY(0),
    m_fonts(CResourceManager::instance().getFonts())
{
    // Title
    if (!p_title.empty())
    {
        m_nbTitle = true;
        m_lines.push_back(p_title);
    }
    // Init clip
    m_clip.x = 0;
    m_clip.y = 0;
    m_clip.w = 0;
    m_clip.h = 0;
}

CDialog::~CDialog(void)
{
    freeResources();
}

void CDialog::freeResources()
{
    for (auto *surface_ptr : { &m_titleImg, &m_image, &m_cursor1, &m_cursor2 })
    {
        if (*surface_ptr == nullptr) continue;
        SDL_FreeSurface(*surface_ptr);
        *surface_ptr = NULL;
    }
    for (auto *surfaces : { &m_linesImg, &m_linesImgCursor1, &m_linesImgCursor2 })
    {
        for (auto *surface_ptr : *surfaces)
            if (surface_ptr != nullptr) SDL_FreeSurface(surface_ptr);
        surfaces->clear();
    }
}

void CDialog::addLabel(const std::string &p_label)
{
    m_lines.push_back(p_label);
    ++m_nbLabels;
}

void CDialog::addOption(const std::string &p_option)
{
    m_lines.push_back(p_option);
    ++m_nbOptions;
}

void CDialog::init(void)
{
    border_x_ = static_cast<int>(DIALOG_BORDER * screen.ppu_x);
    border_y_ = static_cast<int>(DIALOG_BORDER * screen.ppu_y);
    padding_x_ = static_cast<int>(DIALOG_PADDING * screen.ppu_x);
    line_height_ = LINE_HEIGHT_PHYS;
    width_ = 0;

    // The width of the window depends on the width of the largest line
    int cursor_width;

    // Title
    auto l_it = m_lines.begin();
    if (m_nbTitle) {
        m_titleImg = SDL_utils::renderText(m_fonts, *l_it, Globals::g_colorTextTitle, m_borderColor);
        // m_titleImg is nullptr when text has zero width.
        width_ = m_titleImg != nullptr ? m_titleImg->w : 0;
        ++l_it;
    }

    // Render every line
    const std::size_t num_non_title_lines = m_lines.size() - (m_nbTitle ? 1 : 0);
    m_linesImg.reserve(num_non_title_lines);

    SDL_Color label_bg{COLOR_BG_1};
    if (m_nbOptions > 1) label_bg = SDL_Color{COLOR_BG_2};
    const SDL_Color option_bg{COLOR_BG_1};

    for (int i = 0; i < m_nbLabels; ++i, ++l_it) {
        m_linesImg.push_back(SDL_utils::renderText(m_fonts, *l_it, Globals::g_colorTextNormal, label_bg));
        if (m_linesImg.back() != nullptr && m_linesImg.back()->w > width_)
            width_ = m_linesImg.back()->w;
    }

    m_linesImgCursor1.reserve(m_nbOptions);
    m_linesImgCursor2.reserve(m_nbOptions);
    for (int i = 0; i < m_nbOptions; ++i, ++l_it) {
        m_linesImg.push_back(SDL_utils::renderText(m_fonts, *l_it, Globals::g_colorTextNormal, option_bg));
        m_linesImgCursor1.push_back(SDL_utils::renderText(m_fonts, *l_it, Globals::g_colorTextNormal, {COLOR_CURSOR_1}));
        m_linesImgCursor2.push_back(SDL_utils::renderText(m_fonts, *l_it, Globals::g_colorTextNormal, {COLOR_CURSOR_2}));
        if (m_linesImg.back() != nullptr && m_linesImg.back()->w > width_)
            width_ = m_linesImg.back()->w;
    }

    // Cursor width
    width_ += 2 * padding_x_;
    cursor_width = std::min(width_, screen.actual_w - 2 * border_x_);

    // Line clip
    for (auto *img : m_linesImg)
    {
        if (img == nullptr) continue;
        m_clip.h = img->h;
        break;
    }
    m_clip.w = cursor_width - 2 * padding_x_;
    // Adjust image width
    width_
        = std::min(width_ + 2 * border_x_, static_cast<int>(screen.actual_w));

    // Create dialog image
    height_ = m_lines.size() * line_height_ + 2 * border_y_;

    m_image = SDL_utils::createImage(width_, height_,
        SDL_MapRGB(screen.surface->format, m_borderColor.r, m_borderColor.g,
            m_borderColor.b));
    if (m_nbLabels > 0) {
        SDL_Rect rect = SDL_utils::makeRect(border_x_,
            border_y_ + (m_nbTitle ? 1 : 0) * line_height_,
            m_image->w - 2 * border_x_, m_nbLabels * line_height_);
        SDL_FillRect(m_image, &rect, SDL_utils::mapRGB(m_image->format, label_bg));
    }
    {
        SDL_Rect rect = SDL_utils::makeRect(border_x_,
            border_y_ + ((m_nbTitle ? 1 : 0) + m_nbLabels) * line_height_,
            m_image->w - 2 * border_x_, m_nbOptions * line_height_);
        SDL_FillRect(m_image, &rect, SDL_MapRGB(m_image->format, COLOR_BG_1));
    }
    // Create cursor image
    m_cursor1 = SDL_utils::createImage(cursor_width, line_height_, SDL_MapRGB(screen.surface->format, COLOR_CURSOR_1));
    m_cursor2 = SDL_utils::createImage(cursor_width, line_height_, SDL_MapRGB(screen.surface->format, COLOR_CURSOR_2));

    // Adjust dialog coordinates
    m_x = m_x_fn ? m_x_fn() : (screen.actual_w - m_image->w) / 2;
    if (!m_y_fn) {
        m_y = (screen.actual_h - height_) / 2;
    }
    else
    {
        m_y = m_y_fn();

        // Ensure the dialog fits vertically regardless of the requested
        // coordinates.
        m_y = std::max(m_y - (height_ + line_height_) / 2, Y_LIST_PHYS);
        m_y = std::min(m_y, screen.actual_h - FOOTER_H_PHYS + 1 - height_);
    }
    // Cursor coordinates
    m_cursorX = m_x + border_x_;
    m_cursorY = m_y + border_y_ + (m_nbTitle + m_nbLabels) * line_height_;
}

void CDialog::onResize()
{
    freeResources();
    init();
}

void CDialog::render(const bool p_focus) const
{
    INHIBIT(std::cout << "CDialog::render  fullscreen: " << isFullScreen() << "  focus: " << p_focus << std::endl;)
    // Draw background
    SDL_utils::applyPpuScaledSurface(m_x, m_y, m_image, screen.surface);
    // Draw cursor
    SDL_utils::applyPpuScaledSurface(m_cursorX,
        m_cursorY + m_highlightedLine * line_height_,
        p_focus ? m_cursor1 : m_cursor2, screen.surface);
    // Draw lines text
    const int text_x = m_cursorX + padding_x_;
    int text_y = static_cast<int>(m_y) + static_cast<int>(4 * screen.ppu_y);
    if (m_nbTitle) {
        SDL_utils::applyPpuScaledSurface(text_x, text_y - 1, m_titleImg, screen.surface, &m_clip);
        text_y += line_height_;
    }
    for (int i = 0; i < m_linesImg.size(); ++i, text_y += line_height_) {
        SDL_Surface *surface;
        if (i == m_nbLabels + m_highlightedLine) {
            surface = p_focus ? m_linesImgCursor1[i - m_nbLabels] : m_linesImgCursor2[i - m_nbLabels];
        } else {
            surface = m_linesImg[i];
        }
        SDL_utils::applyPpuScaledSurface(text_x, text_y, surface, screen.surface, &m_clip);
    }
}

const bool CDialog::keyPress(const SDL_Event &p_event)
{
    CWindow::keyPress(p_event);
    const auto &c = config();
    const auto sym = p_event.key.keysym.sym;
    if (sym == c.key_parent || sym == c.key_system) {
        m_retVal = -1;
        return true;
    }
    if (sym == c.key_up) return moveCursorUp(/*p_loop=*/true);
    if (sym == c.key_down) return moveCursorDown(/*p_loop=*/true);
    if (sym == c.key_pageup) {
        if (m_highlightedLine == 0) return false;
        m_highlightedLine = 0;
        return true;
    }
    if (sym == c.key_pagedown) {
        if (m_highlightedLine + 1 < m_nbOptions) {
            m_highlightedLine = m_nbOptions - 1;
            return true;
        }
        return false;
    }
    if (sym == c.key_open || sym == c.key_operation) {
        m_retVal = static_cast<int>(m_highlightedLine + 1);
        return true;
    }
    return false;
}

int CDialog::getLineAt(int x, int y) const
{
    const int x0 = m_x + border_x_;
    const int x1 = x0 + width_ - 2 * border_x_;
    const int y0 = m_y + border_y_ + (m_nbTitle + m_nbLabels) * line_height_;
    const int y1 = y0 + m_nbOptions * line_height_;
    if (x < x0 || x > x1 || y < y0 || y > y1) return -1;
    return (y - y0) / line_height_;
}

bool CDialog::mouseWheel(int dx, int dy) {
    if (dy > 0) return moveCursorUp(/*p_loop=*/false);
    if (dy < 0) return moveCursorDown(/*p_loop=*/false);
    return false;
}

bool CDialog::mouseDown(int button, int x, int y) {
    if (x < m_x || x > m_x + width_ || y < m_y || y > m_y + height_) {
        m_retVal = -1;
        return true;
    }
    const int line = getLineAt(x, y);
    if (line == -1) return false;
    switch (button)
    {
        case SDL_BUTTON_LEFT:
            m_highlightedLine = line;
            m_retVal = m_highlightedLine + 1;
            return true;
        case SDL_BUTTON_MIDDLE:
        case SDL_BUTTON_RIGHT:
            m_highlightedLine = line;
            return true;
        case SDL_BUTTON_X1: m_retVal = -1; return true;
    }
    return false;
}

const bool CDialog::moveCursorUp(const bool p_loop)
{
    bool l_ret(false);
    if (m_highlightedLine)
    {
        --m_highlightedLine;
        l_ret = true;
    }
    else if (p_loop && m_highlightedLine + 1 < m_nbOptions)
    {
        m_highlightedLine = m_nbOptions - 1;
        l_ret = true;
    }
    return l_ret;
}

const bool CDialog::moveCursorDown(const bool p_loop)
{
    bool l_ret(false);
    if (m_highlightedLine + 1 < m_nbOptions)
    {
        ++m_highlightedLine;
        l_ret = true;
    }
    else if (p_loop && m_highlightedLine)
    {
        m_highlightedLine = 0;
        l_ret = true;
    }
    return l_ret;
}

const bool CDialog::keyHold(void)
{
    const auto &c = config();
    if (m_lastPressed == c.key_up)
        return tick(c.key_up) && moveCursorUp(/*p_loop=*/false);
    if (m_lastPressed == c.key_down)
        return tick(c.key_down) && moveCursorDown(/*p_loop=*/false);
    return false;
}

const unsigned int &CDialog::getHighlightedIndex(void) const
{
    return m_highlightedLine;
}
