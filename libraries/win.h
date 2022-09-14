#ifdef _WIN32
    #define TokenType WinTokenType
    #include <winbase.h>
#endif
#undef TokenType
#undef FAILED