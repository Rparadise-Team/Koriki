#ifndef _PANEL_H_
#define _PANEL_H_

#include <string>
#include <set>
#include <SDL.h>
#include <SDL_ttf.h>

#include "def.h"
#include "fileLister.h"
#include "resourceManager.h"
#include "sdl_ttf_multifont.h"

class CPanel
{
    public:

    // Constructor
    CPanel(const std::string &p_path, const Sint16 p_x);

    // Destructor
    virtual ~CPanel(void);

    // Draw the panel on the screen
    void render(const bool p_active) const;

    // Move cursor
    const bool moveCursorUp(unsigned char p_step);
    const bool moveCursorDown(unsigned char p_step);
    void moveCursorToVisibleLineIndex(int index);

    // Returns the viewport line index at the given coordinates or -1.
    int getLineAt(int x, int y) const;

    // Open selected item
    const bool open(const std::string &p_path = "");

    // Refresh current directory
    void refresh(void);

    // Go to parent dir
    const bool goToParentDir(void);

    // Selected file with just the name
    const std::string &getHighlightedItem(void) const;

    // Selected file with full path
    const std::string getHighlightedItemFull(void) const;

    // Current path
    const std::string &getCurrentPath(void) const;

    // Selected index
    const unsigned int &getHighlightedIndex(void) const;
    const unsigned int getHighlightedIndexRelative(void) const;

    // True => directory, false => file, or dir ".."
    const bool isDirectoryHighlighted(void) const;

    // Add/remove current file to the select list
    const bool addToSelectList(const bool p_step);

    // Get select list
    const std::set<unsigned int> &getSelectList(void) const;
    void getSelectList(std::vector<std::string> &p_list) const;

    // Clear select list
    void selectAll(void);
    void selectNone(void);

    void setX(int x) { m_x = x; }

    private:

    // Forbidden
    CPanel(void);
    CPanel(const CPanel &p_source);
    const CPanel &operator =(const CPanel &p_source);

    // Returns the number of currently visible list items (lines).
    int getNumVisibleListItems() const;

    // Adjust camera
    void adjustCamera(void);

    // Resources:
    SDL_Surface *icon_dir() const;
    SDL_Surface *icon_file() const;
    SDL_Surface *icon_img() const;
    SDL_Surface *icon_ipk() const;
    SDL_Surface *icon_opk() const;
    SDL_Surface *icon_symlink() const;
    SDL_Surface *icon_up() const;
    SDL_Surface *cursor1() const;
    SDL_Surface *cursor2() const;

    int list_y() const;
    int list_height() const;
    int line_height() const;
    int header_height() const;
    int header_padding_top() const;
    int footer_y() const;
    int footer_height() const;
    int footer_padding_top() const;
    int width() const;

    // File lister
    CFileLister m_fileLister;

    // Current path
    std::string m_currentPath;

    // Index of the first displayed line
    unsigned int m_camera;

    // X coordinate
    int m_x;

    // Highlighted line
    unsigned int m_highlightedLine;

    // Selection list
    std::set<unsigned int> m_selectList;

    // Pointers to resources
    const CResourceManager &resources_;

    const Fonts &m_fonts;
};

#endif
