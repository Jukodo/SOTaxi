#pragma once
#include "framework.h"
#include "MapInfo.h"
#include "CenDLL.h"
#include "MIService.h"

#define MAX_LOADSTRING 100
#define APP_NAME TEXT("Base")
#define WINDOW_TITLE TEXT("MapInfo")

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
Application* app;

RECT rectWindowProp;
HDC memDC = NULL;  //double buffering
HBITMAP hBit = NULL;
HBRUSH roadBrush;
HBRUSH structureBrush;
HBRUSH cellBorderBrush;
HBRUSH taxiBrush;
HBRUSH passengerBrush;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow){
    #pragma region Setup Window

    //More info at: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexw
    WNDCLASSEXW windowInfo;
    windowInfo.cbSize = sizeof(WNDCLASSEX);                                             //The size of this struct

    //Window intern settings
    windowInfo.style = CS_HREDRAW | CS_VREDRAW;                                         //Styles of the window (More info at: https://docs.microsoft.com/en-us/windows/win32/winmsg/window-class-styles)
    windowInfo.lpfnWndProc = WndProc;                                                   //Callback function to handle events
    windowInfo.cbClsExtra = 0;                                                          //The number of extra bytes to allocate following the window-class structure
    windowInfo.cbWndExtra = 0;                                                          //The number of extra bytes to allocate following the window instance
    windowInfo.hInstance = hInstance;                                                   //Handle of the instance that contains the window procedure
    windowInfo.lpszClassName = APP_NAME;                                                //String to the name of the app

    //Window visual settings
    windowInfo.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAPINFO));              //Handle of the icon
    windowInfo.hIconSm = LoadIcon(windowInfo.hInstance, MAKEINTRESOURCE(IDI_SMALL));    //Handle of the small icon
    windowInfo.hCursor = LoadCursor(NULL, IDC_ARROW);                                   //Handle of the cursor
    windowInfo.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);                               //Handle of the background brush
    windowInfo.lpszMenuName = MAKEINTRESOURCEW(IDC_MAPINFO);                           //String to the menu name, if integer use MAKEINTRESOURCE(value)

    //Registers a window class for subsequent use in calls to the CreateWindow or CreateWindowEx function
    if(!RegisterClassEx(&windowInfo))
        return false;

    int maxScreenWidth = (int) round(GetSystemMetrics(SM_CXSCREEN) * 0.9); //Only fill 80% of the screen
    int maxScreenHeight = (int) round(GetSystemMetrics(SM_CYSCREEN) * 0.9); //Only fill 80% of the screen
    TCHAR message[100];
    swprintf_s(message, 100, TEXT("%s80%% of screen size is %dx%d\n"), Utils_NewLine(), maxScreenWidth, maxScreenHeight);
    OutputDebugString(message);



    //More info at: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww
    HWND hWindow = CreateWindowW(   //Creates a new window (overlapped, pop-up or child window)
        APP_NAME,                   //String to the title of the window
        WINDOW_TITLE,               //String to the title of the window
        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),    //Window style values (More info at: https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles)
        CW_USEDEFAULT,              //Initial X position of window
        0,                          //Initial Y position of window
        maxScreenWidth,             //Width value of the window
        maxScreenHeight,            //Height value of the window
        NULL,                       //Handle of the parent of the window being created
        NULL,                       //Handle of the menu for the window being created
        hInstance,                  //Handle of the instance of the module to be associated with the created window
        NULL);                      //Additional parameters

    if(!hWindow)
        return false;
    #pragma endregion

    app = malloc(sizeof(Application));
    if(!Setup_Application(app, hWindow)){
        swprintf_s(message, 100, TEXT("%sError trying to set up central..."), Utils_NewLine());
        OutputDebugString(message);
        _gettchar();
        return false;
    }

    roadBrush = CreateSolidBrush(RGB(220, 220, 220));
    structureBrush = CreateSolidBrush(RGB(30, 30, 30));
    cellBorderBrush = CreateSolidBrush(RGB(15, 15, 15));
    taxiBrush = CreateSolidBrush(RGB(240, 80, 80));
    passengerBrush = CreateSolidBrush(RGB(80, 80, 240));

    //More info at: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
    ShowWindow(     //Changes the window's display state
        hWindow,    //Handle of the window
        nCmdShow);  //Value of new display state (first execution should always be nCmdShow)

    ////More info at: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-updatewindow
    //UpdateWindow(   //Sends WM_PAINT to the window, forcing the paint command
    //    hWindow);   //Handle to the window being updated

    //Start Refresh Routine
    if(!Setup_OpenThreadHandles_RefreshRoutine(&app->threadHandles, hWindow)){
        return false;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAPINFO));
    MSG msg;

    // Main message loop:
    while(GetMessage(&msg, NULL, 0, 0)){
        if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
    HDC hdc;
    PAINTSTRUCT ps;
    
    switch(message){
    case WM_CREATE:
        GetClientRect(hWnd, &rectWindowProp);
        hdc = GetDC(hWnd);
        memDC = CreateCompatibleDC(hdc);
        hBit = CreateCompatibleBitmap(hdc, rectWindowProp.right, rectWindowProp.bottom);
        SelectObject(memDC, hBit);
        DeleteObject(hBit);

        PatBlt(memDC, 0, 0, rectWindowProp.right, rectWindowProp.bottom, PATCOPY);

        ReleaseDC(hWnd, hdc);

        break;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
        {
            Paint_DrawMap(app, memDC, roadBrush, structureBrush, cellBorderBrush);
            Paint_MapCoordinates(app, memDC, cellBorderBrush);
            Paint_Taxis(app, memDC, taxiBrush);
            Paint_Passengers(app, memDC, passengerBrush);

            hdc = BeginPaint(hWnd, &ps);
            BitBlt(hdc, 0, 0, rectWindowProp.right, rectWindowProp.bottom, memDC, 0, 0, SRCCOPY);
            
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_ERASEBKGND: //Prevent flickering effect by disabling erase background
            return(1);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    UNREFERENCED_PARAMETER(lParam);
    switch(message){
    case WM_INITDIALOG:
        return (INT_PTR) TRUE;

    case WM_COMMAND:
        if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR) TRUE;
        }
        break;
    }
    return (INT_PTR) FALSE;
}
