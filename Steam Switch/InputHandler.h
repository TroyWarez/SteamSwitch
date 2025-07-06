#pragma once
#include "framework.h"
class InputHandler
{
public:
	InputHandler();
	~InputHandler();
	void SendControllerInput(PXINPUT_STATE pXstate);
	XINPUT_STATE lastXstate;

};

