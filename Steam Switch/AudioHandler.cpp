#include "AudioHandler.h"
using namespace std;

AudioHandler::AudioHandler()
{
	HANDLE hFile = CreateFileW(L"AudioDevice.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD er = GetLastError();
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
				audioDeviceName = wbuffer;
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
                        if (strTmp == audioDeviceName)
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
                                        if (strTmp == audioDeviceName)
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
        audioDeviceName = newDevice;
    }
}
LPCWSTR AudioHandler::getDefaultAudioDevice()
{
	return audioDeviceName.c_str();
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
                    if (strTmp == audioDeviceName)
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