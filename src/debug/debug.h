#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_MODE
    #define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // do nothing
#endif

#endif 
