//MIT License

//Copyright (c) 2026 wzxTheSlimeball
#include "Canvas.hpp"
#include "Color.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <vector>
using std::vector;
Core::logger CanvasLogger;
Core::Canvas::Canvas(int width,int height)
    :width(width),height(height)
{
    stride=width*sizeof(Color);
    pixels.resize(width*height);
}
int Core::Canvas::getWidth()const{
    return width;
}
int Core::Canvas::getHeight()const{
    return height;
}
void Core::Canvas::clear(const Color& color){
    std::fill(pixels.begin(),pixels.end(),color);
}
void Core::Canvas::setPixel(int x,int y,const Color& color){
    if(x<0||x>=width||y<0||y>=height){
        CanvasLogger.traceLog(Core::logger::LOG_WARNING,"x or y out of bound.This operation will do nothing");
        return;
    }
    pixels[y*width+x]=color;
}
Core::Color Core::Canvas::getPixel(int x,int y)const{
    if(x<0||x>=width||y<0||y>=height){
        CanvasLogger.traceLog(Core::logger::LOG_WARNING,"x or y out of bound.This operation will give default Color");
        return Color();
    }
    return pixels[y*width+x];
}
Core::Canvas::~Canvas(){}
Core::Canvas::Canvas(HDC hdc,int width,int height){
    this->width=width;
    this->height=height;
    pixels.resize(width*height);
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            COLORREF color=GetPixel(hdc,x,y);
            int index=y*width+x;
            pixels[index]=Color::FromCOLORREF(color);
        }
    }
}