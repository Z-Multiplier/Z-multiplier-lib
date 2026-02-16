//MIT License

//Copyright (c) 2026 Z-Multiplier
#ifndef PAINTER_HPP
#define PAINTER_HPP

#define LOCATEMODE_CENTER 0x00
#define LOCATEMODE_LEFTUPCORNER 0x01
#define LOCATEMODE_RIGHTUPCORNER 0x02
#define LOCATEMODE_LEFTBOTTOMCORNER 0x03
#define LOCATEMODE_RIGHTBOTTOMCORNER 0x04

#include "Color.hpp"
#include "Canvas.hpp"
#include "Logger.hpp"
#include "Image.hpp"
#include "Font.hpp"

namespace Window{
    struct Handle;
    Handle *queryWindow(HWND);
    struct Buffer{
        HDC memHDC;
        HBITMAP hBmp;
        HBITMAP oldBmp;
        int width;
        int height;
        Buffer(){
            memHDC=NULL;
            hBmp=NULL;
            oldBmp=NULL;
            width=0;
            height=0;
        }
        ~Buffer(){
            if(memHDC){
                if(oldBmp){
                    SelectObject(memHDC, oldBmp);
                }
                if(hBmp){
                    DeleteObject(hBmp);
                }
                DeleteDC(memHDC);
            }
        }
    };
    struct Point{
        int x,y;
        Point()=default;
        Point(int x,int y):x(x),y(y){};
        Point(std::pair<int,int> pair):x(pair.first),y(pair.second){};
        Point(const Point& other)=default;
        bool operator==(Point& other);
    };
    struct Painter{
        private:
            HWND thisBindHWnd;
            HDC thisHDC;
            HDC windowHDC;
            PAINTSTRUCT ps;
            enum currentHDC{
                WINDOW,
                BUFFER
            }nowHDC;
            Handle *thisBindHandle;
            int radius;
        public:
            Painter()=delete;
            Painter(HWND hWnd,Handle *handle):thisBindHWnd(hWnd),thisHDC(NULL),windowHDC(NULL){
                thisBindHandle=handle;
                nowHDC=WINDOW;
                radius=0;
                this->windowHDC=BeginPaint(this->thisBindHWnd,&this->ps);
                this->thisHDC=this->windowHDC;
            };
            ~Painter(){
                if(this->windowHDC){
                    EndPaint(this->thisBindHWnd,&this->ps);
                    this->windowHDC=NULL;
                }
            }
            Painter(const Painter &)=delete;
            Painter &operator=(const Painter &)=delete;
            Painter(Painter &&)=default;
            Painter &operator=(Painter &&)=default;
            HWND getHWnd(){
                return this->thisBindHWnd;
            }
            bool alphaBlender(int x,int y,int width,int height,const Core::Color &color);
            bool drawBackground(const Core::Color &color);
            HDC getHDC(){
                return this->thisHDC;
            }
            void updateHDC();
            void present();
            void switchHDC();
            bool putPixel(int x,int y,const Core::Color& color);
            bool line(Point a,Point b,const Core::Color& color);
            bool slopeLine(Point a,Point b,const Core::Color& color);
            void setSize(int target);
            bool putUnitPixel(int x,int y,const Core::Color& color);
            bool floodFill(Point source,const Core::Color& color);
            bool hollowPolygon(const vector<Point>& points,const Core::Color& color);
            bool solidPolygon(const vector<Point>& points,const Core::Color& color);
            bool hollowCircle(const Point& origin,int radius,const Core::Color& color);
            bool solidCircle(const Point& origin,int radius,const Core::Color& color);
            bool putImage(char locateMode,const Point& locator,const Assets::Image& src,unsigned char alpha);
            bool putText(char locateMode,const Point& locator,const Assets::Font& font,const std::wstring& text,const Core::Color& color);
            bool bezierCurve(const vector<Point>& points,int accuracy,const Core::Color& color);
    };
}
#endif//PAINTER_HPP