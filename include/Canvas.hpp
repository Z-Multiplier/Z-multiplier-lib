//MIT License

//Copyright (c) 2026 wzxTheSlimeball
#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <vector>
#include "Color.hpp"
#include "Logger.hpp"
using std::vector;
namespace Core{
    struct Canvas{
        private:
            int width;
            int height;
            int stride;
            vector<Color> pixels;
        public:
            Canvas()=default;
            Canvas(int width,int height);
            int getWidth()const;
            int getHeight()const;
            void clear(const Color& color);
            void setPixel(int x,int y,const Color& color);
            Color getPixel(int x,int y)const;
            ~Canvas();
            Canvas(HDC hdc,int width,int height);
    };
}
#endif // CANVAS_HPP