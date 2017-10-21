#pragma once

#if !defined(WIN32) && !defined(_WIN32)
# define MYSQL_HELPER_API
#else
#if defined(_STATIC_LIB)
# define MYSQL_HELPER_API
#else
# if defined(MYSQLHELPER_EXPORTS)
#  define MYSQL_HELPER_API __declspec(dllexport)
# else
#  define MYSQL_HELPER_API __declspec(dllimport)
# endif
#endif
#endif
