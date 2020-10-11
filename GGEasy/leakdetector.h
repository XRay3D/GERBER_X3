#pragma once

#ifdef _DEBUG

    #define _CRTDBG_MAP_ALLOC

//    #include <cstdlib>

    #include <crtdbg.h>

    #define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
    // Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
    // allocations to be of _CLIENT_BLOCK type
#else
    #define new new
#endif
