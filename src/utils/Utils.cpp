//MIT License

//Copyright (c) 2026 Z-Multiplier

#include "Utils.hpp"
#include <windows.h>

namespace Utils{
    BOOL classIsRegistered(const wchar_t *CLASSNAME){
        WNDCLASSEX wc={};
        return GetClassInfoEx(NULL,CLASSNAME,&wc);
    }
    int Random::range(int min,int max){
        std::uniform_int_distribution<int> r(min,max);
        return r(gen);
    }
    float Random::real(int min,int max){
        std::uniform_real_distribution<float> r(min,max);
        return r(gen);
    }
}
