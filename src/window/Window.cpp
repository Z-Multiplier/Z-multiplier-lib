//MIT License

//Copyright (c) 2026 wzxTheSlimeball

#include "Window.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "Painter.hpp"
#include <memory>
#include <algorithm>
#include <thread>
namespace Window{
    Core::logger WindowLogger;
    HandleManager globalHandleManager;
    std::unordered_map<HWND,Handle*> Handle::mHWndMap;
    LRESULT CALLBACK Handle::Handle::thisWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
        Handle *pThis=nullptr;
        if(mHWndMap.find(hWnd)!=mHWndMap.end()){
            pThis=mHWndMap[hWnd];
        }
        if(uMsg==WM_NCCREATE||uMsg==WM_CREATE){
            CREATESTRUCT* cs=reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis=reinterpret_cast<Handle*>(cs->lpCreateParams);
            if(pThis){
                pThis->mHWnd=hWnd;
                mHWndMap[hWnd]=pThis;
            }
        }
        if(pThis==nullptr){
            WindowLogger.traceLog(Core::logger::LOG_INFO,"Failed to find pointer \"pThis\",This will return default(or if this is the first log that the Window throws,it's all fine)");
            return DefWindowProc(hWnd,uMsg,wParam,lParam);
        }
        if(pThis->mID==0){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"You have not registered this to the manager yet!");
            return 0;
        }
        switch(uMsg){
            case WM_DESTROY:{
                if(!pThis->thisDestroy){
                    WindowLogger.traceLog(Core::logger::LOG_WARNING,"The function\"thisDestroy\" is not defined yet,Skipping.");
                }
                else{
                    pThis->thisDestroy(hWnd,uMsg,wParam,lParam);
                }
                pThis->destroy();
                break;
            }
            case WM_PAINT:{
                Window::Painter thisPainter(hWnd,pThis);
                thisPainter.switchHDC();
                if(!pThis->thisPaint){
                    WindowLogger.traceLog(Core::logger::LOG_WARNING,"The function\"thisPaint\" is not defined yet,Skipping.");
                }
                else{
                    pThis->thisPaint(hWnd,uMsg,wParam,lParam,thisPainter);
                    return 0;
                }
                break;
            }
            case WM_CLOSE:{
                if(!pThis->thisOnClose){
                    WindowLogger.traceLog(Core::logger::LOG_WARNING,"The function\"thisOnClose\" is not defined yet,Skipping.");
                }
                else{
                    if(pThis->thisOnClose(hWnd,uMsg,wParam,lParam)){
                        pThis->destroy();
                        DestroyWindow(hWnd);
                    }
                    else{
                        return 0;
                    }
                }
                break;
            }
        }
        return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
    HWND Handle::Handle::initWindow(const wchar_t* className,HINSTANCE hInstance){
        if(Utils::classIsRegistered(className)==FALSE){
            const wchar_t *CLASSNAME=className;
            WNDCLASS wc={};
            wc.lpfnWndProc=thisWindowProc;
            wc.lpszClassName=CLASSNAME;
            wc.hInstance=hInstance;
            RegisterClass(&wc);
        }
        this->mHWnd=CreateWindowEx(this->mWindowStyle,className,this->mTitle.c_str(),WS_OVERLAPPEDWINDOW,this->x,this->y,this->width,this->height,
                                   this->mParentWindow==nullptr?NULL:this->mParentWindow->mHWnd,NULL,hInstance,this);
        if(this->mHWnd!=NULL){
            ShowWindow(this->mHWnd, SW_SHOW);
            UpdateWindow(this->mHWnd);
        }
        return this->mHWnd;
    }
    Handle::Handle(int x,int y,int w,int h,int windowStyle,std::wstring title){
        this->x=x;
        this->y=y;
        this->width=w;
        this->height=h;
        this->mWindowStyle=windowStyle;
        this->mTitle=title;
        this->mHWnd=NULL;
        this->mParentWindow=nullptr;
        this->mID=globalHandleManager.getCnt();
        this->thisCanvas=Core::Canvas(w,h);
    }
    Handle::Handle(int x,int y,int w,int h,int windowStyle,std::wstring title,Handle *ParentWindow){
        this->x=x;
        this->y=y;
        this->width=w;
        this->height=h;
        this->mWindowStyle=windowStyle;
        this->mTitle=title;
        this->mHWnd=NULL;
        this->mParentWindow=ParentWindow;
        this->mID=globalHandleManager.getCnt();
        this->thisCanvas=Core::Canvas(w,h);
    }
    Handle::~Handle(){
        mHWndMap.erase(this->mHWnd);
    }
    bool HandleManager::isEmpty(){
        return this->handles.empty();
    }
    bool HandleManager::checkAndQuit(){
        if(this->isEmpty()){
            WindowLogger.traceLog(Core::logger::LOG_INFO,"Quitting program");
            PostQuitMessage(0);
            return true;
        }
        return false;
    }
    void HandleManager::push(std::shared_ptr<Handle> handle){
        this->handles.push_back(handle);
        this->handlesID.push_back(cnt++);
    }
    long long HandleManager::getCnt(){
        return cnt;
    }
    HandleManager::~HandleManager(){
        for(auto &handle:handles){
            handle.reset();
        }
    }
    void HandleManager::pop(long long ID){
        auto it=std::find(this->handlesID.begin(),this->handlesID.end(), ID);
        if(it!=this->handlesID.end()){
            size_t index=std::distance(this->handlesID.begin(),it);
            this->handles.erase(this->handles.begin()+index);
            this->handlesID.erase(it);
        }
    }
    void Handle::registerToManager(){
        this->mID=globalHandleManager.getCnt();
        globalHandleManager.push(shared_from_this());
    }
    void Handle::destroy(){
        this->isActive=false;
        globalHandleManager.pop(this->mID);
    }
    Handle* Handle::queryWindow(HWND hWnd){
        if(Handle::mHWndMap.find(hWnd)!=Handle::mHWndMap.end()){
            return Handle::mHWndMap[hWnd];
        }
        return nullptr;
    }
    RECT Handle::getRect(){
        RECT rect;
        GetWindowRect(this->mHWnd,&rect);
        return rect;
    }
    HWND Handle::getHWnd(){
        return this->mHWnd;
    }
    void HandleManager::updateAll(){
        for(auto &handle:handles){
            InvalidateRect(handle->getHWnd(),NULL,TRUE);
            UpdateWindow(handle->getHWnd());
        }
    }
    bool Handle::initBuffer(){
        if(!mHWnd){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"The window doesn't have an HWnd yet,maybe it's not created properly?");
            return false;
        }
        if(this->mBuffer.memHDC!=NULL){
            WindowLogger.traceLog(Core::logger::LOG_INFO,"Initialized this buffer,or the HDC broke down");
            return true;
        }
        RECT rect;
        GetClientRect(this->mHWnd,&rect);
        int rectwidth=rect.right;
        int rectheight=rect.bottom;
        HDC windowDC=GetDC(this->mHWnd);
        HDC hdc=CreateCompatibleDC(windowDC);
        if(!hdc){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create compatible DC for buffer");
            return false;
        }
        HBITMAP hbmp=CreateCompatibleBitmap(windowDC,rectwidth,rectheight);
        ReleaseDC(this->mHWnd,windowDC);
        if(!hbmp){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create compatible bitmap for buffer");
            DeleteDC(hdc);
            return false;
        }
        this->mBuffer.memHDC=hdc;
        this->mBuffer.oldBmp=(HBITMAP)SelectObject(hdc,hbmp);
        this->mBuffer.width=rectwidth;
        this->mBuffer.height=rectheight;
        clearBuffer();
        return true;
    }
    HDC Handle::getBufferHDC(){
        return this->mBuffer.memHDC;
    }
    void Handle::clearBuffer(){
        if(!this->mBuffer.memHDC){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"The buffer is not initialized yet,maybe you forgot to call initBuffer()?");
            return;
        }
        RECT rect;
        GetClientRect(this->mHWnd,&rect);
        FillRect(this->mBuffer.memHDC,&rect,GetSysColorBrush(COLOR_WINDOW));
    }
    void Handle::update(){
        if(!this->mBuffer.memHDC||!this->mHWnd){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"The buffer or the window is not initialized yet, your fault:P");
            return;
        }
        HDC hdc=GetDC(this->mHWnd);
        BitBlt(hdc,0,0,this->mBuffer.width,this->mBuffer.height,this->mBuffer.memHDC,0,0,SRCCOPY);
        this->thisCanvas=Core::Canvas(this->mBuffer.memHDC,width,height);
        ReleaseDC(this->mHWnd,hdc);
    }
    void Handle::resizeBuffer(){
        if(!this->mBuffer.memHDC||!this->mHWnd){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"The buffer or the window is not initialized yet, your fault:P");
            return;
        }
        RECT rect;
        GetClientRect(this->mHWnd,&rect);
        int rectwidth=rect.right;
        int rectheight=rect.bottom;
        HDC newhdc=CreateCompatibleDC(this->mBuffer.memHDC);
        if(!newhdc){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create compatible DC for buffer");
            DeleteDC(this->mBuffer.memHDC);
            return;
        }
        HBITMAP hbmp=CreateCompatibleBitmap(newhdc,rectwidth,rectheight);
        if(!hbmp){
            WindowLogger.traceLog(Core::logger::LOG_ERROR,"Failed to create compatible bitmap for buffer");
            DeleteDC(newhdc);
            return;
        }
        HBITMAP currentOldBmp=this->mBuffer.oldBmp;
        SelectObject(newhdc,hbmp);
        SelectObject(this->mBuffer.memHDC,currentOldBmp);
        DeleteObject(currentOldBmp);
        DeleteDC(this->mBuffer.memHDC);
        this->mBuffer.memHDC=newhdc;
        this->mBuffer.oldBmp=hbmp;
        this->mBuffer.width=rectwidth;
        this->mBuffer.height=rectheight;
        clearBuffer();
    }
    Buffer& Handle::getBuffer(){
        return this->mBuffer;
    }
}