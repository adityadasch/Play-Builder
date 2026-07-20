// Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lib/cJSON.h"

#ifdef DEBUG
#include <stdarg.h>
#endif

// Code key words
#ifdef __STDC_VERSION__
    #if __STDC_VERSION__ < 202311L
        #define nullptr ((void *)0)
        #define true 1
        #define false 0
        typedef bool int
    #endif
#endif

// Magic Number(s)
#define BUFFERMAGICNUMBER 2500

// OS
#if defined(_WIN32) || defined(_WIN64)
    #define FILEDRUNCMD "start cmd.exe /k \"%s&&(call %s%s&echo(&&echo Press any key to exit...&&pause >nul&&exit)"

    #define EXEEXT ".exe"
    
#elif defined(__linux__)
    #define FILEDRUNCMD "bash -c '%s && \"./%s%s\"; echo; read -n 1 -s -r -p \"Press any key to continue...\"; echo'"

    #define EXEEXT ".out"
#endif