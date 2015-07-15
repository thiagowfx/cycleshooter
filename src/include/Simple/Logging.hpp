#ifndef _LOGGING_HPP_
#define _LOGGING_HPP_

#include <cstdio>

namespace Cycleshooter {

/**
  * Log debug messages to stdout.
  */

#ifdef LOGGING_ENABLED

#define LOG(...)\
    do {\
        printf("INFO: ");\
        printf(__VA_ARGS__);\
        printf("\n");\
    } while(false)

#else

#define LOG(...)

#endif

#define LOG_WARN(...)\
    do{\
        printf("WARNING: ");\
        printf(__VA_ARGS__);\
        printf("\n");\
    } while(false);

#define LOG_FATAL(...)\
    do{\
        printf("ERROR: ");\
        printf(__VA_ARGS__);\
        printf("\n");\
    } while(false);

}

#endif