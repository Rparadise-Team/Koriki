#ifndef _COMMANDER_FILE_INFO_
#define _COMMANDER_FILE_INFO_

#include <cerrno>
#include <cstring>
#include <string>

#include <sys/stat.h>

#include "error_dialog.h"
#include "fileutils.h"

// Basic file information: whether it's a directory, executable, a symlink, file
// size, etc.
class FileInfo
{
    public:
    static FileInfo Get(const std::string &path);

    bool symlink() const { return symlink_; }

    // Symlinks are executable if they point to a valid target,
    // and the target is executable.
    bool executable() const
    {
        if (symlink_) return have_target_st_ && (target_st_.st_mode & S_IXUSR);
        return st_.st_mode & S_IXUSR;
    }

    // For symlinks, whether the *target* is a directory.
    bool directory() const
    {
        if (symlink_) return have_target_st_ && S_ISDIR(target_st_.st_mode);
        return S_ISDIR(st_.st_mode);
    }

    // For symlinks, the size is that of the target,
    std::size_t size() const
    {
        if (symlink_) return have_target_st_ ? target_st_.st_size : 0;
        return st_.st_size;
    }

    private:
    FileInfo() = default;

    bool symlink_ = false;
    bool have_target_st_ = false;

    struct stat st_;
    struct stat target_st_;
};

inline FileInfo FileInfo::Get(const std::string &path)
{
    FileInfo result;
    if (::lstat(path.c_str(), &result.st_) == -1)
    {
        ErrorDialog("lstat failed", std::strerror(errno));
        return result;
    }
    result.symlink_ = S_ISLNK(result.st_.st_mode);
    if (result.symlink_)
    {
        // Follow the link
        if (::stat(path.c_str(), &result.target_st_) == -1)
            result.have_target_st_ = false; // Points to non-existing file
        else
            result.have_target_st_ = true;
    }
    return result;
}

#endif // _COMMANDER_FILE_INFO_
