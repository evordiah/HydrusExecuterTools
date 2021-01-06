/* Automatically generated from config.h: public/compiler config. */
#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__)
/* Define if compiler provides strnlen */
#define PQXX_HAVE_STRNLEN
/* Define if compiler provides strnlen_s */
#define PQXX_HAVE_STRNLEN_S
/* Define if thread_local is fully supported. */
#define PQXX_HAVE_THREAD_LOCAL
#elif defined(__linux__) || defined(linux) || defined(__linux) || defined(__LINUX__) || defined(LINUX) || \
    defined(_LINUX)
#define PQXX_HAVE_GCC_PURE 1
#define PQXX_HAVE_STRNLEN 1
#define PQXX_HAVE_VARIANT 1
#else
#   error unknown os
#endif

