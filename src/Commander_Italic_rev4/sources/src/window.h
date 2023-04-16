#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <SDL.h>

#include "sdl_backports.h"

class CWindow
{
    public:

    // Destructor
    virtual ~CWindow(void);

    // Execute main loop of the window
    int execute();

    // Return value
    const int getReturnValue(void) const;

    // Draw window
    virtual void render(const bool p_focus) const = 0;

    // Is window full screen?
    virtual bool isFullScreen(void) const;

    // Call SDL_Start/StopTextInput accordingly.
    virtual bool handlesTextInput() const;

    protected:

    // Constructor
    CWindow(void);

    // Window resized.
    virtual void onResize();

    // Mouse down event.
    // Return true if re-render is needed after handling this.
    virtual bool mouseDown(int button, int x, int y);

    // Mouse wheel event.
    // `dx` - the amount scrolled horizontally, positive to the right and negative to the left.
    // `dy` - the amount scrolled vertically, positive away from the user and negative towards the user.
    // Return true if re-render is needed after handling this.
    virtual bool mouseWheel(int dx, int dy);

    // Key press management
    virtual const bool keyPress(const SDL_Event &p_event);

    // Key hold management
    virtual const bool keyHold(void);

    // SDL2 text input events: SDL_TEXTINPUT and SDL_TEXTEDITING
    virtual bool textInput(const SDL_Event &event);

    // Timer tick
    bool tick(SDLC_Keycode p_held);

    // Timer for key hold
    unsigned int m_timer;

    SDLC_Keycode m_lastPressed;

    // Return value
    int m_retVal;

    private:

    bool handleZoomTrigger(const SDL_Event &event);
    void triggerOnResize();

    // Forbidden
    CWindow(const CWindow &p_source);
    const CWindow &operator =(const CWindow &p_source);

};

#endif
