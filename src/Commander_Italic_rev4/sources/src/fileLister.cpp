#include "fileLister.h"

#include <algorithm>
#include <iostream>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "file_info.h"
#include "sdlutils.h"

bool compareNoCase(const T_FILE& p_s1, const T_FILE& p_s2)
{
    return strcasecmp(p_s1.m_name.c_str(), p_s2.m_name.c_str()) <= 0;
}

CFileLister::CFileLister(void)
{
}

CFileLister::~CFileLister(void)
{
}

const bool CFileLister::list(const std::string &p_path)
{
    // Open dir
    DIR *l_dir = opendir(p_path.c_str());
    if (l_dir == NULL)
    {
        std::cerr << "CFileLister::list: Error opening dir " << p_path << std::endl;
        return false;
    }
    // Clean up
    m_listFiles.clear();
    m_listDirs.clear();
    // Read dir
    std::string l_file("");
    std::string l_fileFull("");
    struct stat st;
    struct dirent *l_dirent = readdir(l_dir);
    while (l_dirent != NULL)
    {
        l_file = l_dirent->d_name;
        // Filter the '.' and '..' dirs
        if (l_file != "." && l_file != "..")
        {
            l_fileFull = p_path + "/" + l_file;
            auto info = FileInfo::Get(l_fileFull);
            auto tfile = T_FILE(l_file, info.symlink(), info.size());
            if (info.directory())
                m_listDirs.push_back(std::move(tfile));
            else
                m_listFiles.push_back(std::move(tfile));
        }
        // Next
        l_dirent = readdir(l_dir);
    }
    // Close dir
    closedir(l_dir);
    // Sort lists
    sort(m_listFiles.begin(), m_listFiles.end(), compareNoCase);
    sort(m_listDirs.begin(), m_listDirs.end(), compareNoCase);
    // Add "..", always at the first place
    m_listDirs.insert(m_listDirs.begin(), T_FILE("..", /*is_symlink=*/false, 0));
    return true;
}

const T_FILE &CFileLister::operator[](const unsigned int p_i) const
{
    if (p_i < m_listDirs.size())
        return m_listDirs[p_i];
    else
        return m_listFiles[p_i - m_listDirs.size()];
}

const unsigned int CFileLister::getNbDirs(void) const
{
    return m_listDirs.size();
}

const unsigned int CFileLister::getNbFiles(void) const
{
    return m_listFiles.size();
}

const unsigned int CFileLister::getNbTotal(void) const
{
    return m_listDirs.size() + m_listFiles.size();
}

const bool CFileLister::isDirectory(const unsigned int p_i) const
{
    return p_i < m_listDirs.size();
}

const unsigned int CFileLister::searchDir(const std::string &p_name) const
{
    unsigned int l_ret = 0;
    bool l_found = false;
    // Search name in dirs
    for (std::vector<T_FILE>::const_iterator l_it = m_listDirs.begin(); (!l_found) && (l_it != m_listDirs.end()); ++l_it)
    {
        if ((*l_it).m_name == p_name)
            l_found = true;
        else
            ++l_ret;
    }
    return l_found ? l_ret : 0;
}
