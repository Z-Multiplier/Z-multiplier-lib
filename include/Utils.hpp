//MIT License

//Copyright (c) 2026 Z-Multiplier

#ifndef UTILS_HPP
#define UTILS_HPP
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include "Window.hpp"
#include <windows.h>
#include <string>
#include <random>

namespace Utils{
    BOOL classIsRegistered(wchar_t const*CLASSNAME);
    class Random{
        std::random_device dev;
        std::mt19937 gen;
        public:
            Random():gen(dev()){}
            int range(int min,int max);
            float real(int min,int max);
    };
}

#endif