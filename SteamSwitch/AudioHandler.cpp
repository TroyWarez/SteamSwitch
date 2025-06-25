#include "AudioHandler.h"
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "PolicyConfig.h"
#include "Settings.h"
#include <iostream>
using namespace std;

AudioHandler::AudioHandler()
{
    InitDefaultAudioDevice();
}
AudioHandler::~AudioHandler()
{
}
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
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        IMMDeviceEnumerator* pEnum = NULL;
        // Create a multimedia device enumerator.
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
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
                        if (strTmp == defaultBpAudioDevice)
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
                UINT count;
                pDevices->GetCount(&count);
                if (SUCCEEDED(hr))
                {
                    for (int i = 0; i < count; i++)
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
                                        if (strTmp == defaultBpAudioDevice)
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
    CoUninitialize();
}
int AudioHandler::setDefaultAudioDevice(int device)
{
	return 0;
}
int AudioHandler::getDefaultAudioDevice(int device)
{
	return 0;
}
void AudioHandler::ToggleMode()
{

}