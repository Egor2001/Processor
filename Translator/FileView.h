#ifndef FILE_VIEW_H_INCLUDED
#define FILE_VIEW_H_INCLUDED


#if defined(__WIN32)

#include "Mapping.h"

namespace course {

class CFileView
{
public:
    explicit CFileView(std::shared_ptr<CMapping> mapping_class_set):
        mapping_class_ (mapping_class_set),
        file_view_size_(0),
        file_view_str_ (nullptr)
    {
        DWORD access = -1;

        switch (mapping_class_->get_map_mode())
        {
            case ECMapMode::MAP_READONLY_FILE:  access = FILE_MAP_READ;       break;
            case ECMapMode::MAP_WRITEONLY_FILE: access = FILE_MAP_WRITE;      break;
            case ECMapMode::MAP_READWRITE_FILE: access = FILE_MAP_ALL_ACCESS; break;

            default: break;//TODO
        }

        file_view_size_ = mapping_class_->get_file_length();
        file_view_str_  = static_cast<char*>(MapViewOfFile(mapping_class_->get_map_handle(),
                                                           access, 0, 0, file_view_size_));
    }

    explicit CFileView(ECMapMode map_mode, const char* file_path, DWORD file_length = 0):
        CFileView(std::make_shared<CMapping>(map_mode, file_path, file_length))
    {}

    CFileView             (const CFileView&) = delete;
    CFileView& operator = (const CFileView&) = delete;

    ~CFileView()
    {
        if (file_view_str_)
            UnmapViewOfFile(file_view_str_);

        file_view_str_  = nullptr;
        file_view_size_ = 0;
    }

    std::shared_ptr<CMapping> get_mapping_class () const { return mapping_class_; }

    DWORD       get_file_view_size() const { return file_view_size_; }
    char*       get_file_view_str ()       { return file_view_str_; }
    const char* get_file_view_str () const { return file_view_str_; }

private:
    std::shared_ptr<CMapping> mapping_class_;
    DWORD                     file_view_size_;
    char*                     file_view_str_;
};

}//namespace course

#else

#include "Mapping.h"
#include <sys/mman.h>

namespace course {

class CFileView
{
public:
    explicit CFileView(std::shared_ptr<CMapping> mapping_class_set):
            mapping_class_ (mapping_class_set),
            file_view_size_(0),
            file_view_str_ (nullptr)
    {
        int access = 0;

        switch (mapping_class_->get_map_mode())
        {
            case ECMapMode::MAP_READONLY_FILE:  access = PROT_READ;              break;
            case ECMapMode::MAP_WRITEONLY_FILE: access = PROT_WRITE;             break;
            case ECMapMode::MAP_READWRITE_FILE: access = PROT_READ | PROT_WRITE; break;

            default: break;//TODO
        }

        file_view_size_ = mapping_class_->get_file_length();
        file_view_str_  = static_cast<char*>(mmap(nullptr, file_view_size_, access,
                                             MAP_SHARED, mapping_class_->get_file_handle(), 0));

        assert(file_view_str_ != MAP_FAILED);
        assert(file_view_str_);
    }

    explicit CFileView(ECMapMode map_mode, const char* file_path, size_t file_length = 0):
            CFileView(std::make_shared<CMapping>(map_mode, file_path, file_length))
    {}

    CFileView             (const CFileView&) = delete;
    CFileView& operator = (const CFileView&) = delete;

    ~CFileView()
    {
        if (file_view_str_)
            munmap(file_view_str_, file_view_size_);

        file_view_str_  = nullptr;
        file_view_size_ = 0;
    }

    std::shared_ptr<CMapping> get_mapping_class () const { return mapping_class_; }

    size_t      get_file_view_size() const { return file_view_size_; }
    char*       get_file_view_str ()       { return file_view_str_; }
    const char* get_file_view_str () const { return file_view_str_; }

private:
    std::shared_ptr<CMapping> mapping_class_;
    size_t                    file_view_size_;
    char*                     file_view_str_;
};

}//namespace course

#endif //defined(__WIN32)

#endif // FILE_VIEW_H_INCLUDED

