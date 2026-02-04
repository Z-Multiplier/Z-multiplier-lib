//MIT License

//Copyright (c) 2026 wzxTheSlimeball

#include "Utils.hpp"
#include <windows.h>

BOOL classIsRegistered(const wchar_t *CLASSNAME){
    WNDCLASSEX wc={};
    return GetClassInfoEx(NULL,CLASSNAME,&wc);//艹
}