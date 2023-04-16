#ifndef _FILEUTILS_H_
#define _FILEUTILS_H_

#include <string>
#include <vector>

namespace File_utils
{
    // File operations

    void copyFile(const std::vector<std::string> &p_src, const std::string &p_dest);

    void moveFile(const std::vector<std::string> &p_src, const std::string &p_dest);

    void symlinkFile(const std::vector<std::string> &p_src, const std::string &p_dest);

    void removeFile(const std::vector<std::string> &p_files);

    void executeFile(const std::string &p_file);

    void makeDirectory(const std::string &p_file);

    void renameFile(const std::string &p_file1, const std::string &p_file2);

    // File utilities

    const bool fileExists(const std::string &p_path);

    std::string getLowercaseFileExtension(const std::string &name);

    const unsigned long int getFileSize(const std::string &p_file);

    void formatSize(std::string &p_size);

    const std::string getFileName(const std::string &p_path);

    const std::string getPath(const std::string &p_path);

    void stringReplace(std::string &p_string, const std::string &p_search, const std::string &p_replace);

    // Dialogs

    void diskInfo(void);

    void diskUsed(const std::vector<std::string> &p_files);
}

#endif
