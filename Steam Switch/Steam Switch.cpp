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
#define MB_WAIT_TIMEOUT 30000 // 30 seconds
#define IDM_EXIT 105
#define IDM_SYNC 106
#define IDM_CLEAR 107
#define IDM_CLEAR_SINGLE 108
#define IDM_DEVICE_NOT_FOUND 109
#define IDM_DEVICE_NOT_FOUND_EXIT 110
#define NBOFEVENTS 6
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
SteamHandler* steamHandler = nullptr;
HDEVNOTIFY hDeviceSerial = NULL;
HPOWERNOTIFY hPowerNotify = NULL;

// Use a guid to uniquely identify our icon
class __declspec(uuid("9D0B8B92-4E1C-488e-A1E1-2331AFCE2CB5")) SteamSwitchIcon;
static UINT WM_TaskBarCreated = 0;
DWORD32 controllerCount = -1;
std::wstring controllerCountWStr;
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
   if (steamHandler && steamHandler->monHandler)
   {
       steamHandler->monHandler->StartCecPowerThread(steamHandler);
   }

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   if (hDeviceSerial == NULL)
   {
	   DEV_BROADCAST_DEVICEINTERFACE serialFilter = { };
	   serialFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	   serialFilter.dbcc_classguid = GUID_DEVINTERFACE_COMPORT;
	   serialFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	   hDeviceSerial = RegisterDeviceNotification(hWnd, &serialFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
   }
   if (hPowerNotify == NULL)
   {
	   hPowerNotify = RegisterSuspendResumeNotification(hWnd, DEVICE_NOTIFY_WINDOW_HANDLE);
   }
   WM_TaskBarCreated = RegisterWindowMessageW(L"TaskbarCreated");
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
    GenericInputDeviceChange(hWnd, message, wParam, lParam);
    switch (message)
    {
	case WM_POWERBROADCAST:
	{
		if ( wParam == PBT_APMRESUMEAUTOMATIC ||
			wParam == PBT_APMRESUMESUSPEND )
		{

			if (steamHandler && steamHandler->monHandler && steamHandler->monHandler->isSingleDisplayHDMI())
			{
				HANDLE hResumedFromSleep = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"ResumedFromSleep");
				if (hResumedFromSleep && WaitForSingleObject(hResumedFromSleep, 1) == WAIT_TIMEOUT)
				{
					if (steamHandler)
					{
						steamHandler->serialHandler.ScanForSerialDevices();
					}
					SetEvent(hResumedFromSleep);
					CloseHandle(hResumedFromSleep);
				}
				HANDLE hCECPowerOnEvent = OpenEventW( SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"CECPowerOnEvent");
				if (hCECPowerOnEvent && WaitForSingleObject(hCECPowerOnEvent, 1) == WAIT_TIMEOUT)
				{
					SetEvent(hCECPowerOnEvent);
					CloseHandle(hCECPowerOnEvent);
				}
			}
		}
        if (wParam == PBT_APMSUSPEND)
        {
			if (steamHandler && steamHandler->monHandler && steamHandler->monHandler->isSingleDisplayHDMI())
			{
				steamHandler->monHandler->StandByAllDevicesCEC();
				WaitForSingleObject(steamHandler->monHandler->hCECPowerOffFinishedEvent, 6000);
			}
        }
		break;
	}
    case WM_CREATE:
    {
        AddNotificationIcon(hWnd);
        break;
    }
	case APPWM_ICONNOTIFY:
	{
		switch (lParam)
		{
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			HMENU Hmenu = CreatePopupMenu();
			if (controllerCount != -1)
			{
				if (controllerCount == 1)
				{
					controllerCountWStr = L"Enable Pairing Mode: (1) controller paired.";
				}
				else
				{
					controllerCountWStr = L"Enable Pairing Mode: (" + std::to_wstring(controllerCount) + L") controllers paired.";
				}

				if (controllerCount > 0)
				{
					AppendMenuW(Hmenu, MF_STRING, IDM_CLEAR, L"Clear All Paired Controllers");
					AppendMenuW(Hmenu, MF_STRING, IDM_CLEAR_SINGLE, L"Clear a Single Paired Controller");
				}

				AppendMenuW(Hmenu, MF_STRING, IDM_SYNC, controllerCountWStr.c_str());
				AppendMenuW(Hmenu, MF_STRING, IDM_EXIT, L"Close Steam Switch");
			}
			else
			{
				AppendMenuW(Hmenu, MF_STRING | MF_DISABLED, IDM_DEVICE_NOT_FOUND, L"The Raspberry Pi ZeroW2 device was not found.");
				AppendMenuW(Hmenu, MF_STRING, IDM_DEVICE_NOT_FOUND_EXIT, L"Close Steam Switch");
			}
			POINT p;
			GetCursorPos(&p);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(Hmenu, TPM_LEFTBUTTON, p.x, p.y, 0, hWnd, 0);
			PostMessage(hWnd, WM_NULL, 0, 0);
			break;
		}
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_DEVICE_NOT_FOUND:
		{
			MessageBoxW(hWnd, L"Do not run Steam Switch unless you have the required Raspberry Pi ZeroW2 serial device connected to your computer.", L"Steam Switch Error", MB_OK | MB_ICONERROR);
			break;
		}
		case IDM_DEVICE_NOT_FOUND_EXIT:
		{
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
		case IDM_EXIT:
		{
			HANDLE hLockDeviceEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"LockDeviceEvent");
			HANDLE hShutdownEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, L"ShutdownEvent");
			HANDLE hNewDeviceEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");
			if (hNewDeviceEvent && WaitForSingleObject(hNewDeviceEvent, 1) != WAIT_OBJECT_0)
			{
				SetEvent(hNewDeviceEvent);
			}
			if (hLockDeviceEvent)
			{
				SetEvent(hLockDeviceEvent);
				if (hShutdownEvent && WaitForSingleObject(hShutdownEvent, MB_WAIT_TIMEOUT) == WAIT_OBJECT_0)
				{
					MessageBoxW(hWnd, L"The device is now locked and can be unlocked by running Steam Switch again.", L"Steam Switch Important Information", MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					MessageBoxW(hWnd, L"The device may be unlocked and could power down the your computer unexpectedly when a paired controlled is used.\n\nRun Steam Switch again to attempt to fix this.\n\nDo not run Steam Switch unless you have the required Raspberry Pi ZeroW2 serial device connected to your computer at all times.", L"Steam Switch Error", MB_OK | MB_ICONERROR);
				}
			}
			if (hLockDeviceEvent)
			{
				CloseHandle(hLockDeviceEvent);
			}
			if (hShutdownEvent)
			{
				CloseHandle(hShutdownEvent);
			}
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		}
		case IDM_SYNC:
		{
			int selection = MessageBoxW(hWnd, L"Enable paring mode for the Raspberry Pi ZeroW2 device?\nAnother message box window will appear to indicate if a controller was paired successfully or not.\nClick ok to continue or click cancel to exit.", L"Steam Switch", MB_OKCANCEL | MB_ICONQUESTION);
			switch (selection)
			{
			case IDCANCEL:
				return DefWindowProc(hWnd, message, wParam, lParam);
			case IDOK:
			{
				HANDLE hSyncEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"SyncEvent");
				HANDLE hFinshedSyncEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, L"FinshedSyncEvent");
				HANDLE hNewDeviceEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");

				if (hNewDeviceEvent && WaitForSingleObject(hNewDeviceEvent, 1) != WAIT_OBJECT_0)
				{
					SetEvent(hNewDeviceEvent);
				}
				if (hSyncEvent && hFinshedSyncEvent)
				{
					SetEvent(hSyncEvent);
					if (WaitForSingleObject(hFinshedSyncEvent, MB_WAIT_TIMEOUT) == WAIT_OBJECT_0)
					{
						MessageBoxW(hWnd, L"The controller is now paired with the Raspberry Pi ZeroW2 device. To unpair this controller select the option \"Clear All Paired Controllers\" or \"Clear a Single Paired Controller\" from the system tray menu using the icon on the task bar.", L"GIPSerial Important Information", MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						MessageBoxW(hWnd, L"The Raspberry Pi ZeroW2 device did not find any new controllers to pair.", L"Steam Switch Error", MB_OK | MB_ICONWARNING);
					}
					ResetEvent(hFinshedSyncEvent);
				}
				if (hSyncEvent)
				{
					CloseHandle(hSyncEvent);
				}
				if (hFinshedSyncEvent)
				{
					CloseHandle(hFinshedSyncEvent);
				}
			}
			}
			break;
		}
		case IDM_CLEAR_SINGLE:
		{
			int selection = MessageBoxW(hWnd, L"Warning: This option will attempt to unpair the next controller that tries to connect to the Raspberry Pi ZeroW2 device.\nClick ok to continue or click cancel to exit.", L"GIPSerial Warning", MB_OKCANCEL | MB_ICONWARNING);
			switch (selection)
			{
			case IDCANCEL:
				return DefWindowProc(hWnd, message, wParam, lParam);
			case IDOK:
			{
				HANDLE hClearSingleEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"ClearSingleEvent");
				HANDLE hFinshedClearSingleEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, L"FinshedClearSingleEvent");
				HANDLE hNewDeviceEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");

				if (hNewDeviceEvent && WaitForSingleObject(hNewDeviceEvent, 1) != WAIT_OBJECT_0)
				{
					SetEvent(hNewDeviceEvent);
				}
				if (hClearSingleEvent && hFinshedClearSingleEvent)
				{
					SetEvent(hClearSingleEvent);
					if (WaitForSingleObject(hFinshedClearSingleEvent, MB_WAIT_TIMEOUT) == WAIT_OBJECT_0)
					{
						MessageBoxW(hWnd, L"The controller was unpaired successfully.\nYou can now safety pair this controller to another device.\nTo pair again click the option \"Enable Pairing Mode\" from the system tray menu using the icon on the task bar.", L"GIPSerial Important Information", MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						MessageBoxW(hWnd, L"The Raspberry Pi ZeroW2 device was unable to clear a single paired controller.\nTry restarting your computer before trying again.", L"GIPSerial Error", MB_OK | MB_ICONERROR);
					}
					ResetEvent(hFinshedClearSingleEvent);
				}
				if (hClearSingleEvent)
				{
					CloseHandle(hClearSingleEvent);
				}
				if (hFinshedClearSingleEvent)
				{
					CloseHandle(hFinshedClearSingleEvent);
				}
			}
			}
			break;
		}
		case IDM_CLEAR:
		{
			int selection = MessageBoxW(hWnd, L"Warning: This option will attempt to unpair all paired controllers.\nClick ok to continue or click cancel to exit.", L"GIPSerial Warning", MB_OKCANCEL | MB_ICONWARNING);
			switch (selection)
			{
			case IDCANCEL:
				return DefWindowProc(hWnd, message, wParam, lParam);
			case IDOK:
			{
				HANDLE hClearAllEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"ClearAllEvent");
				HANDLE hFinshedClearAllEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, L"FinshedClearAllEvent");
				HANDLE hNewDeviceEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"NewDeviceEvent");

				if (hNewDeviceEvent && WaitForSingleObject(hNewDeviceEvent, 1) != WAIT_OBJECT_0)
				{
					SetEvent(hNewDeviceEvent);
				}
				if (hClearAllEvent && hFinshedClearAllEvent)
				{
					ResetEvent(hClearAllEvent);
					SetEvent(hClearAllEvent);
					if (WaitForSingleObject(hFinshedClearAllEvent, MB_WAIT_TIMEOUT) == WAIT_OBJECT_0)
					{
						MessageBoxW(hWnd, L"All controllers were unpaired successfully.\nYou can now safety pair this controller to another device.\nTo pair again click the option \"Enable Pairing Mode\" from the system tray menu using the icon on the task bar.", L"GIPSerial Important Information", MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						MessageBoxW(hWnd, L"The Raspberry Pi ZeroW2 device was unable to clear all paired controller.\nTry restarting your computer before trying again.", L"Steam Switch Error", MB_OK | MB_ICONWARNING);
					}
					ResetEvent(hFinshedClearAllEvent);
				}
				if (hClearAllEvent)
				{
					CloseHandle(hClearAllEvent);
				}
				if (hFinshedClearAllEvent)
				{
					CloseHandle(hFinshedClearAllEvent);
				}
			}
			}
			break;
		}
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
            if (steamHandler)
            {
				steamHandler->serialHandler.ScanForSerialDevices();
            }
		}
        break;
    }
    case WM_QUERYENDSESSION:
    {
		if (steamHandler && steamHandler->isSteamInBigPictureMode &&
            steamHandler->monHandler && steamHandler->monHandler->isSingleDisplayHDMI())
		{
			steamHandler->monHandler->StandByAllDevicesCEC();
			WaitForSingleObject(steamHandler->monHandler->hCECPowerOffFinishedEvent, 6000);
		}
        break;
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
			break;
        }
    case WM_DESTROY:
    {
		if (hDeviceSerial)
		{
			UnregisterDeviceNotification(hDeviceSerial);
			hDeviceSerial = NULL;
		}
		if (hPowerNotify)
		{
			UnregisterSuspendResumeNotification(hPowerNotify);
			hPowerNotify = NULL;
		}
        PostQuitMessage(0);
        break;
    }
    default:
    {
        if (message == WM_TaskBarCreated)
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
        lstrcpyW(nid.szTip, L"Click here to open Steam Switch options");
        nid.uCallbackMessage = APPWM_ICONNOTIFY;
	return Shell_NotifyIconW(NIM_ADD, &nid);
}