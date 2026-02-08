# SlimeballGraphicsLib帮助文档

*If you don't speak Chinese,scroll all the way down, you will find an English edition.(not yet)

- **版本** v0.0
- **状态** 施工中
- **最后更新** 26/02/07

## 项目信息

- **背景** 因为没有找到合适的多窗口库，又稍微懂一点面向对象，所以尝试再造一个轮子
- **目标** 
- - 主要目标：2D功能完全可用且单函数效率控制在可接受范围内（正常情况下，至少让窗口达到120fps）
- - 次要目标：如果时间宽裕，可能会尝试完成物理模拟及3D渲染（不着急，可能1~2年后）

函数/类对照表（斜体表示非必须，不包含在本表内的函数也可以使用但不建议，通常因为它们已废弃/已被结合包装/仅内部调用）
| 名称 | 命名空间或所属类 | 参数类型 | 返回值 | 用途 |
|:---|:---|:---|:---|:---|
|Color|Core/Graphics|N/A|N/A|通过RGBA或HSLA创建颜色|
|toCOLORREF()|Color|void|COLORREF|将本库的颜色结构体转化为COLORREF，注意这会失去alpha通道|
|FromCOLORREF()|Color|COLORREF|Color|将COLORREF转化为本库结构体，alpha值为255|
|logger|Core/Graphics|N/A|N/A|日志输出结构体，负责进行日志的输出，可以绑定不同的std::ostream，默认为clog|
|traceLog()|logger|LogLevel,std::string|void|日志打印纯文本字符串|
|varLog()|logger|LogLevel,std::string,any|void|formatLog的快速书写版本，可以输出单个变量的值，string为变量名|
|formatLog()|logger|LogLevel,std::string,any...|void|与printf()相同的格式化输出，除了LogLevel其余与printf用法相同|
|classIsRegistered()|Utils|wchar_t const*|BOOL|返回窗口类名是否被注册|
|Handle|Window/Graphics|N/A|N/A|本库的窗口结构体，可以创建多个实现多窗口，但是请使用规范方式创建|
|createInitWindow()|N/A|int,int,int,int,int,std::wstring,*hInstance*|std::pair\<std::shared_ptr\<Window::Handle>, HWND>|创建窗口的规范方式，可以观察参数名来使用|
|getRect()|Handle|void|RECT|将窗口转换为WIN32RECT结构|
|getHWnd()|Handle|void|HWND|获取窗口句柄|
|globalHandleManager.updateAll()|N/A|void|void|像对待一般函数一样对待它，从技术上讲，创建多个WindowManager虽然可行，但所需技术极高，不建议这么做|
|getBufferHDC()|Handle|void|HDC|返回窗口缓冲区的上下文，可以以此实现你的绘制函数|
|clearBuffer()|Handle|void|void|清空缓冲区，但在调用drawBackground后可能无意义|
|Painter|Window|N/A|N/A|本库笔刷结构体，一般无需手动定义|
|drawBackground()|Painter|Color|bool|以某种颜色填充背景|
|putPixel()|Painter|int,int,Color|bool|放置笔刷大小的“像素”|
|line()|Painter|Point,Point,Color|bool|bresenham直线，同样遵循笔刷大小|
|slopeLine()|Painter|Point,Point,Color|bool|斜率/DDA直线，遵循笔刷大小，若为solidPolygon()描边则推荐使用本函数|
|setSize()|Painter|int|void|设置笔刷半径|
|putUnitPixel()|Painter|int,int,Color|bool|放置单位像素，即1\*1像素|
|floodFill()|Painter|Point,Color|bool|洪水填充|
|hollowPolygon()|Painter|vector\<Point>,Color|bool|空心多边形|
|solidPolygon()|Painter|vector\<Point>,Color|bool|实行多边形|
|hollowCircle()|Painter|Point,int,Color|bool|圆心直径空心圆|


Sorry, the English edition is still writing, but if you are interested in this program, you could give a star