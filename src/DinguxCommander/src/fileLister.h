#ifndef _FILE_LISTER_H_
#define _FILE_LISTER_H_

#include <vector>
#include <string>
#include "fileutils.h"

// Class used to store file info
struct T_FILE
{
    T_FILE(void) : m_size(0) {}
    T_FILE(std::string p_name, bool is_symlink, unsigned long int p_size = 0)
        : m_name(std::move(p_name)),
          is_symlink(is_symlink),
          m_ext(File_utils::getLowercaseFileExtension(m_name)),
          m_size(p_size) {}
    T_FILE(const T_FILE &p_source) = default;
    T_FILE &operator=(const T_FILE &p_source) = default;
    std::string m_name;
    std::string m_ext;
    bool is_symlink;
    unsigned long int m_size;
};

class CFileLister
{
    public:

    // Constructor
    CFileLister(void);

    // Destructor
    virtual ~CFileLister(void);

    // Read the contents of the given path
    // Returns false if the path does not exist
    const bool list(const std::string &p_path);

    // Get an element in the list (dirs and files combined)
    const T_FILE &operator[](const unsigned int p_i) const;

    // Get the number of dirs/files
    const unsigned int getNbDirs(void) const;
    const unsigned int getNbFiles(void) const;
    const unsigned int getNbTotal(void) const;

    // True => directory, false => file
    const bool isDirectory(const unsigned int p_i) const;

    // Get index of the given dir name, 0 if not found
    const unsigned int searchDir(const std::string &p_name) const;

    private:

    // Forbidden
    CFileLister(const CFileLister &p_source);
    const CFileLister &operator =(const CFileLister &p_source);

    // The list of files/dir
    std::vector<T_FILE> m_listDirs;
    std::vector<T_FILE> m_listFiles;
};

#endif
