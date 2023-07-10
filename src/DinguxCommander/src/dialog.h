#ifndef _DIALOG_H_
#define _DIALOG_H_

#include <functional>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>

#include "sdl_ttf_multifont.h"
#include "window.h"

class CDialog : public CWindow
{
    public:

    // x_fn and y_fn are function that return dialog coordinates.
    // They are functions so that we can handle moving the dialog on resize.
    // Empty functions mean centered.
    CDialog(const std::string &p_title, std::function<Sint16()> x_fn = {},
        std::function<Sint16()> y_fn = {});

    // Destructor
    virtual ~CDialog(void);

    // Add a label
    void addLabel(const std::string &p_label);

    // Add a menu option
    void addOption(const std::string &p_option);

    // Sets the color of the borders and title background.
    void setBorderColor(SDL_Color color) { m_borderColor = color; }

    // Init. Call after all options are added.
    void init(void);

    // Accessors
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    const unsigned int &getHighlightedIndex(void) const;

    int border_x() const { return border_x_; };
    int border_y() const { return border_y_; };
    int width() const { return width_; };
    int height() const { return height_; };
    int line_height() const { return line_height_; };

    private:

    // Forbidden
    CDialog(void);
    CDialog(const CDialog &p_source);
    const CDialog &operator =(const CDialog &p_source);

    void onResize() override;

    // Key press management
    const bool keyPress(const SDL_Event &p_event) override;

    // Key hold management
    const bool keyHold(void) override;

    bool mouseDown(int button, int x, int y) override;
    bool mouseWheel(int dx, int dy) override;

    // Draw
    void render(const bool p_focus) const override;

    // Move cursor
    const bool moveCursorUp(const bool p_loop);
    const bool moveCursorDown(const bool p_loop);

    void freeResources();

    // Returns the line index at the given coordinates or -1.
    int getLineAt(int x, int y) const;

    // Dimensions in physical pixels.
    int border_x_, border_y_;
    int padding_x_;
    int line_height_;
    int width_, height_;

    SDL_Color m_borderColor;

    // Number of titles (0 or 1), labels, and options
    bool m_nbTitle;
    unsigned char m_nbLabels;
    unsigned char m_nbOptions;

    // List of lines
    std::vector<std::string> m_lines;
    SDL_Surface *m_titleImg;
    std::vector<SDL_Surface *> m_linesImg;
    std::vector<SDL_Surface *> m_linesImgCursor1;
    std::vector<SDL_Surface *> m_linesImgCursor2;

    // The highlighted item
    unsigned int m_highlightedLine;

    // The image representing the dialog
    SDL_Surface *m_image;

    // The cursor
    SDL_Surface *m_cursor1;
    SDL_Surface *m_cursor2;

    // Coordinates
    std::function<Sint16()> m_x_fn;
    std::function<Sint16()> m_y_fn;
    int m_x, m_y;
    int m_cursorX, m_cursorY;

    // Relative coordinates, for better resize handling.
    float relative_x;
    float relative_y;

    // Line clip
    mutable SDL_Rect m_clip;

    // Pointers to resources
    const Fonts &m_fonts;
};

#endif
