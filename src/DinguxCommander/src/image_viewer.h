#ifndef IMAGE_VIEWER_H_
#define IMAGE_VIEWER_H_

#include <string>

#include "sdl_ptrs.h"
#include "window.h"

class ImageViewer : public CWindow {
  public:
    explicit ImageViewer(std::string filename);
    virtual ~ImageViewer() = default;

    ImageViewer(const ImageViewer &) = delete;
    ImageViewer &operator=(const ImageViewer &) = delete;

    bool ok() const { return ok_; }

  private:
    void init();
    void render(const bool focused) const override;
    const bool keyPress(const SDL_Event &event) override;
    void onResize() override;
    bool isFullScreen() const override { return true; }

    std::string filename_;
    SDLSurfaceUniquePtr image_;
    SDLSurfaceUniquePtr background_;
    bool ok_;
};

#endif // IMAGE_VIEWER_H_
