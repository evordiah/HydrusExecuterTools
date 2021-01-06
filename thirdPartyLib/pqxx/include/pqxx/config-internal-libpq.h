/* Automatically generated from config.h: internal/libpq config. */
#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
#define PQXX_HAVE_STRNLEN
/* Define if compiler provides strnlen_s */
#define PQXX_HAVE_STRNLEN_S
/* Define if thread_local is fully supported. */
#define PQXX_HAVE_THREAD_LOCAL
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
#define PQXX_HAVE_PQENCRYPTPASSWORDCONN 1
#else
#   error unknown os
#endif
