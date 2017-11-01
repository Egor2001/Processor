#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <memory>
#include <cstdio>
#include <limits>
#include <ctype.h>
#include <cmath>
#include <typeinfo>
#include <cstring>

#define CRS_POISON_INT(var) \
    (var = std::numeric_limits<std::remove_reference<decltype(var)>::type>::max())

#define CRS_POISON_FLOAT(var) \
    (var = std::numeric_limits<std::remove_reference<decltype(var)>::type>::quiet_NaN())

#define CRS_IS_POISON_INT(var) \
    (var == std::numeric_limits<std::remove_reference<decltype(var)>::type>::max())

#define CRS_IS_POISON_FLOAT(var) (std::isnan(var))

#define CRS_STRINGIZE_HELPER(name) #name
#define CRS_STRINGIZE(name) CRS_STRINGIZE_HELPER(name)

#ifdef CRS_LOGSTAMP
    #define CRS_STATIC_MSG(message_literal) \
        CLogger::instance()->print_str("[FILE: " __FILE__ ", LINE:" CRS_STRINGIZE(__LINE__) "] " \
                                       "[" message_literal "] \n")

    #define CRS_STATIC_LOG(format_literal, ...) \
        CLogger::instance()->print_str("[FILE: " __FILE__ ", LINE:" CRS_STRINGIZE(__LINE__) "] " \
                                       "[" format_literal "] \n", __VA_ARGS__)

#else
    #define CRS_STATIC_MSG(message_literal) \
        CLogger::instance()->print_str("[" message_literal "] \n")

    #define CRS_STATIC_LOG(format_literal, ...) \
        CLogger::instance()->print_str("[" format_literal "] \n", __VA_ARGS__)

#endif //CRS_LOGSTAMP

#define CRS_STATIC_DUMP(format_literal, ...) \
    CLogger::instance()->print_str(format_literal " \n", __VA_ARGS__)


#define CRS_CONSTRUCT_CHECK() \
{ \
    CRS_STATIC_LOG("CONSTRUCTING object: [%s]", typeid(*this).name()); \
    \
    if (!this->ok()) \
        this->dump(); \
}


#define CRS_DESTRUCT_CHECK() \
{ \
    if (!this->ok()) \
        this->dump(); \
    \
    CRS_STATIC_LOG("DESTRUCTING object: [%s]", typeid(*this).name()); \
}

#define CRS_BEG_CHECK() \
{ \
    CRS_STATIC_LOG("BEGIN functon check: %s", __func__); \
    \
    if (!this->ok()) \
        this->dump(); \
}

#define CRS_END_CHECK() \
{ \
    if (!this->ok()) \
        this->dump(); \
    \
    CRS_STATIC_LOG("END functon check: %s", __func__); \
}

namespace course_stack {

template<char first_byte, char... rest_bytes>
size_t crs_hash_helper()
{
    return (crs_hash_helper<rest_bytes...>()) ^
           (first_byte << (sizeof...(rest_bytes))%(sizeof(size_t)/sizeof(char))*sizeof(char));
}

template<char first_byte>
size_t crs_hash_helper()
{
    return first_byte;
}

template<char... bytes>
size_t operator "" _crs_hash()
{
    return crs_hash_helper<bytes...>();
}

class CLogger
{
public:
    enum class ELogMode
    {
        LOG_NONE = 0,
        LOG_INFO,
        LOG_DEBUG
    };

private:
    static const size_t MAX_ERROR_STR = 128 + FILENAME_MAX;

    static std::shared_ptr<CLogger> instance_;

public:
    static CLogger* create(const char* log_file_name, ELogMode log_mode_set = ELogMode::LOG_INFO)
    {
        instance_ = std::make_shared<CLogger>(log_file_name, log_mode_set);

        return instance_.get();
    }

    static CLogger* instance()
    {
        if (!instance_)
            throw std::runtime_error("CLogger instance is null pointer");

        return instance_.get();
    }

    static void destroy()
    {
        instance_.reset();
    }

public:
    explicit CLogger(const char* log_file_name, ELogMode log_mode_set = ELogMode::LOG_INFO):
        tabs_num_(0), log_mode_(log_mode_set), log_file_(fopen(log_file_name, "w"))
    {
        if (!log_file_name)
            throw std::invalid_argument("[CLogger constructor]: log file name is null pointer");

        if (!log_file_)
        {
            char error_str[MAX_ERROR_STR] = "[CLogger constructor]: unable to open file: ";
            throw std::runtime_error(strncat(error_str, log_file_name, FILENAME_MAX));
        }
    }

    CLogger             (const CLogger&) = delete;
    CLogger& operator = (const CLogger&) = delete;

    ~CLogger()
    {
        if (log_file_)
            fclose(log_file_);

        log_file_ = nullptr;

        CRS_POISON_INT(tabs_num_);
        log_mode_ = ELogMode::LOG_NONE;
    }

    template<typename... Types>
    void print_str(const char* format_str, Types&&... args) const
    {
        if (!format_str)
            throw std::invalid_argument("[print_str]: format_str is null pointer");

        if (!log_file_)
            throw std::runtime_error("[print_str]: log_file_ is null pointer");

        fprintf(log_file_, format_str, std::forward<Types>(args)...);
    }

    template<typename... Types>
    void log_msg(const char* format_str, Types&&... args) const
    {
        if (!log_file_)
            throw std::runtime_error("[log_msg]: log_file_ is null pointer");

        fprintf(log_file_, "\n" "[LOG] ");
        print_str(format_str, std::forward<Types>(args)...);
    }

    void reset_log_file(const char* new_file_name)
    {
        if (!new_file_name)
            throw std::invalid_argument("[reset_log_file]: new file name is null pointer");

        if (log_file_)
            fclose(log_file_);

        log_file_ = fopen(new_file_name, "w");

        if (!log_file_)
        {
            char error_str[MAX_ERROR_STR] = "[reset_log_file]: unable to open file: ";
            throw std::runtime_error(strncat(error_str, new_file_name, FILENAME_MAX));
        }
    }

    size_t   get_tabs_num() const { return tabs_num_; }
    ELogMode get_log_mode() const { return log_mode_; }

    const char* get_log_mode_string() const
    {
        switch (log_mode_)//TODO: macros for enum declaration and ToString function are better
        {
        case ELogMode::LOG_DEBUG: return "LOG_DEBUG";
        case ELogMode::LOG_INFO:  return "LOG_INFO";
        case ELogMode::LOG_NONE:  return "LOG_NONE";
        default:                  return "UNKNOWN LOG TYPE";
        }

        return "UNKNOWN LOG TYPE";
    }

public:
    bool ok() const
    {
        return (this &&
                !(CRS_IS_POISON_INT(tabs_num_)) &&
                (log_mode_ != ELogMode::LOG_NONE) && log_file_);
    }

    void dump() const
    {
        print_str("CLogger[%s, this : %p] \n"
                  "{ \n"
                  "    tabs_num_ : {%d} \n"
                  "    log_mode_ : {%s} \n"
                  "    log_file_ : {%p} \n"
                  "} \n",
                  (ok() ? "OK" : "ERROR"), this,
                  tabs_num_, get_log_mode_string(), log_file_);
    }

private:
    size_t   tabs_num_;
    ELogMode log_mode_;
    FILE*    log_file_;
};

std::shared_ptr<CLogger> CLogger::instance_ = std::make_shared<CLogger>("Stack/Logs/log.txt", CLogger::ELogMode::LOG_DEBUG);

}

#endif // LOGGER_H_INCLUDED

