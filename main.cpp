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
#include "T3DLIB1.h"    // DirectDraw ģ��
#include "T3DLIB2.h"    // DirectInput ģ��
#include "T3DLIB3.h"    // DirectSound / DirectMusic ģ��


/**************************************************************************
* consts
*/
#define INITGUID    // ȷ�����е� COM���ӿڿ��ã�Ҳ���Բ������������ǰ��� .lib �ļ� dxguid.lib
// ���� Windows �ӿڵĳ���
#define WINDOW_CLASS_NAME "WIN3DCLASS"  // Windows ������
#define WINDOW_TITLE "T3D Graphics Console Version 2.0"
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_BPP    16    // ���ڵ�λ�8��16��24 �ȣ�
                            // ������ô���ģʽ��������ȫ����λ�������ϵͳλ����ͬ
                            // ���⣬���λ��Ϊ 8 λ��������һ����ɫ�壬��������Ӧ�ó����������
#define WINDOWED_APP  1     // 0 ȫ��ģʽ��1 ����ģʽ


/*************************************************************************
* global variables
*/
HWND main_window_handle = NULL;     // ���ڴ洢���ھ��
HINSTANCE main_instance = NULL;     // ���ڴ洢ʵ��
char buffer[256];                   // ���ڴ�ӡ�ı�


/*************************************************************************
* function prototypes
*/
void createAndShowWindow(HINSTANCE hInstance, int nCmdShow);
void registerWindow(HINSTANCE hInstance);
void createWindow(HINSTANCE hInstance);
void adjustClientSizeToRequestedSize();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// ��Ϸ����̨
int Game_Init(void* params = NULL);
int Game_Shutdown(void* params = NULL);
int Game_Main(void* params = NULL);


/**************************************************************************
* function definitions
*/
void createAndShowWindow(HINSTANCE hInstance, int nCmdShow) {
    registerWindow(hInstance);
    createWindow(hInstance);    // �����ھ����ʵ���洢��ȫ�ֱ�����
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

    // �����ھ����ʵ���洢��ȫ�ֱ�����
    main_window_handle = hWnd;
    main_instance = hInstance;

    // �������ڴ�С��ʹ client_size Ϊ width * height
    adjustClientSizeToRequestedSize();
}

void adjustClientSizeToRequestedSize() {
    if (WINDOWED_APP) {
        // �������ڵĴ�С��ʹ client_size �Ĵ�С��������Ĵ�С
        // ���Ӧ�ó���Ϊ����ģʽʱ�����ɱ߿�Ϳؼ�����
        // Ӧ�ó����Ǵ���ģʽʱ���������
        RECT window_rect = { 0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1 };

        // ���ú��������� window_rect
        AdjustWindowRectEx(&window_rect,
                           GetWindowStyle(main_window_handle),
                           GetMenu(main_window_handle) != NULL,
                           GetWindowExStyle(main_window_handle));

        // �� client_size ƫ�������浽ȫ�ֱ����У���Ϊ DDraw_Flip() Ҫʹ������
        window_client_x0 = -window_rect.left;
        window_client_y0 = -window_rect.top;

        // ���� MoveWindow() ���ƶ�����
        MoveWindow(main_window_handle,
                   0,
                   0,
                   window_rect.right - window_rect.left,   // ���
                   window_rect.bottom - window_rect.top,   // �߶�
                   FALSE);
    }
}

int Game_Init(void* params) {
    // ������Ϸ��ʼ�������������������ִ��

    // ���� DirectDraw
    DDraw_Init(WINDOW_WIDTH,
               WINDOW_HEIGHT,
               WINDOW_BPP,
               WINDOWED_APP);

    // ��ʼ�� DirectInput
    DInput_Init();

    // �ӹܼ���
    DInput_Init_Keyboard();

    // ���������ӹ����� DirectInput �豸�ĺ�������...

    // ��ʼ�� DirectSound �� DirectMusic
    DSound_Init();
    DMusic_Init();

    // �������
    ShowCursor(FALSE);

    // �����������
    srand(Start_Clock());

    // ����������ʼ������...

    // �ɹ�����
    return 1;
}

int Game_Shutdown(void* params) {
    // ����������йر���Ϸ���ͷ�Ϊ��Ϸ�����������Դ

    // �ر�һ��

    // ����������ͷ�Ϊ��Ϸ�������Դ�Ĵ���...

    // �ر� DirectSound
    DSound_Stop_All_Sounds();
    DSound_Delete_All_Sounds();
    DSound_Shutdown();

    // �ر� DirectMusic
    DMusic_Delete_All_MIDI();
    DMusic_Shutdown();

    // �ͷ���������豸
    DInput_Release_Keyboard();

    // �ر� DirectInput
    DInput_Shutdown();

    // ���ر� DirectDraw
    DDraw_Shutdown();

    // �ɹ�����
    return 1;
}

int Game_Main(void* params) {
    // ������Ϸ���ģ������ϵر�ʵʱ����
    // �������� C �����е� main()��������Ϸ���ö�����������е�

    int index;  // ѭ������

    // ������ʱʱ��
    Start_Clock();

    // ��ջ���
    DDraw_Fill_Surface(lpddsback, 0);

    // ��ȡ���̺������豸����
    DInput_Read_Keyboard();

    // �����������Ϸ�߼�����...



    // ��������
    DDraw_Flip();

    // ͬ���� 30 ֡/��
    Wait_Clock(30);

    // ����û��Ƿ�Ҫ�˳�
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

    // ��������ʾ Windows ����
    createAndShowWindow(hInstance, nShowCmd);

    // ִ����Ϸ����̨���еĳ�ʼ��
    Game_Init();

    // �������¼�ѭ��
    MSG msg;
    while (true) {

        // �����Ϣ�������Ƿ�����Ϣ������У���ȡ��
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

            // ����Ƿ����˳���Ϣ
            if (msg.message == WM_QUIT) {
                break;
            }

            // ת�����ټ�
            TranslateMessage(&msg);

            // ����Ϣ���͸� Window proc
            DispatchMessage(&msg);
        }

        // ����Ϸ�����߼�
        Game_Main();
    }

    // �ر���Ϸ���ͷ�������Դ
    Game_Shutdown();

    // ���ص� Windows ����ϵͳ
    return (msg.wParam);
}

// the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam) {

    // ����ϵͳ������dϢ������
    PAINTSTRUCT paintStruct;    // ���� WM_PAINT
    HDC hdc;                    // Handle of Device Context���豸�������

    // sort through and find what code to run for the message given
    switch (message) {
        case WM_CREATE: {
            // ִ�г�ʼ���Ĵ���
            return 0;
        } break;
        case WM_PAINT: {
            // ��ʼ����
            hdc = BeginPaint(hWnd, &paintStruct);
            // ��������
            EndPaint(hWnd, &paintStruct);
            // �ɹ�����
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