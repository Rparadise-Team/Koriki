#include <iostream>
#include <sstream>
#include "panel.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"
#include "fileutils.h"

CPanel::CPanel(const std::string &p_path, const Sint16 p_x):
    m_currentPath(""),
    m_camera(0),
    m_x(p_x),
    m_highlightedLine(0),
    resources_(CResourceManager::instance()),
    m_fonts(resources_.getFonts())
{
    // List the given path
    if (m_fileLister.list(p_path))
    {
        // Path OK
        m_currentPath = p_path;
    }
    else
    {
        // The path is wrong => take default
        m_fileLister.list(PATH_DEFAULT);
        m_currentPath = PATH_DEFAULT;
    }
}

CPanel::~CPanel(void) { }

SDL_Surface *CPanel::icon_dir() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_FOLDER);
}

SDL_Surface *CPanel::icon_file() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_FILE);
}

SDL_Surface *CPanel::icon_img() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_FILE_IMAGE);
}

SDL_Surface *CPanel::icon_ipk() const
{
    return resources_.getSurface(
        CResourceManager::T_SURFACE_FILE_INSTALLABLE_PACKAGE);
}

SDL_Surface *CPanel::icon_opk() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_FILE_PACKAGE);
}

SDL_Surface *CPanel::icon_symlink() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_FILE_IS_SYMLINK);
}

SDL_Surface *CPanel::icon_up() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_UP);
}

SDL_Surface *CPanel::cursor1() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_CURSOR1);
}

SDL_Surface *CPanel::cursor2() const
{
    return resources_.getSurface(CResourceManager::T_SURFACE_CURSOR2);
}

int CPanel::list_y() const { return Y_LIST_PHYS; }

int CPanel::line_height() const
{
    return LINE_HEIGHT_PHYS;
}

int CPanel::header_height() const
{
    return HEADER_H_PHYS;
}

int CPanel::header_padding_top() const
{
    return HEADER_PADDING_TOP_PHYS;
}

int CPanel::footer_height() const
{
    return FOOTER_H_PHYS;
}

int CPanel::footer_y() const { return screen.actual_h - footer_height(); }

int CPanel::footer_padding_top() const
{
    return FOOTER_PADDING_TOP_PHYS;
}

int CPanel::width() const
{
    return (screen.actual_w - static_cast<int>(2 * screen.ppu_x)) / 2;
}

int CPanel::list_height() const
{
    return screen.actual_h - header_height() - footer_height();
}

void CPanel::render(const bool p_active) const
{
    // Draw panel
    const int l_x = m_x + icon_dir()->w + 2 * screen.ppu_x;
    const unsigned int l_nbTotal = m_fileLister.getNbTotal();
    int l_y = list_y();
    SDL_Surface *l_surfaceTmp = NULL;
    const SDL_Color *l_color = NULL;
    SDL_Rect l_rect;
    // Current dir
    l_surfaceTmp = SDL_utils::renderText(
        m_fonts, m_currentPath, Globals::g_colorTextTitle, { COLOR_TITLE_BG });
    if (l_surfaceTmp->w > width()) {
        l_rect.x = l_surfaceTmp->w - width();
        l_rect.y = 0;
        l_rect.w = width();
        l_rect.h = l_surfaceTmp->h;
        SDL_utils::applyPpuScaledSurface(
            m_x, header_padding_top(), l_surfaceTmp, screen.surface, &l_rect);
    } else {
        SDL_utils::applyPpuScaledSurface(
            m_x, header_padding_top(), l_surfaceTmp, screen.surface);
    }

    SDL_FreeSurface(l_surfaceTmp);
    SDL_Rect clip_contents_rect = SDL_utils::Rect(0, list_y(), screen.actual_w, list_height());
    // Content
    SDL_SetClipRect(screen.surface, &clip_contents_rect);
    // Draw cursor
    SDL_utils::applyPpuScaledSurface(m_x - static_cast<int>(1 * screen.ppu_x),
        list_y() + (m_highlightedLine - m_camera) * line_height(),
        p_active ? cursor1() : cursor2(), screen.surface);
    for (unsigned int l_i = m_camera;
         l_i < m_camera + NB_VISIBLE_LINES && l_i < l_nbTotal; ++l_i) {
        // Icon and color
        if (m_fileLister.isDirectory(l_i))
        {
            // Icon
            if (m_fileLister[l_i].m_name == "..")
                l_surfaceTmp = icon_up();
            else
                l_surfaceTmp = icon_dir();
            // Color
            if (m_selectList.find(l_i) != m_selectList.end())
                l_color = &Globals::g_colorTextSelected;
            else
                l_color = &Globals::g_colorTextDir;
        }
        else
        {
            // Icon
            const std::string &ext = m_fileLister[l_i].m_ext;
            if (SDL_utils::isSupportedImageExt(ext))
                l_surfaceTmp = icon_img();
            else if (ext == "ipk")
                l_surfaceTmp = icon_ipk();
            else if (ext == "opk")
                l_surfaceTmp = icon_opk();
            else
                l_surfaceTmp = icon_file();
            // Color
            if (m_selectList.find(l_i) != m_selectList.end())
                l_color = &Globals::g_colorTextSelected;
            else
                l_color = &Globals::g_colorTextNormal;
        }
        SDL_utils::applyPpuScaledSurface(m_x, l_y, l_surfaceTmp, screen.surface);
        if (m_fileLister[l_i].is_symlink)
            SDL_utils::applyPpuScaledSurface(m_x, l_y, icon_symlink(), screen.surface);
        // Text
        SDL_Color l_bg;
        if (l_i == m_highlightedLine) {
            if (p_active)
                l_bg = {COLOR_CURSOR_1};
            else
                l_bg = {COLOR_CURSOR_2};
        } else {
            static const SDL_Color kLineBg[2] = {{COLOR_BG_1}, {COLOR_BG_2}};
            l_bg = kLineBg[(l_i - m_camera) % 2];
        }
        l_surfaceTmp = SDL_utils::renderText(m_fonts, m_fileLister[l_i].m_name, *l_color, l_bg);
        const int max_name_width = width() - static_cast<int>(18 * screen.ppu_x);
        SDL_Rect *text_clip_rect = nullptr;
        if (l_surfaceTmp->w > max_name_width)
        {
            l_rect.x = 0;
            l_rect.y = 0;
            l_rect.w = max_name_width;
            l_rect.h = l_surfaceTmp->h;
            text_clip_rect = &l_rect;
        }
        SDL_utils::applyPpuScaledSurface(l_x,
            l_y + static_cast<int>(2 * screen.ppu_y), l_surfaceTmp,
            screen.surface, text_clip_rect);
        SDL_FreeSurface(l_surfaceTmp);
        // Next line
        l_y += line_height();
    }
    SDL_SetClipRect(screen.surface, nullptr);

    // Footer
    std::string l_footer("-");
    if (!m_fileLister.isDirectory(m_highlightedLine))
    {
        std::ostringstream l_s;
        l_s << m_fileLister[m_highlightedLine].m_size;
        l_footer = l_s.str();
        File_utils::formatSize(l_footer);
    }
    SDL_utils::applyPpuScaledText(m_x + static_cast<int>(2 * screen.ppu_x),
        footer_y() + footer_padding_top(), screen.surface, m_fonts,
        "Size:", Globals::g_colorTextTitle, { COLOR_TITLE_BG });
    SDL_utils::applyPpuScaledText(
        m_x + width() - static_cast<int>(2 * screen.ppu_x),
        footer_y() + footer_padding_top(), screen.surface, m_fonts, l_footer,
        Globals::g_colorTextTitle, { COLOR_TITLE_BG },
        SDL_utils::T_TEXT_ALIGN_RIGHT);
}

const bool CPanel::moveCursorUp(unsigned char p_step)
{
    if (m_highlightedLine)
    {
        // Move cursor
        if (m_highlightedLine > p_step)
            m_highlightedLine -= p_step;
        else
            m_highlightedLine = 0;
        // Adjust camera
        adjustCamera();
        // Return true for new render
        return true;
    }
    return false;
}

const bool CPanel::moveCursorDown(unsigned char p_step)
{
    const unsigned int l_nb = m_fileLister.getNbTotal();
    if (m_highlightedLine < l_nb - 1)
    {
        // Move cursor
        if (m_highlightedLine + p_step > l_nb - 1)
            m_highlightedLine = l_nb - 1;
        else
            m_highlightedLine += p_step;
        // Adjust camera
        adjustCamera();
        // Return true for new render
        return true;
    }
    return false;
}

int CPanel::getNumVisibleListItems() const
{
    return std::min(
        static_cast<decltype(m_fileLister.getNbTotal())>(NB_VISIBLE_LINES),
        m_fileLister.getNbTotal() - m_camera);
}

int CPanel::getLineAt(int x, int y) const
{
    if (x < 0 || y < list_y()) return -1;
    const int y0 = list_y();
    const int line_h = line_height();
    if (y - y0 >= getNumVisibleListItems() * line_h) return -1;
    return (y - y0) / line_h;
}

void CPanel::moveCursorToVisibleLineIndex(int index)
{
    m_highlightedLine = m_camera + index;
}

const bool CPanel::open(const std::string &p_path)
{
    bool l_ret(false);
    std::string l_newPath("");
    std::string l_oldDir("");
    if (p_path.empty())
    {
        // Open highlighted dir
        if (m_fileLister[m_highlightedLine].m_name == "..")
        {
            // Go to parent dir
            size_t l_pos = m_currentPath.rfind('/');
            // Remove the last dir in the path
            l_newPath = m_currentPath.substr(0, l_pos);
            if (l_newPath.empty())
                // We're at /
                l_newPath = "/";
            l_oldDir = m_currentPath.substr(l_pos + 1);
        }
        else
        {
            l_newPath = m_currentPath + (m_currentPath == "/" ? "" : "/") + m_fileLister[m_highlightedLine].m_name;
        }
    }
    else
    {
        // Open given dir
        if (p_path == m_currentPath)
            return false;
        l_newPath = p_path;
    }
    // List the new path
    if (m_fileLister.list(l_newPath))
    {
        // Path OK
        m_currentPath = l_newPath;
        // If it's a back movement, restore old dir
        if (!l_oldDir.empty())
            m_highlightedLine = m_fileLister.searchDir(l_oldDir);
        else
            m_highlightedLine = 0;
        // Camera
        adjustCamera();
        // Clear select list
        m_selectList.clear();
        // New render
        l_ret = true;
    }
    INHIBIT(std::cout << "open - new current path: " << m_currentPath << std::endl;)
    return l_ret;
}

const bool CPanel::goToParentDir(void)
{
    bool l_ret(false);
    // Select ".." and open it
    if (m_currentPath != "/")
    {
        m_highlightedLine = 0;
        l_ret = open();
    }
    return l_ret;
}

void CPanel::adjustCamera(void)
{
    if (m_fileLister.getNbTotal() <= NB_VISIBLE_LINES)
        m_camera = 0;
    else if (m_highlightedLine < m_camera)
        m_camera = m_highlightedLine;
    else if (m_highlightedLine > m_camera + NB_FULLY_VISIBLE_LINES - 1)
        m_camera = m_highlightedLine - NB_FULLY_VISIBLE_LINES + 1;
}

const std::string &CPanel::getHighlightedItem(void) const
{
    return m_fileLister[m_highlightedLine].m_name;
}

const std::string CPanel::getHighlightedItemFull(void) const
{
    return m_currentPath + (m_currentPath == "/" ? "" : "/") + m_fileLister[m_highlightedLine].m_name;
}

const std::string &CPanel::getCurrentPath(void) const
{
    return m_currentPath;
}

const unsigned int &CPanel::getHighlightedIndex(void) const
{
    return m_highlightedLine;
}

const unsigned int CPanel::getHighlightedIndexRelative(void) const
{
    return m_highlightedLine - m_camera;
}

void CPanel::refresh(void)
{
    // List current path
    if (m_fileLister.list(m_currentPath))
    {
        // Adjust selected line
        if (m_highlightedLine > m_fileLister.getNbTotal() - 1)
            m_highlightedLine = m_fileLister.getNbTotal() - 1;
    }
    else
    {
        // Current path doesn't exist anymore => default
        m_fileLister.list(PATH_DEFAULT);
        m_currentPath = PATH_DEFAULT;
        m_highlightedLine = 0;
    }
    // Camera
    adjustCamera();
    // Clear select list
    m_selectList.clear();
}

const bool CPanel::addToSelectList(const bool p_step)
{
    if (m_fileLister[m_highlightedLine].m_name != "..")
    {
        // Search highlighted element in select list
        std::set<unsigned int>::iterator l_it = m_selectList.find(m_highlightedLine);
        if (l_it == m_selectList.end())
            // Element not present => we add it
            m_selectList.insert(m_highlightedLine);
        else
            // Element present => we remove it from the list
            m_selectList.erase(m_highlightedLine);
        if (p_step)
            moveCursorDown(1);
        return true;
    }
    else
    {
        return false;
    }
}

const std::set<unsigned int> &CPanel::getSelectList(void) const
{
    return m_selectList;
}

void CPanel::getSelectList(std::vector<std::string> &p_list) const
{
    p_list.clear();
    // Insert full path of selected files
    for (std::set<unsigned int>::const_iterator l_it = m_selectList.begin(); l_it != m_selectList.end(); ++l_it)
    {
        if (m_currentPath == "/")
            p_list.push_back(m_currentPath + m_fileLister[*l_it].m_name);
        else
            p_list.push_back(m_currentPath + "/" + m_fileLister[*l_it].m_name);
    }
}

void CPanel::selectAll(void)
{
    const unsigned int l_nb = m_fileLister.getNbTotal();
    for (unsigned int l_i = 1; l_i < l_nb; ++l_i)
        m_selectList.insert(l_i);
}

void CPanel::selectNone(void)
{
    m_selectList.clear();
}

const bool CPanel::isDirectoryHighlighted(void) const
{
    return m_fileLister.isDirectory(m_highlightedLine);
}
