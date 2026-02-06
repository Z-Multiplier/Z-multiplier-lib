#include "Graphics.hpp"
using namespace Graphics;
long long mainWindowDrawer(HWND hWndm,UINT message,WPARAM wParam,LPARAM lParam,Painter& painter){
    painter.drawBackground(Color((unsigned char)255,255,255,255));
    painter.setSize(1);
    painter.line({50,50},{50,500},Color((unsigned char)0,0,0,255));
    painter.line({500,50},{50,50},Color((unsigned char)0,0,0,255));
    painter.line({500,50},{500,500},Color((unsigned char)0,0,0,255));
    painter.line({50,500},{500,500},Color((unsigned char)0,0,0,255));
    painter.floodFill({75,75},Color((unsigned char)0,255,0,255));
    painter.present();
    return 0;
}//this is the window's "thisPaint" function
int main(){
    auto mainWindow=createInitWindow(0,0,800,600,0,L"Window");
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
            //globalLogger.traceLog(Core::logger::LOG_NOTE,"loop #");
        }//Just make the compiler shut up
        //Sleep(16);
        globalHandleManager.updateAll();
        globalHandleManager.checkAndQuit();
    }
    return 0;
}