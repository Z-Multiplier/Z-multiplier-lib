# Z-Multiplier Lib
------------------
#### **Still under development**

本图形库是基于GDI和GDI+的**CPU 2D**的，**Windows平台**~~图形~~游戏库，可以自主实现3D
This library is a **CPU-based 2D ~~graphics~~ game library** for **Windows**, built on GDI and GDI+,allows you to implement 3D rendering manually.

#### 主要特点：

- 易用性和易维护性平衡：函数调用自然，融入逻辑，同时几乎所有绘制函数都返回`bool`类型进行错误处理
- 完善的日志系统：通过`logger`调用日志，支持绑定不同的输出流，预设globalLogger绑定`clog`，可通过`freopen`等手段重定向至文件
- 功能全面：支持常用的图形，图片加载和字体加载，窗口回调齐全，均可自定义，详见下方样例
- 模块化：易于寻找声明及实现

#### Main features:

- Balance of ease of use and maintenance: function calls are natural and integrated into logic, while almost all drawing functions return the `bool` type for error handling
- Complete log system: call logs through `logger`, support binding different output streams, default globalLogger is bound to `clog`, and can be redirected to files through `freopen` and other means
- Comprehensive functions: supports commonly used graphics, image loading and font loading, comprehensive window callback support, all can be customized, see the example below for details
- Modular: easy to find declarations and implementations

下面是一个画椭圆的样例代码：
Here is an example code of drawing an ellipse:
```cpp
#include "Graphics.hpp"
using namespace Graphics;
long long mainWindowPaint(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam,Painter& painter){
    painter.drawBackground(Color((unsigned char)255,255,255,255));
    painter.solidEllipse(Point(100,100),50,25,Color((unsigned char)0,0,0,255));
    hWnd;uMsg;wParam;lParam;//make the compiler shut up,I hate -Werror
    return 0;
}
int main(){
    auto mainWindow=createInitWindow(400,400,207,230,L"Window");//x,y,width,height,title
    //notice that the width and the height includes the non-client area,
    //after some tests, I found +7,+30 is good, but it may change as the environment changes
    //The type that createInitWindow() return is std::pair<std::shared_ptr<Handle>,HWND>
    mainWindow.first->thisPaint=mainWindowPaint;
    MSG msg={};
    while(msg.message!=WM_QUIT){
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(16);//60FPS
        globalHandleManager.updateAll();//literally
    }
}
```

本项目采用[MIT许可](/COPYING)
This project is licensed under the [MIT license](/COPYING).

#### 安装与配置

#### Installation

**Notice that you need GDI and GDI+ to compile,which usually already exist in Windows SDK,but just in case some beginners may put meaningless issues**

I'm sorry,but there is not a command that could copy now.
If you wanna use now,just download the [`/src`](/src/) and [`/include`](/include/), and that should be enough.