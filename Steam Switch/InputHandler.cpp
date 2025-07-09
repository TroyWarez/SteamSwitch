#include "InputHandler.h"
#include "SteamHandler.h"
#include "XInputDeviceIO.h"
extern SteamHandler* steamHandler;
#define MOUSE_OFFSET 0.1111
InputHandler::InputHandler()
{
	ZeroMemory(&lastXstate, sizeof(lastXstate));
	hXInputDLL = LoadLibraryW(L"XInput1_3.dll");
	xbox_init();
	//gameInput = nullptr;
	//gamepad = nullptr;
	//HRESULT hr = GameInput::v1::GameInputCreate(&gameInput);
	//int a = 1;
}
InputHandler::~InputHandler()
{
	if (hXInputDLL)
	{
		FreeLibrary(hXInputDLL);
	}
	//if (gameInput)
	//{
	//	gameInput->Release();
	//}
}
void InputHandler::SendControllerInput(PXINPUT_STATE pXstate)
{
	if (pXstate)
	{
		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP &&
			!lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP )
		{
			INPUT inputs[2] = {};
			ZeroMemory(inputs, sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_UP;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_UP;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
		}
		if ((((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) && !(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)) ||
			(((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && !(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)) )
		{
			INPUT inputs[2] = {};
			ZeroMemory(inputs, sizeof(inputs));
					


			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_LEFT;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_LEFT;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
		}
		if((((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && !(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)))
		{
			INPUT inputs[2] = {};
			ZeroMemory(inputs, sizeof(inputs));



			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_RIGHT;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_RIGHT;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
		}
		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN &&
			!(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN))
		{
			INPUT inputs[2] = {};
			ZeroMemory(inputs, sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_DOWN;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_DOWN;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
		}

		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_A &&
			!(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) )
		{
			INPUT inputs[2] = {};
			ZeroMemory(inputs, sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_RETURN;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_RETURN;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
			if (steamHandler)
			{
				//HWND bphWnd = FindWindowW(SDL_CLASS, steamHandler->getSteamBigPictureModeTitle());
				//ShowWindow(bphWnd, SW_SHOW);
				//SetActiveWindow(bphWnd);
				//SetForegroundWindow(bphWnd);
				//SwitchToThisWindow(bphWnd, TRUE);
			}

		}

		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_START &&
			!(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_START))
		{
			INPUT inputs[2] = {};
			ZeroMemory(inputs, sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_RETURN;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_RETURN;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
		}
	}
	if (pXstate)
	{
		lastXstate = (*pXstate);
	}
}
void InputHandler::turnOffXinputController()
{
	if (hXInputDLL)
	{
		for (short i = 0; i < 4; ++i)
		{
			XINPUT_STATE state;
			memset(&state, 0, sizeof(XINPUT_STATE));

			if (XInputGetState(i, &state) == ERROR_SUCCESS)
			{
				typedef DWORD(WINAPI* XInputPowerOffController_t)(DWORD i);
				XInputPowerOffController_t realXInputPowerOffController = (XInputPowerOffController_t)GetProcAddress(hXInputDLL, (LPCSTR)103);
				realXInputPowerOffController(i);
			}

			ZeroMemory(&state, sizeof(XINPUT_STATE));
		}
	}
}
DWORD InputHandler::GetXInputStateDeviceIO(DWORD index, PXINPUT_STATE pXstate)
{
	return xbox_get(0, pXstate);
}