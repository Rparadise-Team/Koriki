#ifndef TEXT_VIEWER_H_
#define TEXT_VIEWER_H_

#include <string>

#include "sdl_ptrs.h"
#include "sdl_ttf_multifont.h"
#include "window.h"

class TextViewer : public CWindow {
  public:
    explicit TextViewer(std::string filename);
    virtual ~TextViewer() = default;

    TextViewer(const TextViewer &) = delete;
    TextViewer &operator=(const TextViewer &) = delete;

  private:
    void init();

    void render(const bool focused) const override;
    const bool keyPress(const SDL_Event &event) override;
    const bool keyHold() override;
    bool mouseDown(int button, int x, int y) override;
    bool mouseWheel(int dx, int dy) override;

    void onResize() override;
    bool isFullScreen() const override { return true; }

    // Returns the viewport line index at the given coordinates or -1.
    int getLineAt(int x, int y) const;
    int maxFirstLine() const;

    // Scroll:
    bool moveUp(unsigned step);
    bool moveDown(unsigned step);
    bool moveLeft();
    bool moveRight();

    // Open line editing dialog for the currently highlighted line.
    bool editLine();
    void saveFile();

    const Fonts &fonts_;
    std::string filename_;
    SDLSurfaceUniquePtr background_;
    SDLSurfaceUniquePtr image_;
    SDL_Rect clip_;

    // Colors:
    std::uint32_t border_color_;
    std::uint32_t bg_color_;
    SDL_Color sdl_bg_color_;
    std::uint32_t highlight_color_;
    SDL_Color sdl_highlight_color_;

    // Text mode:
    std::vector<std::string> lines_;
    std::vector<std::string> lines_for_display_;
    std::size_t first_line_;
    std::size_t current_line_;
};

#endif // TEXT_VIEWER_H_
