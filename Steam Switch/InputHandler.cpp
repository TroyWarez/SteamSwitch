#include "InputHandler.h"
#include "SteamHandler.h"
#include "XInputDeviceIO.h"
extern SteamHandler* steamHandler;
#define MOUSE_OFFSET 0.1111
InputHandler::InputHandler()
{
	hXInputDLL = nullptr;
	lastXstate = { 0x0 };
	ZeroMemory(&lastXstate, sizeof(lastXstate));
	hXInputDLL = LoadLibraryW(L"XInput1_3.dll");
	xbox_init();
}
InputHandler::~InputHandler()
{
	if (hXInputDLL)
	{
		FreeLibrary(hXInputDLL);
	}
}
void InputHandler::SendControllerInput(PXINPUT_STATE pXstate)
{
	if (pXstate)
	{
		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP &&
			!lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP )
		{
			std::array<INPUT, 2> inputs = { 0 };
			ZeroMemory(inputs.data(), sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_UP;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_UP;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(static_cast<int>(inputs.size()), inputs.data(), sizeof(INPUT));
		}
		if ((((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) && !(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)) ||
			(((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && !(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)) )
		{
			std::array<INPUT, 2> inputs = { 0 };
			ZeroMemory(inputs.data(), sizeof(inputs));
					


			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_LEFT;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_LEFT;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(static_cast<int>(inputs.size()), inputs.data(), sizeof(INPUT));
		}
		if((((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) && !(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)))
		{
			std::array<INPUT, 2> inputs = { 0 };
			ZeroMemory(inputs.data(), sizeof(inputs));



			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_RIGHT;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_RIGHT;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(static_cast<int>(inputs.size()), inputs.data(), sizeof(INPUT));
		}
		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN &&
			!(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN))
		{
			std::array<INPUT, 2> inputs = { 0 };
			ZeroMemory(inputs.data(), sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_DOWN;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_DOWN;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(static_cast<int>(inputs.size()), inputs.data(), sizeof(INPUT));
		}

		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_A &&
			!(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) )
		{
			std::array<INPUT, 2> inputs = { 0 };
			ZeroMemory(inputs.data(), sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_RETURN;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_RETURN;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(static_cast<int>(inputs.size()), inputs.data(), sizeof(INPUT));

		}

		if ((*pXstate).Gamepad.wButtons & XINPUT_GAMEPAD_START &&
			!(lastXstate.Gamepad.wButtons & XINPUT_GAMEPAD_START))
		{
			std::array<INPUT, 2> inputs = { 0 };
			ZeroMemory(inputs.data(), sizeof(inputs));

			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_RETURN;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_RETURN;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			UINT uSent = SendInput(static_cast<int>(inputs.size()), inputs.data(), sizeof(INPUT));
		}
	}
	if (pXstate)
	{
		lastXstate = (*pXstate);
	}
}
const void InputHandler::turnOffXinputController()
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