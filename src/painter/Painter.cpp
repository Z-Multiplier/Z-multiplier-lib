//MIT License

//Copyright (c) 2026 Z-Multiplier
#include "Painter.hpp"
#include "Logger.hpp"
#include "Color.hpp"
#include "Utils.hpp"
#include "Image.hpp"
#include <windows.h>
#include <stack>
#include <map>
#include <queue>
#include <algorithm>
#include <climits>
#include <cmath>

Core::logger PainterLogger;
bool Window::Painter::alphaBlender(int x,int y,int width,int height,const Core::Color &color){
    if(x<0||y<0||width<0||height<0){
        PainterLogger.formatLog(Core::logger::LOG_WARNING,"Invalid rectangle,This will do literally nothing:x=%d,y=%d,w=%d,h=%d",x,y,width,height);
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
    if(this->nowHDC==WINDOW){
        this->thisHDC=this->windowHDC;
    }
    else{
        this->thisHDC=this->thisBindHandle->getBuffer().memHDC;
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
    int x1=a.x,x2=b.x,y1=a.y,y2=b.y;
    if(x1==x2){
        int yStart=std::min(y1,y2);
        int yEnd=std::max(y1,y2);
        for(int y=yStart;y<=yEnd;++y) {
            if(!putPixel(x1,y,color)) return false;
        }
        return true;
    }
    if(y1==y2){
        int xStart=std::min(x1,x2);
        int xEnd=std::max(x1,x2);
        for(int x=xStart;x<=xEnd;++x){
            if(!putPixel(x,y1,color)) return false;
        }
        return true;
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
bool Window::Painter::hollowPolygon(const vector<Point>& points,const Core::Color& color){
    for(unsigned int i=0;i<points.size();i++){
        bool result=line(points[i],points[(i+1)%points.size()],color);
        if(!result){
            return false;
        }
    }
    return true;
}
bool Window::Painter::slopeLine(Point a,Point b,const Core::Color& color){
    int dx=b.x-a.x;
    int dy=b.y-a.y;
    int steps=abs(dx)>abs(dy)?abs(dx):abs(dy);
    if(steps==0){
        return putPixel(a.x,a.y,color);
    }
    float xInc=dx/(float)steps;
    float yInc=dy/(float)steps;
    float x=(float)a.x;
    float y=(float)a.y;
    for(int i=0;i<=steps;i++){
        int xi=(int)(x+(x>=0.0f?0.5f:-0.5f));
        int yi=(int)(y+(y>=0.0f?0.5f:-0.5f));
        if(!putPixel(xi,yi,color)){
            return false;
        }
        x+=xInc;
        y+=yInc;
    }
    return true;
}
bool Window::Painter::solidPolygon(const vector<Point>& points,const Core::Color& color){
    if(points.size()<3) return false;
    struct Edge{int minY;int maxY;float curX;float invSlope;};
    int globalMinY=INT_MAX;
    int globalMaxY=INT_MIN;
    for(const Point& pt:points){
        globalMaxY=std::max(globalMaxY,pt.y);
        globalMinY=std::min(globalMinY,pt.y);
    }
    if(globalMinY>globalMaxY) return false;
    vector<vector<Edge>> ET(static_cast<size_t>(globalMaxY-globalMinY+1));
    for(size_t i=0;i<points.size();++i){
        size_t nxt=(i+1)%points.size();
        Point a=points[i];
        Point b=points[nxt];
        if(a.y==b.y){
            alphaBlender(std::min(a.x,b.x),a.y,abs(a.x-b.x),1,color);
            continue;
        }
        int edgeMinY=std::min(a.y,b.y);
        int edgeMaxY=std::max(a.y,b.y);
        float curX=(a.y<b.y)?(float)a.x:(float)b.x;
        float invSlope=(b.x-a.x)/static_cast<float>(b.y-a.y);
        Edge e{edgeMinY,edgeMaxY,curX,invSlope};
        ET[edgeMinY-globalMinY].push_back(e);
    }
    vector<Edge> AET;
    for(int y=globalMinY;y<globalMaxY;++y){
        auto& bucket=ET[y-globalMinY];
        for(const Edge& e:bucket) AET.push_back(e);
        AET.erase(std::remove_if(AET.begin(),AET.end(),[y](const Edge& e){return e.maxY<=y;}),AET.end());
        if(AET.empty()) continue;
        std::sort(AET.begin(),AET.end(),[](const Edge& a,const Edge& b){return a.curX<b.curX;});
        if(AET.size()%2==1){
            PainterLogger.traceLog(Core::logger::LOG_WARNING,"solidPolygon: odd number of intersections at scanline "+std::to_string(y));
        }
        for(size_t i=0;i+1<AET.size();i+=2){
            int xStart=static_cast<int>(std::ceil(AET[i].curX));
            int xEnd=static_cast<int>(std::floor(AET[i+1].curX));
            if(xEnd>=xStart){
                alphaBlender(xStart,y,xEnd-xStart+1,1,color);
            }
        }
        for(auto &e:AET) e.curX+=e.invSlope;
    }
    return true;
}
bool Window::Painter::hollowCircle(const Window::Point& origin,int radius,const Core::Color& color){
    auto insideWindow=[this](int tx,int ty){
        return tx>=0&&tx<thisBindHandle->getRect().right&&ty>=0&&ty<thisBindHandle->getRect().bottom;
    };
    int ox=origin.x;
    int oy=origin.y;
    int x=0;
    int y=radius;
    int d=1-radius;
    while(x<=y){
        if(insideWindow(ox+x,oy+y))
        putUnitPixel(ox+x,oy+y,color);
        if(insideWindow(ox-x,oy+y))
        putUnitPixel(ox-x,oy+y,color);
        if(insideWindow(ox+x,oy-y))
        putUnitPixel(ox+x,oy-y,color);
        if(insideWindow(ox-x,oy-y))
        putUnitPixel(ox-x,oy-y,color);
        if(insideWindow(ox+y,oy+x))
        putUnitPixel(ox+y,oy+x,color);
        if(insideWindow(ox-y,oy+x))
        putUnitPixel(ox-y,oy+x,color);
        if(insideWindow(ox+y,oy-x))
        putUnitPixel(ox+y,oy-x,color);
        if(insideWindow(ox-y,oy-x))
        putUnitPixel(ox-y,oy-x,color);
        if(d<0){
            d=d+2*x+3;
        }
        else{
            d=d+2*(x-y)+5;
            y--;
        }
        x++;
    }
    return true;
}
bool Window::Painter::solidCircle(const Point& origin,int radius,const Core::Color& color){
    if(radius<0) return false;
    int ox=origin.x;
    int oy=origin.y;
    int width=this->thisBindHandle->getRect().right;
    int height=this->thisBindHandle->getRect().bottom;
    if(width<=0||height<=0) return false;
    for(int dy=-radius;dy<=radius;++dy){
        int y=oy+dy;
        if(y<0||y>=height) continue;
        int dx=static_cast<int>(std::floor(std::sqrt((double)radius*radius - (double)dy*dy)));
        int xStart=ox-dx;
        int xEnd=ox+dx;
        if(xEnd<0||xStart>=width) continue;
        if(xStart<0) xStart=0;
        if(xEnd>=width) xEnd=width-1;
        if(!alphaBlender(xStart,y,xEnd-xStart+1,1,color)) return false;
    }
    return hollowCircle(origin,radius,color);
}
Window::Point calculateDrawPosition(char locateMode,Window::Point locator,unsigned long long width,unsigned long long height){
    Window::Point result={locator.x,locator.y};
    switch(locateMode){
        case LOCATEMODE_CENTER:{
            result.x-=width/2;
            result.y-=height/2;
            break;
        }
        case LOCATEMODE_LEFTUPCORNER:{
            break;
        }
        case LOCATEMODE_RIGHTUPCORNER:{
            result.x-=width;
            break;
        }
        case LOCATEMODE_LEFTBOTTOMCORNER:{
            result.y-=height;
            break;
        }
        case LOCATEMODE_RIGHTBOTTOMCORNER:{
            result.x-=width;
            result.y-=height;
            break;
        }
        default:{
            break;
        }
    }
    return result;
}
bool Window::Painter::putImage(char locateMode,const Point& locator,const Assets::Image& src,unsigned char alpha){
    if(!thisHDC){
        PainterLogger.traceLog(Core::logger::LOG_WARNING,"This painter doesn't have an hdc yet");
        return false;
    }
    HBITMAP hbmp=src.getHBITMAP();
    if(!hbmp){
        PainterLogger.traceLog(Core::logger::LOG_ERROR,"Source image has no valid bitmap");
        return false;
    }
    HDC hdcMem=CreateCompatibleDC(thisHDC);
    HBITMAP hOldBitmap=static_cast<HBITMAP>(SelectObject(hdcMem,hbmp));
    ULONG_PTR width=src.getWidth();
    ULONG_PTR height=src.getHeight();
    Point drawPos=calculateDrawPosition(locateMode,locator,width,height);
    BLENDFUNCTION bf={AC_SRC_OVER,0,alpha,0};
    bf.BlendOp=AC_SRC_OVER;
    bf.BlendFlags=0;
    bf.SourceConstantAlpha=255;
    bf.AlphaFormat=AC_SRC_ALPHA;
    BOOL result=AlphaBlend(
        thisHDC,
        drawPos.x,drawPos.y,
        width,height,
        hdcMem,
        0,0,
        width,height,
        bf
    );
    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    return result != FALSE;
}
bool Window::Painter::putText(char locateMode,const Point& locator,const Assets::Font& font,const std::wstring& text,const Core::Color& color){
    SelectObject(this->thisHDC,font.thisHFont);
    TEXTMETRIC tm;
    SIZE size;
    GetTextMetrics(this->thisHDC,&tm);
    GetTextExtentPoint32W(this->thisHDC,text.c_str(),text.length(),&size);
    SetTextColor(this->thisHDC,color.toCOLORREF());
    SetBkMode(this->thisHDC,TRANSPARENT);
    Point locate=calculateDrawPosition(locateMode,locator,size.cx,size.cy);
    TextOutW(this->thisHDC,locate.x,locate.y,text.c_str(),text.length());
    return true;
}
Window::Point calcBezierPoint(double t,const vector<Window::Point>& vec){
    std::vector<Window::Point> temp=vec;
    int n=temp.size();
    for (int r=1;r<n;++r){
        for (int i=0;i<n-r;++i){
            temp[i].x=(1-t)*temp[i].x+t*temp[i+1].x;
            temp[i].y=(1-t)*temp[i].y+t*temp[i+1].y;
        }
    }
    return temp[0];
}
bool Window::Painter::bezierCurve(const vector<Point>& points,int accuracy,const Core::Color& color){
    if(points.size()<=2){
        PainterLogger.traceLog(Core::logger::LOG_WARNING,"expected more than 3 points,actually <=2,use line() instead");
        return false;
    }
    double step=1.0/accuracy;
    vector<Point> vec;
    double t=0.0;
    Point start=points.at(0);
    Point end=points.at(points.size()-1);
    vec.push_back(start);
    for(int i=1;i<accuracy;i++)
    {
        t+=step;
        vec.push_back(calcBezierPoint(t,points));
    }
    vec.push_back(end);
    for(unsigned int i=0;i<vec.size()-1;i++){
        bool result=line(vec[i],vec[i+1],color);
        if(!result){
            return false;
        }
    }
    return true;
}