//MIT License

//Copyright (c) 2026 wzxTheSlimeball
#ifndef WINDOW_HPP
#define WINDOW_HPP
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <string>
#include <functional>
#include <memory>
#include "Canvas.hpp"
#include "Utils.hpp"

namespace Window{
    struct Handle:public std::enable_shared_from_this<Handle>{
        private:
            int x,y;
            int width,height;
            HWND mHWnd;
            int mWindowStyle;
            std::wstring mTitle;
            Handle *mParentWindow;
            long long mID;
        public:
            static std::function<long long(HWND,UINT,WPARAM,LPARAM)> thisDestroy;
            static LRESULT CALLBACK thisWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
            auto initWindow(const wchar_t* className,HINSTANCE hInstance);
            Handle()=default;
            Handle(int x,int y,int w,int h,HWND hWnd,int windowStyle,std::wstring title);
            Handle(int x,int y,int w,int h,HWND hWnd,int windowStyle,std::wstring title,Handle *ParentWindow);
            ~Handle();
    };
    struct HandleManager{
        private:
            long long cnt=0;
            vector<long long> handlesID;
            vector<std::shared_ptr<Handle>> handles;
        public:
            bool isEmpty();
            bool checkAndQuit();
            void push(std::shared_ptr<Handle> handle);
            long long getCnt();
            HandleManager()=default;
            ~HandleManager()=default;
    }globalHandleManager;
}
#endif // WINDOW_HPP