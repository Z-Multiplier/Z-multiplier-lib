#include "Graphics.hpp"
using namespace Graphics;
long long mainWindowDrawer(HWND hWndm,UINT message,WPARAM wParam,LPARAM lParam,Painter& painter){
    globalLogger.formatLog(Core::logger::LOG_NOTE,"hWndm:%p,message:%u,wParam:%p,lParam:%p",hWndm,message,wParam,lParam);
    //Again,make the compiler shut up
    painter.drawBackground(Color((unsigned char)255,0,0));//add unsigned char so the compiler knows that you wanna use the RGB init.
    //so this will draw a pure red background
    painter.present();
    return 0;
}//this is the window's "thisPaint" function
int main(){
    auto mainWindow=createInitWindow(0,0,800,600,0,L"Window");//Well actually we have this
    //it returns a pair, the pair.first is the Handle, the pair.second is the HWND
    //also,the function's last parameter is default to mainHInstance,which is better than GetModuleHandle(NULL) we used before.
    std::function<long long(HWND,UINT,WPARAM,LPARAM,Painter&)> mainWindowDrawerFunc=mainWindowDrawer;
    mainWindow.first->thisPaint=mainWindowDrawerFunc;
    //the window should proceed the paint function every update.
    MSG msg={};//again,don't forget this
    while(msg.message!=WM_QUIT){
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //Proceed your main loop code here
        if(mainWindow.second!=NULL){
            globalLogger.traceLog(Core::logger::LOG_NOTE,"loop #");
        };//Just make the compiler shut up
        globalHandleManager.updateAll();
        Sleep(100);
        globalHandleManager.checkAndQuit();
    }
    return 0;
}