#include "AudioHandler.h"
using namespace std;

AudioHandler::AudioHandler()
{
	WCHAR programFiles[MAX_PATH] = { 0 };
	ExpandEnvironmentStringsW(L"%userProfile%", programFiles, MAX_PATH);
	std::wstring programFilesPath(programFiles);
    std::wstring programFilesDeskPath(programFiles);
	programFilesPath = programFilesPath + L"\\SteamSwitch\\BPAudioDevice.txt";
    programFilesDeskPath = programFilesDeskPath + L"\\SteamSwitch\\DESKAudioDevice.txt";
	HANDLE hFile = CreateFileW(programFilesPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD re3 = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        CHAR buffer[MAX_PATH] = { 0 };
        WCHAR wbuffer[MAX_PATH] = { 0 };
        bytesRead = GetFileSize(hFile, NULL);
        if (bytesRead > 0 && bytesRead < MAX_PATH)
        {
            if (ReadFile(hFile, buffer, bytesRead, &bytesRead, NULL))
            {
				MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, wbuffer, MAX_PATH);
                BPaudioDeviceName = wbuffer;
            }
		}
        CloseHandle(hFile);
    }
    else
    {
		MessageBoxW(NULL, (L"Could not default audio file: " + programFilesPath + L"\nError code: " + std::to_wstring(re3)).c_str(), L"Error", MB_ICONERROR);
		PostQuitMessage(0);
    }

	hFile = CreateFileW(programFilesDeskPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD bytesRead;
		CHAR buffer[MAX_PATH] = { 0 };
		WCHAR wbuffer[MAX_PATH] = { 0 };
		bytesRead = GetFileSize(hFile, NULL);
		if (bytesRead > 0 && bytesRead < MAX_PATH)
		{
			if (ReadFile(hFile, buffer, bytesRead, &bytesRead, NULL))
			{
				MultiByteToWideChar(CP_UTF8, 0, buffer, bytesRead, wbuffer, MAX_PATH);
				DESKaudioDeviceName = wbuffer;
			}
		}
		CloseHandle(hFile);
	}
}
AudioHandler::~AudioHandler()
{}
HRESULT SetDefaultAudioPlaybackDevice(LPCWSTR devID)
{
    IPolicyConfigVista* pPolicyConfig;
    ERole reserved = eConsole;

    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient),
        NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID*)&pPolicyConfig);
    if (SUCCEEDED(hr))
    {
        hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
        pPolicyConfig->Release();
    }
    return hr;
}
void AudioHandler::InitDefaultAudioDevice()
{
        IMMDeviceEnumerator* pEnum = NULL;
        // Create a multimedia device enumerator.
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
            CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
        if (SUCCEEDED(hr))
        {
            //Determine if it is the default audio device
            bool bExit = false;
            IMMDevice* pDefDevice = NULL;
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
                pEnum->Release();
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
                        IMMDevice* pDevice;
                        hr = pDevices->Item(i, &pDevice);
                        if (SUCCEEDED(hr))
                        {
                            LPWSTR wstrID = NULL;
                            hr = pDevice->GetId(&wstrID);
                            if (SUCCEEDED(hr))
                            {
                                IPropertyStore* pStore;
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
            pEnum->Release();
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
	IMMDeviceEnumerator* pEnum = NULL;
	// Create a multimedia device enumerator.
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
		CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnum);
    if (SUCCEEDED(hr))
    {
        //Determine if it is the default audio device
        bool bExit = false;
        IMMDevice* pDefDevice = NULL;
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