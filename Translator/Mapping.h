#ifndef MAPPING_H_INCLUDED
#define MAPPING_H_INCLUDED

#include <cassert>

#if defined(__WIN32)

#include "windows.h"

namespace course {

enum class ECMapMode
{
    MAP_READONLY_FILE, MAP_WRITEONLY_FILE, MAP_READWRITE_FILE
};

class CMapping
{
private:
    static DWORD get_granularity_()
    {
        SYSTEM_INFO sys_info;
        memset(&sys_info, 0x00, sizeof(sys_info));

        GetSystemInfo(&sys_info);

        return sys_info.dwAllocationGranularity;
    }

private:
    static DWORD granularity;

public:
    CMapping             (const CMapping&) = delete;
    CMapping& operator = (const CMapping&) = delete;

    CMapping(ECMapMode map_mode_set, const char* file_path, DWORD file_length_set = 0):
        map_mode_   (map_mode_set),
        map_handle_ (INVALID_HANDLE_VALUE),
        file_handle_(INVALID_HANDLE_VALUE),
        file_length_(file_length_set)
    {
        switch (map_mode_)
        {
            case ECMapMode::MAP_READONLY_FILE:
                file_handle_ = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

                break;

            case ECMapMode::MAP_WRITEONLY_FILE:
            case ECMapMode::MAP_READWRITE_FILE:
                file_handle_ = CreateFile(file_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

                break;

            default: break;//TODO
        }

        assert(file_handle_ != INVALID_HANDLE_VALUE);

        if (!file_length_) file_length_ = GetFileSize(file_handle_, &file_length_);

        char mapping_name_str[16] = "";
        snprintf(mapping_name_str, 16, "%p", this);

        switch (map_mode_)
        {
            case ECMapMode::MAP_READONLY_FILE:
                map_handle_ = CreateFileMapping(file_handle_, NULL, PAGE_READONLY, 0, file_length_, mapping_name_str);

                break;

            case ECMapMode::MAP_WRITEONLY_FILE:
            case ECMapMode::MAP_READWRITE_FILE:
                map_handle_ = CreateFileMapping(file_handle_, NULL, PAGE_READWRITE, 0, file_length_, mapping_name_str);

                break;

            default: assert(0);
        }

        assert(map_handle_ != INVALID_HANDLE_VALUE);
    }

    ~CMapping()
    {
        if (map_handle_)
        {
            CloseHandle(map_handle_);
            map_handle_ = NULL;
        }

        if (file_handle_)
        {
            CloseHandle(file_handle_);
            file_handle_ = INVALID_HANDLE_VALUE;
        }
    }

    ECMapMode get_map_mode  () const { return map_mode_; }
    HANDLE    get_map_handle() const { return map_handle_; }

    DWORD get_file_length() const { return file_length_; }

private:
    ECMapMode map_mode_;
    HANDLE    map_handle_;

    HANDLE file_handle_;
    DWORD  file_length_;
};

DWORD CMapping::granularity = CMapping::get_granularity_();

}//namespace course

#else

#include <errno.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../Logger/Logger.h"

namespace course {

enum class ECMapMode
{
    MAP_READONLY_FILE, MAP_WRITEONLY_FILE, MAP_READWRITE_FILE
};

class CMapping
{
private:
    static const size_t granularity = PAGE_SIZE;

public:
    CMapping             (const CMapping&) = delete;
    CMapping& operator = (const CMapping&) = delete;

    CMapping(ECMapMode map_mode_set, const char* file_path, size_t file_length_set = 0):
            map_mode_   (map_mode_set),
            file_handle_(-1),
            file_length_(file_length_set)
    {
        switch (map_mode_)
        {
            case ECMapMode::MAP_READONLY_FILE:
            {
                file_handle_ = open(file_path, O_RDONLY);

                if (!file_length_)
                {
                    struct stat file_stat = {};
                    fstat(file_handle_, &file_stat);

                    file_length_ = file_stat.st_size;
                }
            }
            break;

            case ECMapMode::MAP_WRITEONLY_FILE:
            {
                file_handle_ = open(file_path, O_RDWR);

                if (file_length_)
                    ftruncate(file_handle_, file_length_);
            }
            break;

            case ECMapMode::MAP_READWRITE_FILE:
            {
                file_handle_ = open(file_path, O_RDWR);

                if (file_length_)
                    ftruncate(file_handle_, file_length_);
            }
            break;

            default: break;//TODO
        }
    }

    ~CMapping()
    {
        if (file_handle_ != -1)
        {
            close(file_handle_);
            file_handle_ = -1;
        }
    }

    ECMapMode get_map_mode()    const { return map_mode_; }
    size_t    get_file_length() const { return file_length_; }

    int get_file_handle() const { return file_handle_; }

private:
    ECMapMode map_mode_;

    int    file_handle_;
    size_t file_length_;
};

}//namespace course

#endif //defined(__WIN32)

#endif //MAPPING_H_INCLUDED
