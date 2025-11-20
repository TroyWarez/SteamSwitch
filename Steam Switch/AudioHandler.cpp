#include "AudioHandler.h"
AudioHandler::AudioHandler()
{
    if (FAILED(CoInitialize(nullptr)))
    {
        return;
    }
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum)))
    {
        pEnum = nullptr;
    }
    if (FAILED(CoCreateInstance(__uuidof(CPolicyConfigVistaClient),
		nullptr, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID*)&pPolicyConfig)))
    {
        pPolicyConfig = nullptr;
	}
	WCHAR userProfile[MAX_PATH] = { 0 };
	ExpandEnvironmentStringsW(L"%userProfile%", userProfile, MAX_PATH);
	std::wstring userProfileFilePath(userProfile);
    std::wstring userProfileLastFilePath(userProfile);
    std::wstring userProfileDeskPath(userProfile);
    userProfileFilePath = userProfileFilePath + L"\\SteamSwitch\\BPAudioDevice.txt";
    userProfileLastFilePath = userProfileLastFilePath + L"\\SteamSwitch\\LastBPAudioDevice.txt";
    userProfileDeskPath = userProfileDeskPath + L"\\SteamSwitch\\DESKAudioDevice.txt";
	HANDLE hFile = CreateFileW(userProfileFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	DWORD re3 = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        CHAR buffer[MAX_PATH] = { 0 };
        WCHAR wbuffer[MAX_PATH] = { 0 };
        bytesRead = GetFileSize(hFile, nullptr);
        if (bytesRead > 0 && bytesRead < MAX_PATH)
        {
            if (ReadFile(hFile, buffer, bytesRead, &bytesRead, nullptr))
            {
				MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, wbuffer, MAX_PATH);
                BPaudioDeviceName = wbuffer;
            }
		}
        CloseHandle(hFile);
    }
    else
    {
		MessageBoxW(nullptr, (L"Could not default audio file: " + userProfileFilePath + L"\nError code: " + std::to_wstring(re3)).c_str(), L"Error", MB_ICONERROR);
		PostQuitMessage(0);
    }

	hFile = CreateFileW(userProfileLastFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesRead;
		CHAR buffer[MAX_PATH] = { 0 };
		WCHAR wbuffer[MAX_PATH] = { 0 };
		bytesRead = GetFileSize(hFile, nullptr);
		if (bytesRead > 0 && bytesRead < MAX_PATH)
		{
			if (ReadFile(hFile, buffer, bytesRead, &bytesRead, nullptr))
			{
				MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, wbuffer, MAX_PATH);
				lastBPaudioDeviceName = wbuffer;
			}
		}
		CloseHandle(hFile);
	}

	hFile = CreateFileW(userProfileDeskPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesRead;
		CHAR buffer[MAX_PATH] = { 0 };
		WCHAR wbuffer[MAX_PATH] = { 0 };
		bytesRead = GetFileSize(hFile, nullptr);
		if (bytesRead > 0 && bytesRead < MAX_PATH)
		{
			if (ReadFile(hFile, buffer, bytesRead, &bytesRead, nullptr))
			{
				MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, wbuffer, MAX_PATH);
				DESKaudioDeviceName = wbuffer;
			}
		}
		CloseHandle(hFile);
	}
}
AudioHandler::~AudioHandler()
{
    if (pEnum)
    {
       pEnum->Release();
	}
    if (pPolicyConfig)
    {
		pPolicyConfig->Release();
    }
    CoUninitialize();
}
void AudioHandler::ToggleAudioDevice()
{
    if (BPaudioDeviceName == L"" || lastBPaudioDeviceName == L"" || BPaudioDeviceName == lastBPaudioDeviceName)
    {
        return;
	}


    std::wstring temp = BPaudioDeviceName;
    BPaudioDeviceName = lastBPaudioDeviceName;
    lastBPaudioDeviceName = temp;
    InitDefaultAudioDevice();
    WCHAR userProfile[MAX_PATH] = { 0 };
    ExpandEnvironmentStringsW(L"%userProfile%", userProfile, MAX_PATH);
    std::wstring userProfileFilePath(userProfile);
    std::wstring userProfileLastFilePath(userProfile);
    userProfileFilePath = userProfileFilePath + L"\\SteamSwitch\\BPAudioDevice.txt";
    userProfileLastFilePath = userProfileLastFilePath + L"\\SteamSwitch\\LastBPAudioDevice.txt";
    HANDLE hFile = CreateFileW(userProfileFilePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        CHAR buffer[MAX_PATH] = { 0 };
        int size = WideCharToMultiByte(CP_UTF8, 0, BPaudioDeviceName.c_str(), -1, buffer, MAX_PATH, nullptr, nullptr);
        if (size > 0 && size < MAX_PATH)
        {
            WriteFile(hFile, buffer, size - 1, &bytesWritten, nullptr);
        }
        CloseHandle(hFile);
    }
    hFile = CreateFileW(userProfileLastFilePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        CHAR buffer[MAX_PATH] = { 0 };
        int size = WideCharToMultiByte(CP_UTF8, 0, lastBPaudioDeviceName.c_str(), -1, buffer, MAX_PATH, nullptr, nullptr);
        if (size > 0 && size < MAX_PATH)
        {
            WriteFile(hFile, buffer, size - 1, &bytesWritten, nullptr);
        }
        CloseHandle(hFile);
	}
}
HRESULT AudioHandler::SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{
	HRESULT hr = E_FAIL;
    ERole reserved = eConsole;
    if (pPolicyConfig)
    {
        hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
    }
    return hr;
}
void AudioHandler::InitDefaultAudioDevice()
{

        if (pEnum)
        {
            //Determine if it is the default audio device
            bool bExit = false;
            IMMDevice* pDefDevice = nullptr;
            HRESULT hr = pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefDevice);
            if (SUCCEEDED(hr))
            {
                IPropertyStore* pStore;
                hr = pDefDevice->OpenPropertyStore(STGM_READ, &pStore);
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT friendlyName;
                    PropVariantInit(&friendlyName);
                    hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
                    if (SUCCEEDED(hr))
                    {
                        std::wstring strTmp = friendlyName.pwszVal;
                        if (strTmp == BPaudioDeviceName)
                        {
                            bExit = true;
                        }
                        PropVariantClear(&friendlyName);
                    }
                    pStore->Release();
                }
                pDefDevice->Release();
            }
            if (bExit)
            {
                return;
            }

            IMMDeviceCollection* pDevices;
            // Enumerate the output devices.
            hr = pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
            if (SUCCEEDED(hr))
            {
                UINT count = 0;
                pDevices->GetCount(&count);
                if (SUCCEEDED(hr))
                {
                    for (UINT i = 0; i < count; i++)
                    {
                        bool bFind = false;
                        IMMDevice* pDevice = nullptr;
                        hr = pDevices->Item(i, &pDevice);
                        if (SUCCEEDED(hr))
                        {
                            LPWSTR wstrID = nullptr;
                            hr = pDevice->GetId(&wstrID);
                            if (SUCCEEDED(hr))
                            {
                                IPropertyStore* pStore = nullptr;
                                hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
                                if (SUCCEEDED(hr))
                                {
                                    PROPVARIANT friendlyName;
                                    PropVariantInit(&friendlyName);
                                    hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
                                    if (SUCCEEDED(hr))
                                    {
                                        // if no options, print the device
                                        // otherwise, find the selected device and set it to be default
                                        std::wstring strTmp = friendlyName.pwszVal;
                                        if (strTmp == BPaudioDeviceName)
                                        {
                                            SetDefaultAudioPlaybackDevice(wstrID);
                                            bFind = true;
                                        }
                                        PropVariantClear(&friendlyName);
                                    }
                                    pStore->Release();
                                }
                            }
                            pDevice->Release();
                        }

                        if (bFind)
                        {
                            break;
                        }
                    }
                }
                pDevices->Release();
            }
        }
}
void AudioHandler::setDefaultAudioDevice(LPCWSTR newDevice)
{
    if (newDevice)
    {
        BPaudioDeviceName = newDevice;
    }
}
LPCWSTR AudioHandler::getDefaultAudioDevice()
{
	return BPaudioDeviceName.c_str();
}
bool AudioHandler::BPisDefaultAudioDevice()
{
	IMMDeviceEnumerator* pEnum = nullptr;
	// Create a multimedia device enumerator.
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (SUCCEEDED(hr))
    {
        //Determine if it is the default audio device
        bool bExit = false;
        IMMDevice* pDefDevice = nullptr;
        hr = pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefDevice);
        if (SUCCEEDED(hr))
        {
            IPropertyStore* pStore;
            hr = pDefDevice->OpenPropertyStore(STGM_READ, &pStore);
            if (SUCCEEDED(hr))
            {
                PROPVARIANT friendlyName;
                PropVariantInit(&friendlyName);
                hr = pStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
                if (SUCCEEDED(hr))
                {
                    std::wstring strTmp = friendlyName.pwszVal;
                    if (strTmp == BPaudioDeviceName)
                    {
                        return true;
                    }
                    PropVariantClear(&friendlyName);
                }
                pStore->Release();
            }
            pDefDevice->Release();
        }
    }
    return false;
}