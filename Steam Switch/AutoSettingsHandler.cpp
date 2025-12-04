#include "AutoSettingsHandler.h"

AutoSettingsHandler::AutoSettingsHandler()
{
	//HANDLE gameSettingsEvent = CreateEventW(nullptr, FALSE, FALSE, L"GameSettingsEvent");
	std::array<WCHAR, MAX_PATH>  BPprogramFileAutoSettingsPath = { L'\0' };
	std::array<WCHAR, MAX_PATH>  DESKprogramFileAutoSettingsPath = { L'\0' };
	ExpandEnvironmentStringsW(BPsettingsSearchFolderPathEnv, BPprogramFileAutoSettingsPath.data(), MAX_PATH);
	ExpandEnvironmentStringsW(DESKsettingsSearchFolderPathEnv, DESKprogramFileAutoSettingsPath.data(), MAX_PATH);
	BPsettingsSearchPath = BPprogramFileAutoSettingsPath.data();
	DESKsettingsSearchPath = DESKprogramFileAutoSettingsPath.data();
	WIN32_FIND_DATAW ffd = { 0 };
	HANDLE hFind = FindFirstFileW(BPsettingsSearchPath.c_str(), &ffd);
	std::wstring foundDirectoryName;
	std::wstring foundFilePath;
	std::vector<CHAR> testPathBuffer(MAX_PATH, '\0');
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			foundDirectoryName = ffd.cFileName;
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && foundDirectoryName != L"." && foundDirectoryName != L"..")
			{
				foundFilePath = BPsettingsSearchPath.substr(0, BPsettingsSearchPath.size() - 1); // Remove the '*' at the end
				foundFilePath = foundFilePath + foundDirectoryName + L"\\path.txt";
				HANDLE hTestDir = CreateFileW(foundFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
				if (hTestDir != INVALID_HANDLE_VALUE)
				{
					DWORD fileSize = GetFileSize(hTestDir, nullptr);
					if (fileSize)
					{
						testPathBuffer.resize(fileSize / sizeof(WCHAR) + 1, L'\0');
					}
					DWORD bytesRead = 0;
					if (ReadFile(hTestDir, testPathBuffer.data(), static_cast<DWORD>(testPathBuffer.size() * sizeof(WCHAR)), &bytesRead, nullptr))
					{
						std::wstring tempPathBuffer(MAX_PATH, L'\0');
						if (MultiByteToWideChar(CP_UTF8, 0, testPathBuffer.data(), static_cast<int>(bytesRead), tempPathBuffer.data(), MAX_PATH))
						{
							std::array<WCHAR, MAX_PATH> programFileAutoSettingsFilePath = { L'\0' };
							ExpandEnvironmentStringsW(tempPathBuffer.c_str(), programFileAutoSettingsFilePath.data(), MAX_PATH);
							std::wstring programFileAutoSettingsFilePathStr(programFileAutoSettingsFilePath.data());
							HANDLE hTestAutoSettingsFile = CreateFileW(programFileAutoSettingsFilePathStr.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
							if (hTestAutoSettingsFile != INVALID_HANDLE_VALUE)
							{
								CloseHandle(hTestAutoSettingsFile);
								BPautoSettingsPaths.emplace_back(std::wstring(programFileAutoSettingsFilePath.data()));
							}
						}
					}
					CloseHandle(hTestDir);
				}
				foundFilePath = L"";
			}
		} while (FindNextFileW(hFind, &ffd) != 0);
		FindClose(hFind);
	}

	ffd = { 0 };
	hFind = FindFirstFileW(BPsettingsSearchPath.c_str(), &ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			foundDirectoryName = ffd.cFileName;
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && foundDirectoryName != L"." && foundDirectoryName != L"..")
			{
				foundFilePath = DESKsettingsSearchPath.substr(0, DESKsettingsSearchPath.size() - 1); // Remove the '*' at the end
				foundFilePath = foundFilePath + foundDirectoryName + L"\\path.txt";
				HANDLE hTestDir = CreateFileW(foundFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
				if (hTestDir != INVALID_HANDLE_VALUE)
				{
					DWORD fileSize = GetFileSize(hTestDir, nullptr);
					if (fileSize)
					{
						testPathBuffer.resize(fileSize / sizeof(WCHAR) + 1, L'\0');
					}
					DWORD bytesRead = 0;
					if (ReadFile(hTestDir, testPathBuffer.data(), static_cast<DWORD>(testPathBuffer.size() * sizeof(WCHAR)), &bytesRead, nullptr))
					{
						std::wstring tempPathBuffer(MAX_PATH, L'\0');
						if (MultiByteToWideChar(CP_UTF8, 0, testPathBuffer.data(), static_cast<int>(bytesRead), tempPathBuffer.data(), MAX_PATH))
						{
							std::array<WCHAR, MAX_PATH> programFileAutoSettingsFilePath = { L'\0' };
							ExpandEnvironmentStringsW(tempPathBuffer.c_str(), programFileAutoSettingsFilePath.data(), MAX_PATH);
							std::wstring programFileAutoSettingsFilePathStr(programFileAutoSettingsFilePath.data());
							HANDLE hTestAutoSettingsFile = CreateFileW(programFileAutoSettingsFilePathStr.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
							if (hTestAutoSettingsFile != INVALID_HANDLE_VALUE)
							{
								CloseHandle(hTestAutoSettingsFile);
								DESKautoSettingsPaths.emplace_back(std::wstring(programFileAutoSettingsFilePath.data()));
							}
						}
					}
					CloseHandle(hTestDir);
				}
				foundFilePath = L"";
			}
		} while (FindNextFileW(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	SetAllBPModeSettings();
}
AutoSettingsHandler::~AutoSettingsHandler()
{
	BPsettingsSearchPath = L"";
	DESKsettingsSearchPath = L"";
	BPautoSettingsPaths = std::vector<std::wstring>();
	DESKautoSettingsPaths = std::vector<std::wstring>();
}
void AutoSettingsHandler::SetAllBPModeSettings()
{
	return ApplySettingsFromFolder(TRUE);
}
void AutoSettingsHandler::SetAllDESKModeSettings()
{
	return ApplySettingsFromFolder(FALSE);
}
void AutoSettingsHandler::ApplySettingsFromFolder(BOOL isBigPictureMode)
{
	std::vector<std::wstring> autoSettingsPaths = isBigPictureMode ? BPautoSettingsPaths : DESKautoSettingsPaths;
	for (size_t i = 0; i < BPautoSettingsPaths.size(); i++)
	{
		HANDLE hAutoSettingsFile = CreateFileW(autoSettingsPaths[i].c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		if (hAutoSettingsFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hAutoSettingsFile);
			SHFILEOPSTRUCTW sfo = { nullptr };
			sfo.hwnd = GetDesktopWindow();
			sfo.wFunc = FO_MOVE;
			sfo.pFrom = autoSettingsPaths[i].c_str();
			sfo.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;

			std::wstring settingsSearchPath(autoSettingsPaths[i]);
			settingsSearchPath = settingsSearchPath + L"\\";
			if (SUCCEEDED(PathCchRemoveFileSpec(settingsSearchPath.data(), settingsSearchPath.size())))
			{
				sfo.pTo = settingsSearchPath.c_str();
				int ret = SHFileOperationW(&sfo);
				ret = GetLastError();
				ret = 2;
			}
		}
	}
}