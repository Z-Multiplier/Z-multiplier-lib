#include "Painter.hpp"
#include "Logger.hpp"
#include "Color.hpp"
#include "Utils.hpp"
#include <windows.h>
#include <stack>
#include <map>
#include <queue>

Core::logger PainterLogger;
bool Window::Painter::alphaBlender(int x,int y,int width,int height,const Core::Color &color){
    if(x<0||y<0||width<0||height<0){
        PainterLogger.traceLog(Core::logger::LOG_WARNING,"Invalid rectangle,This will do literally nothing");
        return true;
    }
    if(color.a==0)return true;
    if(color.a==255){
        HBRUSH brush=CreateSolidBrush(color.toCOLORREF());
        if(!brush){
            PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create brush");
            return false;
        }
        RECT rect={x,y,x+width,y+height};
        FillRect(this->thisHDC,&rect,brush);
        DeleteObject(brush);
        return true;
    }
    HDC memHDC=CreateCompatibleDC(this->thisHDC);
    if(!memHDC){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create memory DC");
        return false;
    }
    HBITMAP hBmp=CreateCompatibleBitmap(this->thisHDC,width,height);
    if(!hBmp){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create bitmap");
        DeleteDC(memHDC);
        return false;
    }
    HBITMAP hOldBmp=(HBITMAP)SelectObject(memHDC,hBmp);
    HBRUSH hBrush=CreateSolidBrush(color.toCOLORREF());
    if(!hBrush){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create brush");
        DeleteDC(memHDC);
        DeleteObject(hBmp);
        return false;
    }
    RECT rect={0,0,width,height};
    FillRect(memHDC,&rect,hBrush);
    DeleteObject(hBrush);
    BLENDFUNCTION bf={AC_SRC_OVER,0,color.a,0};
    bool result=AlphaBlend(this->thisHDC,x,y,width,height,memHDC,0,0,width,height,bf);
    SelectObject(memHDC,hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(memHDC);
    return (result!=FALSE);
}
bool Window::Painter::drawBackground(const Core::Color &color){
    RECT rect;
    GetClientRect(this->thisBindHWnd,&rect);
    return this->alphaBlender(0,0,rect.right,rect.bottom,color);
}
void Window::Painter::updateHDC()
{
    switch(this->nowHDC){
        case WINDOW:{
            EndPaint(this->thisBindHWnd,&this->ps);
            this->thisHDC=BeginPaint(this->thisBindHWnd,&this->ps);
            break;
        }
        case BUFFER:{
            this->thisHDC=this->thisBindHandle->getBuffer().memHDC;
            break;
        }
    }
}
void Window::Painter::present(){
    this->thisBindHandle->update();
}
void Window::Painter::switchHDC(){
    switch(this->nowHDC){
        case WINDOW:{
            this->nowHDC=BUFFER;
            break;
        }
        case BUFFER:{
            this->nowHDC=WINDOW;
            break;
        }
    }
    this->updateHDC();
}
bool Window::Point::operator==(Point& other){
    return this->x==other.x&&this->y==other.y;
}
bool Window::Painter::putUnitPixel(int x,int y,const Core::Color& color){
    return this->alphaBlender(x,y,1,1,color);
}
bool Window::Painter::putPixel(int x,int y,const Core::Color& color){
    return this->alphaBlender(x-this->radius,y-this->radius,1+this->radius,1+this->radius,color);
}
bool Window::Painter::line(Point a,Point b,const Core::Color& color){
    if(a==b){
        PainterLogger.traceLog(Core::logger::LOG_WARNING,"The points are the same,skip this");
        return false;
    }
    int deltaX=abs(a.x-b.x);
    int deltaY=abs(a.y-b.y);
    int sX=(a.x<b.x)?1:-1;
    int sY=(a.y<b.y)?1:-1;
    int fault=((deltaX>deltaY)?deltaX:-deltaY)/2;
    int nowX=a.x;
    int nowY=a.y;
    while(true){
        if(!putPixel(nowX,nowY,color)){
            return false;
        }
        if(nowX==b.x&&nowY==b.y){
            break;
        }
        int doubleFault=fault*2;
        if(doubleFault>-deltaY){
            fault-=deltaY;
            nowX+=sX;
        }
        if(doubleFault<deltaX){
            fault+=deltaX;
            nowY+=sY;
        }
    }
    return true;
}
void Window::Painter::setSize(int target){
    this->radius=target;
}
bool Window::Painter::floodFill(Window::Point source,const Core::Color& color){
    int width=this->thisBindHandle->getRect().right;
    int height=this->thisBindHandle->getRect().bottom;
    HDC tempHDC=this->thisBindHandle->getBuffer().memHDC;
    if(width<=0||height<=0){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Invalid canvas size");
        return false;
    }
    if(source.x<0||source.x>=width||
        source.y<0||source.y>=height){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Source point out of bounds");
        return false;
    }
    HDC srcHDC=tempHDC?tempHDC:this->thisHDC;
    HDC memDC=CreateCompatibleDC(srcHDC);
    if(!memDC){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create memory DC for pixels");
        return false;
    }
    HBITMAP hBmp = CreateCompatibleBitmap(tempHDC,width,height);
    if(!hBmp){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create compatible bitmap");
            DeleteDC(memDC);
        return false;
    }
    HBITMAP hOldBmp=(HBITMAP)SelectObject(memDC,hBmp);
    BitBlt(memDC,0,0,width,height,srcHDC,0,0,SRCCOPY);
    BITMAPINFO bmi;
    ZeroMemory(&bmi,sizeof(bmi));
    bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth=width;
    bmi.bmiHeader.biHeight=-height;
    bmi.bmiHeader.biPlanes=1;
    bmi.bmiHeader.biBitCount=32;
    bmi.bmiHeader.biCompression=BI_RGB;
    std::vector<DWORD> pixels(static_cast<size_t>(width)*static_cast<size_t>(height));
    if(!GetDIBits(srcHDC,hBmp,0,height,pixels.data(),&bmi,DIB_RGB_COLORS)){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"GetDIBits failed");
        SelectObject(memDC,hOldBmp);
        DeleteObject(hBmp);
        DeleteDC(memDC);
        return false;
    }
    auto getColorFromBuffer=[&pixels,width](int x,int y)->Core::Color{
        DWORD px=pixels[static_cast<size_t>(y)*static_cast<size_t>(width)+static_cast<size_t>(x)];
        unsigned char b=px&0xFF;
        unsigned char g=(px>>8)&0xFF;
        unsigned char r=(px>>16)&0xFF;
        COLORREF cref=RGB(r,g,b);
        return Core::Color::FromCOLORREF(cref);
    };
    Core::Color srcColor=getColorFromBuffer(source.x,source.y);
    if(srcColor==color){
        PainterLogger.traceLog(Core::logger::LOG_INFO,"Source already has target color");
        SelectObject(memDC,hOldBmp);
        DeleteObject(hBmp);
        DeleteDC(memDC);
        return true;
    }
    auto getIndex=[width](int x,int y)->size_t{
        return static_cast<size_t>(y)*static_cast<size_t>(width)+static_cast<size_t>(x);
    };
    std::vector<bool> visited(static_cast<size_t>(width)*static_cast<size_t>(height),false);
    std::vector<std::pair<int,int>> stack;
    stack.push_back({source.x,source.y});
    visited[getIndex(source.x,source.y)]=true;
    while(!stack.empty()){
        auto p=stack.back(); stack.pop_back();
        int x=p.first;
        int y=p.second;
        int left=x;
        while(left>=0){
            Core::Color c=getColorFromBuffer(left,y);
            if(!(c==srcColor)) break;
            left--;
        }
        left++;
        int right=x;
        while(right<width){
            Core::Color c=getColorFromBuffer(right,y);
            if(!(c==srcColor)) break;
            right++;
        }
        for(int xi=left;xi<right;++xi){
            if(!putUnitPixel(xi,y,color)){
                PainterLogger.traceLog(Core::logger::LOG_ERROR,"Failed to put pixel at ("+
                                      std::to_string(xi)+","+std::to_string(y)+")");
                SelectObject(memDC,hOldBmp);
                DeleteObject(hBmp);
                DeleteDC(memDC);
                return false;
            }
            visited[getIndex(xi,y)]=true;
        }
        for(int dir=-1;dir<=1;dir+=2){
            int ny=y+dir;
            if(ny<0||ny>=height) continue;
            int xi=left;
            while(xi<right){
                bool found=false;
                while(xi<right){
                    Core::Color c=getColorFromBuffer(xi,ny);
                    if((c==srcColor) && !visited[getIndex(xi,ny)]){ found=true; break; }
                    xi++;
                }
                if(!found) break;
                int runStart=xi;
                while(xi<right){
                    Core::Color c=getColorFromBuffer(xi,ny);
                    if(!(c==srcColor)||visited[getIndex(xi,ny)]) break;
                    visited[getIndex(xi,ny)]=true;
                    xi++;
                }
                stack.push_back({runStart,ny});
            }
        }
    }
    SelectObject(memDC,hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(memDC);
    return true;
}