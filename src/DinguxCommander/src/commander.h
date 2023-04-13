#ifndef _COMMANDER_H_
#define _COMMANDER_H_

#include <SDL.h>
#include "panel.h"
#include "sdl_ttf_multifont.h"
#include "window.h"

class CCommander : public CWindow
{
    public:

    // Constructor
    CCommander(const std::string &p_pathL, const std::string &p_pathR);

    // Destructor
    virtual ~CCommander(void);

    private:

    // Forbidden
    CCommander(void);
    CCommander(const CCommander &p_source);
    const CCommander &operator =(const CCommander &p_source);

    // Window resized.
    void onResize() override;

    // Key press management
    const bool keyPress(const SDL_Event &p_event) override;

    // Key hold management
    const bool keyHold(void) override;

    bool mouseDown(int button, int x, int y) override;
    bool mouseWheel(int dx, int dy) override;

    CPanel* focusPanelAt(int *x, int *y, bool *changed);

    // Draw
    virtual void render(const bool p_focus) const;

    // Is window full screen?
    virtual bool isFullScreen(void) const;

    // Open the file operation menus
    bool itemMenu() const;
    bool operationMenu() const;

    const bool openCopyMenu(void) const;
    void openExecuteMenu(void) const;

    // Open the selection menu
    const bool openSystemMenu(void);

    // The two panels
    CPanel m_panelLeft;
    CPanel m_panelRight;
    CPanel* m_panelSource;
    CPanel* m_panelTarget;

    SDL_Surface *m_background;
};

#endif
