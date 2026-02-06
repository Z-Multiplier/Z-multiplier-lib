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
#include "Painter.hpp"
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
            static std::unordered_map<HWND,Handle*> mHWndMap;
            bool isActive=true;
            Buffer mBuffer;
        public:
            Core::Canvas thisCanvas;
            std::function<long long(HWND,UINT,WPARAM,LPARAM)> thisDestroy;
            std::function<bool(HWND,UINT,WPARAM,LPARAM)> thisOnClose;//return true to close,false to not close
            std::function<long long(HWND,UINT,WPARAM,LPARAM,Window::Painter&)> thisPaint;
            static LRESULT CALLBACK thisWindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
            HWND initWindow(const wchar_t* className,HINSTANCE hInstance);
            Handle()=default;
            Handle(int x,int y,int w,int h,int windowStyle,std::wstring title);
            Handle(int x,int y,int w,int h,int windowStyle,std::wstring title,Handle *ParentWindow);
            ~Handle();
            void destroy();
            void registerToManager();
            static Handle* queryWindow(HWND hWnd);
            RECT getRect();
            HWND getHWnd();
            bool initBuffer();
            HDC getBufferHDC();
            void update();
            void clearBuffer();
            void resizeBuffer();
            Buffer& getBuffer();
    };
    struct HandleManager{
        private:
            long long cnt=1;
            vector<long long> handlesID;
            vector<std::shared_ptr<Handle>> handles;
        public:
            bool isEmpty();
            bool checkAndQuit();
            void push(std::shared_ptr<Handle> handle);
            void pop(long long id);
            long long getCnt();
            void updateAll();
            HandleManager()=default;
            ~HandleManager();
    };
    extern HandleManager globalHandleManager;
}
#endif // WINDOW_HPP