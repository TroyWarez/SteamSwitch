#pragma once
#include "framework.h"
class InputHandler
{
public:
	InputHandler();
	~InputHandler();
	void SendControllerInput(PXINPUT_STATE pXstate);
	void turnOffXinputController();
	XINPUT_STATE lastXstate;
	HINSTANCE hXInputDLL;

};

