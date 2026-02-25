#include "Graphics.hpp"
#include "Game.hpp"
#include <memory>
#include <algorithm>  // for clamp

using namespace std;

const int GRID_ROWS = 30;          // 行数（垂直）
const int GRID_COLS = 40;          // 列数（水平）
const int CELL_SIZE = 20;          // 格子像素大小
const int WINDOW_WIDTH = GRID_COLS * CELL_SIZE;
const int WINDOW_HEIGHT = GRID_ROWS * CELL_SIZE;

// 全局对象
unique_ptr<Game::Terrain> g_terrain;
Game::Life g_life;
shared_ptr<Window::Handle> g_windowHandle;
HWND g_hWnd;

int main() {
    // 1. 创建窗口
    auto [handle, hwnd] = createInitWindow(100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, L"Blue Ball Wander");
    g_windowHandle = handle;
    g_hWnd = hwnd;

    // 2. 设置绘制回调
    handle->thisPaint = [](HWND, UINT, WPARAM, LPARAM, Window::Painter& painter) -> LONGLONG {
        painter.drawBackground(Core::Color((unsigned char)255, 255, 255));  // 白色背景

        // 从 attributes 中获取当前像素坐标（float），但绘制时需要 int
        float px = std::get<float>(*g_life["px"]);
        float py = std::get<float>(*g_life["py"]);
        int radius = CELL_SIZE / 2 - 2;  // 小球半径略小于格子一半

        painter.solidCircle(Window::Point(static_cast<int>(px), static_cast<int>(py)),
                            radius, Core::Color((unsigned char)0, 0, 255));
        return 0;
    };

    // 3. 初始化地形
    vector<string> gridTypes = Game::Defaults::defaultGridTypes;
    vector<pair<string, wstring>> textures;   // 不使用纹理
    auto terrainGrid = Game::Defaults::terrainAllWalkable(GRID_ROWS, GRID_COLS);
    unordered_map<string, float> costRules = Game::Defaults::defaultCostRules;

    g_terrain = make_unique<Game::Terrain>(
        WINDOW_HEIGHT, WINDOW_WIDTH,
        GRID_ROWS, GRID_COLS,
        gridTypes, textures, terrainGrid, costRules
    );
    Game::nowTerrain = g_terrain.get();

    // 4. 初始化小球
    // 设置网格坐标
    g_life.attributes["x"] = GRID_ROWS / 2;
    g_life.attributes["y"] = GRID_COLS / 2;
    // 根据网格坐标计算像素坐标（格子中心）
    float startPx = (GRID_ROWS / 2) * CELL_SIZE + CELL_SIZE / 2.0f;
    float startPy = (GRID_COLS / 2) * CELL_SIZE + CELL_SIZE / 2.0f;
    g_life.attributes["px"] = startPx;
    g_life.attributes["py"] = startPy;
    g_life.attributes["facing"] = 0.0f;

    // 5. 添加 AI 行为
    // 速度设为 5.0 像素/帧（避免太快），范围 5 格，频率 0.01
    g_life.behaviors.push_back(Game::Defaults::wander(5.0f, 5, 0.01f));
    g_life.behaviors.push_back(Game::Defaults::updateAttributes());

    // 6. 消息循环
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 执行 AI 逻辑
        g_life.act();

        // 请求重绘
        InvalidateRect(g_hWnd, NULL, TRUE);

        Sleep(16);  // 约 60 FPS
        Graphics::globalHandleManager.updateAll();  // 触发窗口绘制
    }

    return 0;
}