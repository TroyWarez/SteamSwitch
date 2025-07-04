// Steam Switch.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Steam Switch.h"
#include "SteamHandler.h"
#include "AudioHandler.h"
#include "MonitorHandler.h"
#include "Settings.h"
#define MAX_LOADSTRING 100
#define APPWM_ICONNOTIFY (WM_APP + 1)

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Use a guid to uniquely identify our icon
class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) SteamSwitchIcon;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
BOOL                AddNotificationIcon(HWND hwnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

AudioHandler audioHandler;
std::wstring defaultAudioDevice = defaultBpAudioDevice;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	HANDLE mutex = CreateMutex(0, 0, "SteamSwitchMutex");
    MSG msg = {};

	switch (GetLastError())
	{
	case ERROR_ALREADY_EXISTS:
		// app already running
		break;

	case ERROR_SUCCESS:
		// first instance
		SteamHandler* steamHandler = new SteamHandler();

		// Initialize global strings
		LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadStringW(hInstance, IDC_STEAMSWITCH, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// Perform application initialization:
		if (!InitInstance(hInstance, nCmdShow))
		{
			return FALSE;
		}

		HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STEAMSWITCH));

		// Main message loop:
        return steamHandler->StartSteamHandler();
	}
	return FALSE;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = 0;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STEAMSWITCH));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_STEAMSWITCH);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 0,
      0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   //SteamHandler* steamHandler = new SteamHandler();

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   return TRUE;
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTaskbarRestart = 0;

    switch (message)
    {
    case WM_CREATE:
    {
        AddNotificationIcon(hWnd);
        s_uTaskbarRestart = RegisterWindowMessageW(L"TaskbarCreated");
        break;
    }
    case APPWM_ICONNOTIFY:
    {
		switch (lParam)
		{
		case WM_LBUTTONUP:
            PostQuitMessage(0);
			break;
		}
        break;
    }
    case WM_DEVICECHANGE:
    {
        if (wParam == DBT_DEVNODES_CHANGED)
        {
			audioHandler.InitDefaultAudioDevice(defaultBpAudioDevice);
		}
        break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
    {
        if (message == s_uTaskbarRestart)
        {
            AddNotificationIcon(hWnd);
        }
		return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
    return 0;
}

BOOL AddNotificationIcon(HWND hwnd)
{
	NOTIFYICONDATA nid;
        nid.hWnd = hwnd;
	    nid.cbSize = sizeof(NOTIFYICONDATAA_V3_SIZE);
	    nid.uTimeout = 500;
	    nid.uID = 1;
	    nid.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE | NIF_INFO | 0x00000080;
	    nid.uCallbackMessage = WM_USER + 200;
	    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_STEAMSWITCH));
	    lstrcpyA(nid.szTip, "Steam Switch");
	    lstrcpyA(nid.szInfoTitle, "Steam Switch");
        nid.uCallbackMessage = APPWM_ICONNOTIFY;
	return Shell_NotifyIconA(NIM_ADD, &nid);
}