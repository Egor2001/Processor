#ifndef MACRO_H_INCLUDED
#define MACRO_H_INCLUDED

#define CRS_STRINGIZE_HELPER(name) #name
#define CRS_STRINGIZE(name) CRS_STRINGIZE_HELPER(name)

#define CRS_POISON_INT(var) \
    (var = std::numeric_limits<std::remove_reference<decltype(var)>::type>::max())

#define CRS_POISON_FLOAT(var) \
    (var = std::numeric_limits<std::remove_reference<decltype(var)>::type>::quiet_NaN())

#define CRS_IS_POISON_INT(var) \
    (var == std::numeric_limits<std::remove_reference<decltype(var)>::type>::max())

#define CRS_IS_POISON_FLOAT(var) (std::isnan(var))

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

#define CRS_PROCESS_ERROR(format_str, ...) \
{ \
    char error_str[CCourseException::MAX_MSG_LEN] = ""; \
    snprintf(error_str, CCourseException::MAX_MSG_LEN, \
             format_str, __VA_ARGS__); \
    \
    CRS_STATIC_LOG("%s", error_str); \
    \
    throw CCourseException(error_str); \
}

#endif // MACRO_H_INCLUDED

