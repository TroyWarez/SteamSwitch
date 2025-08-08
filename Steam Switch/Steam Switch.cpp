// Steam Switch.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Steam Switch.h"
#include "SteamHandler.h"
#include "AudioHandler.h"
#include "MonitorHandler.h"
#include <GenericInput.h>
#define MAX_LOADSTRING 100 
#define APPWM_ICONNOTIFY (WM_APP + 1)
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
SteamHandler* steamHandler = nullptr;
// Use a guid to uniquely identify our icon
class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) SteamSwitchIcon;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
BOOL                AddNotificationIcon(HWND hwnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

AudioHandler audioHandler;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	HRESULT hr = CoInitialize(nullptr);

	if (FAILED(hr))
	{
		return FALSE;
	} 

	HANDLE mutex = CreateMutex(0, 0, "SteamSwitchMutex");
    MSG msg = {};

	switch (GetLastError())
	{
	case ERROR_ALREADY_EXISTS:
		// app already running
		break;

    case ERROR_SUCCESS:
    {
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
   if (GenericInputInit(hWnd, FALSE) == ERROR_GEN_FAILURE)
   {
	   return FALSE;
   }
   GenericInputDeviceChange(hWnd, 0, 0, 0);
   steamHandler = new SteamHandler(hWnd);

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
    GenericInputDeviceChange(hWnd, message, wParam, lParam);
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
		DEV_BROADCAST_HDR* hdr = (DEV_BROADCAST_HDR*)lParam;
		if (lParam && hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
		{
			DEV_BROADCAST_DEVICEINTERFACE_W* dif = (DEV_BROADCAST_DEVICEINTERFACE_W*)hdr;
			if (wParam == DBT_DEVICEARRIVAL)
			{
				DWORD index = xbox_connect(dif->dbcc_name);
			}
			else if (wParam == DBT_DEVICEREMOVECOMPLETE)
			{
				DWORD index = xbox_disconnect(dif->dbcc_name);
			}
		}
		if (wParam == DBT_DEVNODES_CHANGED)
		{
            audioHandler.InitDefaultAudioDevice();
		}
        break;
    }
    case WM_ENDSESSION:
    {
		if (steamHandler)
		{
			steamHandler->inputHandler->turnOffXinputController();
			if (steamHandler->isSteamInBigPictureMode)
			{
				steamHandler->monHandler->ToggleMode(false);
			}
		}
        PostQuitMessage(0);
        break;
    }
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
	NOTIFYICONDATAW nid;
        nid.hWnd = hwnd;
	    nid.cbSize = sizeof(NOTIFYICONDATAW_V3_SIZE);
	    nid.uTimeout = 500;
	    nid.uID = 1;
	    nid.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE | NIF_INFO | 0x00000080;
	    nid.uCallbackMessage = WM_USER + 200;
	    nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_STEAMSWITCH));
        std::copy(L"Click here to close Steam Switch", L"Click here to close Steam Switch" + 33, nid.szTip);
        std::copy(L"Click here to close Steam Switch", L"Click here to close Steam Switch" + 13, nid.szTip);
        nid.uCallbackMessage = APPWM_ICONNOTIFY;
	return Shell_NotifyIconW(NIM_ADD, &nid);
}