#pragma once
/// interface
#include "framework.h"
#define XBOX_MAX_CONTROLLERS 16

#define XBOX_DPAD_UP          0x0001
#define XBOX_DPAD_DOWN        0x0002
#define XBOX_DPAD_LEFT        0x0004
#define XBOX_DPAD_RIGHT       0x0008
#define XBOX_START            0x0010 // or "view"
#define XBOX_BACK             0x0020 // or "menu"
#define XBOX_LEFT_THUMB       0x0040
#define XBOX_RIGHT_THUMB      0x0080
#define XBOX_LEFT_SHOULDER    0x0100
#define XBOX_RIGHT_SHOULDER   0x0200
#define XBOX_GUIDE            0x0400 // or "xbox" button
#define XBOX_A                0x1000
#define XBOX_B                0x2000
#define XBOX_X                0x4000
#define XBOX_Y                0x8000

typedef struct
{
	DWORD packet;
	WORD buttons;
	BYTE left_trigger;
	BYTE right_trigger;
	SHORT left_thumb_x;
	SHORT left_thumb_y;
	SHORT right_thumb_x;
	SHORT right_thumb_y;
} xbox_state;

typedef struct
{
	BYTE type;
	BYTE subtype;
	WORD flags;

	DWORD buttons;
	BYTE left_trigger;
	BYTE right_trigger;
	SHORT left_thumb_x;
	SHORT left_thumb_y;
	SHORT right_thumb_x;
	SHORT right_thumb_y;

	BYTE low_freq;
	BYTE high_freq;
} xbox_caps;

typedef struct
{
	BYTE type;
	BYTE level;
} xbox_battery;

// populate initial list of devices
void xbox_init();

// add new device, call this from WM_DEVICECHANGE message when wparam is DBT_DEVICEARRIVAL
// returns index on success, or negative number on failure
int xbox_connect(LPWSTR path);

// removes existing device, call this from WM_DEVICECHANGE message when wparam is DBT_DEVICEREMOVECOMPLETE
// returns index on success, or negative number on failure (wrong path)
int xbox_disconnect(LPWSTR path);

// functions return 0 on success, negative value most likely means disconnect
int xbox_get_caps(DWORD index, xbox_caps* caps);
int xbox_get_battery(DWORD index, xbox_battery* bat);
int xbox_get(DWORD index, XINPUT_STATE* state);
int xbox_set(DWORD index, BYTE low_freq, BYTE high_freq);


/// implementation

struct
{
	HANDLE handle;
	WCHAR path[MAX_PATH];
}
static xbox_devices[XBOX_MAX_CONTROLLERS];

static const GUID xbox_guid = { 0xec87f1e3, 0xc13b, 0x4100, { 0xb5, 0xf7, 0x8b, 0x84, 0xd5, 0x42, 0x60, 0xcb } };