#include "Graphics.hpp"
using namespace Graphics;

long long mainWindowPaint(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,Painter& painter){
    painter.setSize(1);
    painter.drawBackground(Color((unsigned char)255,255,255,255));
    painter.bezierCurve({{10,10},{10,190},{190,190}},5,Color((unsigned char)0,0,0,255));
    return 0;
}
int main(){
    auto mainWindow=createInitWindow(400,400,207,230,L"Window");
    mainWindow.first->thisPaint=mainWindowPaint;
    MSG msg={};
    while(msg.message!=WM_QUIT){
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(16);
        globalHandleManager.updateAll();
    }
}