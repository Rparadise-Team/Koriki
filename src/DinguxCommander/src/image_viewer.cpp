#include "image_viewer.h"

#include "config.h"
#include "def.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"
#include "text_viewer.h"

#define VIEWER_MARGIN 1

ImageViewer::ImageViewer(std::string filename)
    : filename_(std::move(filename))
{
    init();
}

void ImageViewer::init()
{
    image_ = SDLSurfaceUniquePtr { SDL_utils::loadImageToFit(
        filename_, screen.w, screen.h) };
    ok_ = (image_ != nullptr);
    if (!ok_) return;

    const auto &fonts = CResourceManager::instance().getFonts();
    bool show_title = image_->h <= screen.actual_h - Y_LIST * screen.ppu_y;

    // Create background image
    background_ = SDLSurfaceUniquePtr { SDL_utils::createImage(screen.actual_w,
        screen.actual_h, SDL_MapRGB(screen.surface->format, COLOR_BG_1)) };
    if (show_title) {
        SDL_Rect rect = SDL_utils::Rect(
            0, 0, screen.actual_w, HEADER_H * screen.ppu_y);
        SDL_FillRect(background_.get(), &rect,
            SDL_MapRGB(background_->format, COLOR_BORDER));
    }
    // Print title
    if (show_title) {
        SDLSurfaceUniquePtr tmp { SDL_utils::renderText(
            fonts, filename_, Globals::g_colorTextTitle, { COLOR_TITLE_BG }) };
        if (tmp->w > background_->w - 2 * VIEWER_MARGIN) {
            SDL_Rect rect;
            rect.x = tmp->w - (background_->w - 2 * VIEWER_MARGIN);
            rect.y = 0;
            rect.w = background_->w - 2 * VIEWER_MARGIN;
            rect.h = tmp->h;
            SDL_utils::applyPpuScaledSurface(VIEWER_MARGIN * screen.ppu_x,
                HEADER_PADDING_TOP * screen.ppu_y, tmp.get(), background_.get(),
                &rect);
        } else {
            SDL_utils::applyPpuScaledSurface(VIEWER_MARGIN * screen.ppu_x,
                HEADER_PADDING_TOP * screen.ppu_y, tmp.get(),
                background_.get());
        }
    }

    // Transparency grid background.
    constexpr int kTransparentBgRectSize = 10;
    const Uint32 colors[2] = {
        SDL_MapRGB(background_->format, COLOR_BG_1),
        SDL_MapRGB(background_->format, COLOR_BG_2),
    };
    int j = 0;
    const int rect_w = static_cast<int>(kTransparentBgRectSize * screen.ppu_x);
    const int rect_h = static_cast<int>(kTransparentBgRectSize * screen.ppu_y);
    for (int j = 0, y = show_title ? Y_LIST * screen.ppu_y : 0; y < screen.actual_h; y += rect_h, ++j) {
        for (int i = 0, x = 0; x < screen.actual_w; x += rect_w, ++i) {
            SDL_Rect rect = SDL_utils::makeRect(x, y, rect_w, rect_h);
            SDL_FillRect(background_.get(), &rect, colors[(i + j) % 2]);
        }
    }

    SDL_Rect frame_left_rect = SDL_utils::makeRect(screen.actual_w - 1, 0, 1, screen.actual_h);
    SDL_Rect frame_bottom_rect = SDL_utils::makeRect(0, screen.actual_h - 1, screen.actual_w, 1);
    SDL_FillRect(background_.get(), &frame_left_rect, 0);
    SDL_FillRect(background_.get(), &frame_bottom_rect, 0);
}

void ImageViewer::onResize()
{
    image_ = nullptr;
    init();
}

void ImageViewer::render(const bool focused) const
{
    SDL_utils::applyPpuScaledSurface(0, 0, background_.get(), screen.surface);
    int pos_y = image_->h <= screen.actual_h - Y_LIST * screen.ppu_y ?
        (Y_LIST * screen.ppu_y
            + (screen.actual_h - Y_LIST * screen.ppu_y - image_->h) / 2)
        : (screen.actual_h - image_->h) / 2;
    SDL_utils::applyPpuScaledSurface((screen.actual_w - image_->w) / 2,
        pos_y,
        image_.get(), screen.surface);
}

// Key press management
const bool ImageViewer::keyPress(const SDL_Event &event)
{
    CWindow::keyPress(event);
    const auto &c = config();
    const auto sym = event.key.keysym.sym;
    if (sym == c.key_system || sym == c.key_parent) {
        m_retVal = -1;
        return true;
    }
    else if (sym == c.key_down || sym == c.key_right) {
        m_retVal = 2;
        return true;
    }
    else if (sym == c.key_up || sym == c.key_left) {
        m_retVal = 1;
        return true;
    }
    return false;
}
