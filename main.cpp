/**************************************************************************
* includes
*/
#include <windows.h>    // include the basic windows header file
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> 
#include <math.h>
#include <io.h>
#include <fcntl.h>

#include <ddraw.h>  // directX includes
#include <dsound.h>
#include <dmksctrl.h>
#include <dmusici.h>
#include <dmusicc.h>
#include <dmusicf.h>
#include <dinput.h>
#include "T3DLIB1.h"    // DirectDraw 模块
#include "T3DLIB2.h"    // DirectInput 模块
#include "T3DLIB3.h"    // DirectSound / DirectMusic 模块


/**************************************************************************
* consts
*/
#define INITGUID    // 确保所有的 COM　接口可用，也可以不这样做，而是包含 .lib 文件 dxguid.lib
// 用于 Windows 接口的常量
#define WINDOW_CLASS_NAME "WIN3DCLASS"  // Windows 类名称
#define WINDOW_TITLE "T3D Graphics Console Version 2.0"
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_BPP    16    // 窗口的位深（8、16、24 等）
                            // 如果采用窗口模式，但不是全屏，位深必须与系统位深相同
                            // 另外，如果位深为 8 位，将创建一个调色板，并将其与应用程序关联起来
#define WINDOWED_APP  1     // 0 全屏模式，1 窗口模式


/*************************************************************************
* global variables
*/
HWND main_window_handle = NULL;     // 用于存储窗口句柄
HINSTANCE main_instance = NULL;     // 用于存储实例
char buffer[256];                   // 用于打印文本


/*************************************************************************
* function prototypes
*/
void createAndShowWindow(HINSTANCE hInstance, int nCmdShow);
void registerWindow(HINSTANCE hInstance);
void createWindow(HINSTANCE hInstance);
void adjustClientSizeToRequestedSize();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// 游戏控制台
int Game_Init(void* params = NULL);
int Game_Shutdown(void* params = NULL);
int Game_Main(void* params = NULL);


/**************************************************************************
* function definitions
*/
void createAndShowWindow(HINSTANCE hInstance, int nCmdShow) {
    registerWindow(hInstance);
    createWindow(hInstance);    // 将窗口句柄和实例存储到全局变量中
    ShowWindow(main_window_handle, SW_SHOW);
}

void registerWindow(HINSTANCE hInstance) {
    // this struct holds information for the window class
    // the 'EX' is there to indicate that this is the extended version of the struct WNDCLASS
    WNDCLASSEX winclass;

    // clear out the window class for use
    // ZeroMemory() is a function that initializes an entire block of memory to NULL.
    // The address provided in the first parameter sets where the block is to start.
    // The second parameter indicates how long the block is.
    ZeroMemory(&winclass, sizeof(WNDCLASSEX));

    // fill in the struct with the needed information
    winclass.cbSize = sizeof(WNDCLASSEX);
    winclass.style = CS_HREDRAW |                                   // What these two values do is tell Windows to redraw the window if it is moved vertically or horizontally.
                     CS_VREDRAW |
                     CS_OWNDC |
                     CS_DBLCLKS;
    winclass.lpfnWndProc = WindowProc;                              // "lpfnWndProc" tells the window class what function to use ("WindowProc()") when it gets a message from Windows
    winclass.cbClsExtra = 0;
    winclass.cbWndExtra = 0;
    winclass.hInstance = hInstance;                                 // a handle to a copy of our application
    winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    winclass.hCursor = LoadCursor(NULL, IDC_ARROW);                 // stores the default mouse image for the window class
    winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);   // contains the "brush" (is used to indicate the color of the background) that will be used to color the background of our window. 
    winclass.lpszMenuName = NULL;
    winclass.lpszClassName = WINDOW_CLASS_NAME;                     // the name of the window class we are building.

    RegisterClassEx(&winclass);
}

void createWindow(HINSTANCE hInstance) {
    // the handle for the window, filled by a function
    HWND hWnd = CreateWindowEx(NULL,
                               WINDOW_CLASS_NAME,                              // name of the window class
                               WINDOW_TITLE,                                   // title of the window
                               (WINDOWED_APP ? (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION) : (WS_POPUP | WS_VISIBLE)),  // window style
                               0,                                              // x-position of the window
                               0,                                              // y-position of the window
                               WINDOW_WIDTH,                                   // width of the window
                               WINDOW_HEIGHT,                                  // height of the window
                               NULL,                                           // we have no parent window, NULL
                               NULL,                                           // we aren't using menus, NULL
                               hInstance,                                      // application handle
                               NULL);                                          // used with multiple windows, NULL

    // 将窗口句柄和实例存储到全局变量中
    main_window_handle = hWnd;
    main_instance = hInstance;

    // 调整窗口大小，使 client_size 为 width * height
    adjustClientSizeToRequestedSize();
}

void adjustClientSizeToRequestedSize() {
    if (WINDOWED_APP) {
        // 调整窗口的大小，使 client_size 的大小等于请求的大小
        // 如果应该程序为窗口模式时，将由边框和控件调整
        // 应用程序不是窗口模式时，无需调整
        RECT window_rect = { 0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1 };

        // 调用函数来调整 window_rect
        AdjustWindowRectEx(&window_rect,
                           GetWindowStyle(main_window_handle),
                           GetMenu(main_window_handle) != NULL,
                           GetWindowExStyle(main_window_handle));

        // 将 client_size 偏移量保存到全局变量中，因为 DDraw_Flip() 要使用它们
        window_client_x0 = -window_rect.left;
        window_client_y0 = -window_rect.top;

        // 调用 MoveWindow() 来移动窗口
        MoveWindow(main_window_handle,
                   0,
                   0,
                   window_rect.right - window_rect.left,   // 宽度
                   window_rect.bottom - window_rect.top,   // 高度
                   FALSE);
    }
}

int Game_Init(void* params) {
    // 所有游戏初始化工作都在这个函数中执行

    // 启动 DirectDraw
    DDraw_Init(WINDOW_WIDTH,
               WINDOW_HEIGHT,
               WINDOW_BPP,
               WINDOWED_APP);

    // 初始化 DirectInput
    DInput_Init();

    // 接管键盘
    DInput_Init_Keyboard();

    // 在这里加入接管其它 DirectInput 设备的函数调用...

    // 初始化 DirectSound 和 DirectMusic
    DSound_Init();
    DMusic_Init();

    // 隐藏鼠标
    ShowCursor(FALSE);

    // 随机数生成器
    srand(Start_Clock());

    // 在这里加入初始化代码...

    // 成功返回
    return 1;
}

int Game_Shutdown(void* params) {
    // 在这个函数中关闭游戏并释放为游戏分配的所有资源

    // 关闭一切

    // 在这里加入释放为游戏分配的资源的代码...

    // 关闭 DirectSound
    DSound_Stop_All_Sounds();
    DSound_Delete_All_Sounds();
    DSound_Shutdown();

    // 关闭 DirectMusic
    DMusic_Delete_All_MIDI();
    DMusic_Shutdown();

    // 释放所有输出设备
    DInput_Release_Keyboard();

    // 关闭 DirectInput
    DInput_Shutdown();

    // 最后关闭 DirectDraw
    DDraw_Shutdown();

    // 成功返回
    return 1;
}

int Game_Main(void* params) {
    // 这是游戏核心，将不断地被实时调用
    // 它类似于 C 语言中的 main()，所有游戏调用都是在这里进行的

    int index;  // 循环变量

    // 启动定时时钟
    Start_Clock();

    // 清空缓存
    DDraw_Fill_Surface(lpddsback, 0);

    // 读取键盘和其它设备输入
    DInput_Read_Keyboard();

    // 在这里加入游戏逻辑代码...



    // 交换缓存
    DDraw_Flip();

    // 同步到 30 帧/秒
    Wait_Clock(30);

    // 检查用户是否要退出
    if (KEY_DOWN(VK_ESCAPE) ||
        keyboard_state[DIK_ESCAPE]) {
        PostMessage(main_window_handle, WM_DESTROY, 0, 0);
    }

    return 1;
}


/**************************************************************************
* main entry
*/
// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nShowCmd) {

    // 创建并显示 Windows 窗口
    createAndShowWindow(hInstance, nShowCmd);

    // 执行游戏控制台特有的初始化
    Game_Init();

    // 进入主事件循环
    MSG msg;
    while (true) {

        // 检测消息队列中是否有消息，如果有，读取它
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

            // 检测是否是退出消息
            if (msg.message == WM_QUIT) {
                break;
            }

            // 转换加速键
            TranslateMessage(&msg);

            // 将消息发送给 Window proc
            DispatchMessage(&msg);
        }

        // 主游戏处理逻辑
        Game_Main();
    }

    // 关闭游戏并释放所有资源
    Game_Shutdown();

    // 返回到 Windows 操作系统
    return (msg.wParam);
}

// the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam) {

    // 这是系统的主消d息处理函数
    PAINTSTRUCT paintStruct;    // 用于 WM_PAINT
    HDC hdc;                    // Handle of Device Context，设备场景句柄

    // sort through and find what code to run for the message given
    switch (message) {
        case WM_CREATE: {
            // 执行初始化的代码
            return 0;
        } break;
        case WM_PAINT: {
            // 开始绘制
            hdc = BeginPaint(hWnd, &paintStruct);
            // 结束绘制
            EndPaint(hWnd, &paintStruct);
            // 成功返回
            return 0;
        } break;
        case WM_DESTROY: {      // this message is read when the window is closed
            // close the application entirely
            PostQuitMessage(0);
            return 0;
        } break;
        default:
            break;
    }

    // Handle any messages the switch statement didn't
    return DefWindowProc(hWnd, message, wParam, lParam);
}