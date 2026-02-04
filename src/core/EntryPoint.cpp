//MIT License

//Copyright (c) 2026 wzxTheSlimeball
#include "EntryPoint.hpp"
#include <windows.h>
HINSTANCE mainHInstance;

extern int main();

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR lpCmdLine,int nCmdShow)
{
    mainHInstance=hInstance;
    return main();
}