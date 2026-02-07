#ifndef PAINTER_HPP
#define PAINTER_HPP

#include "Color.hpp"
#include "Canvas.hpp"
#include "Logger.hpp"

namespace Window{
    struct Handle;
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
        Point(const Point& other)=default;
        bool operator==(Point& other);
    };
    struct Painter{
        private:
            HWND thisBindHWnd;
            HDC thisHDC;
            PAINTSTRUCT ps;
            enum currentHDC{
                WINDOW,
                BUFFER
            }nowHDC;
            Handle *thisBindHandle;
            int radius;
        public:
            Painter()=delete;
            Painter(HWND hWnd,Handle *handle):thisBindHWnd(hWnd),thisHDC(BeginPaint(hWnd,&ps)){
                thisBindHandle=handle;
                nowHDC=WINDOW;
            };
            ~Painter(){
                EndPaint(thisBindHWnd,&ps);
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
            bool line(Point a,Point b,const Core::Color& color);//bresenham line
            void setSize(int target);
            bool putUnitPixel(int x,int y,const Core::Color& color);
            bool floodFill(Point source,const Core::Color& color);
    };
}
#endif//PAINTER_HPP