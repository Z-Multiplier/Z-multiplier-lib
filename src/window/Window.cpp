//MIT License

//Copyright (c) 2026 wzxTheSlimeball

#include "Window.hpp"
#include "Logger.hpp"
#include <memory>
namespace Window{
    Core::logger WindowLogger;
    LRESULT CALLBACK Handle::Handle::thisWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
        switch(uMsg){
            case WM_DESTROY:{
                thisDestroy(hWnd,uMsg,wParam,lParam);
                break;
            }
        }
    }
    auto Handle::Handle::initWindow(const wchar_t* className,HINSTANCE hInstance){
        if(classIsRegistered(className)==FALSE){
            const wchar_t *CLASSNAME=className;
            WNDCLASS wc={};
            wc.lpfnWndProc=thisWindowProc;
            wc.lpszClassName=CLASSNAME;
            wc.hInstance=hInstance;
            RegisterClass(&wc);
        }
        return CreateWindowEx(this->mWindowStyle,className,this->mTitle.c_str(),WS_OVERLAPPEDWINDOW,this->x,this->y,this->width,this->height,
                              this->mParentWindow->mHWnd,NULL,hInstance,NULL);
    }
    Handle::Handle(int x,int y,int w,int h,HWND hWnd,int windowStyle,std::wstring title){
        this->x=x;
        this->y=y;
        this->width=w;
        this->height=h;
        this->mHWnd=hWnd;
        this->mWindowStyle=windowStyle;
        this->mTitle=title;
        this->mParentWindow=nullptr;
        globalHandleManager.push(shared_from_this());
        this->mID=globalHandleManager.getCnt();
    }
    Handle::Handle(int x,int y,int w,int h,HWND hWnd,int windowStyle,std::wstring title,Handle *ParentWindow){
        this->x=x;
        this->y=y;
        this->width=w;
        this->height=h;
        this->mHWnd=hWnd;
        this->mWindowStyle=windowStyle;
        this->mTitle=title;
        this->mParentWindow=ParentWindow;
        globalHandleManager.push(shared_from_this());
        this->mID=globalHandleManager.getCnt();
    }
    Handle::~Handle(){
        delete this->mParentWindow;
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
}