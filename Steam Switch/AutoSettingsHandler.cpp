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
	std::wstring foundFilePathTextFile;
	std::vector<CHAR> testPathBuffer(MAX_PATH, '\0');
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			foundDirectoryName = ffd.cFileName;
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && foundDirectoryName != L"." && foundDirectoryName != L"..")
			{
				foundFilePath = BPsettingsSearchPath.substr(0, BPsettingsSearchPath.size() - 1); // Remove the '*' at the end
				foundFilePathTextFile = foundFilePath + foundDirectoryName + L"\\path.txt";
				HANDLE hTestDir = CreateFileW(foundFilePathTextFile.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
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
								BPFolderSettingsPaths.emplace_back(std::wstring(foundFilePath + foundDirectoryName));
							}
						}
					}
					CloseHandle(hTestDir);
				}
				foundFilePathTextFile = L"";
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
				foundFilePathTextFile = foundFilePath + foundDirectoryName + L"\\path.txt";
				HANDLE hTestDir = CreateFileW(foundFilePathTextFile.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
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
								DESKFolderSettingsPaths.emplace_back(std::wstring(foundFilePath + foundDirectoryName));
							}
						}
					}
					CloseHandle(hTestDir);
				}
				foundFilePathTextFile = L"";
			}
		} while (FindNextFileW(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	DWORD ret = SetAllBPModeSettings();
	ret = 2;
}
AutoSettingsHandler::~AutoSettingsHandler()
{
	BPsettingsSearchPath = L"";
	DESKsettingsSearchPath = L"";
	BPautoSettingsPaths = std::vector<std::wstring>();
	DESKautoSettingsPaths = std::vector<std::wstring>();
}
DWORD AutoSettingsHandler::SetAllBPModeSettings()
{
	return ApplySettingsFromFolder(TRUE);
}
DWORD AutoSettingsHandler::SetAllDESKModeSettings()
{
	return ApplySettingsFromFolder(FALSE);
}
DWORD AutoSettingsHandler::ApplySettingsFromFolder(BOOL isBigPictureMode)
{
	std::vector<std::wstring> autoSettingsPaths = isBigPictureMode ? BPautoSettingsPaths : DESKautoSettingsPaths;
	std::vector<std::wstring> autoFolderSettingsPaths = isBigPictureMode ? BPFolderSettingsPaths : DESKFolderSettingsPaths;
	for (size_t i = 0; i < autoSettingsPaths.size(); i++)
	{
		HANDLE hAutoSettingsFile = CreateFileW(autoSettingsPaths[i].c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		if (hAutoSettingsFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hAutoSettingsFile);
			SHFILEOPSTRUCTW sfo = { nullptr };
			sfo.hwnd = GetDesktopWindow();
			sfo.wFunc = FO_COPY;
			sfo.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR;

			size_t last_backslash_pos = autoSettingsPaths[i].find_last_of(L'\\');

			if (last_backslash_pos != std::wstring::npos) {
				// Extract the substring after the last backslash
				// Start position is after the last backslash, length until the end
				std::wstring folderName = autoFolderSettingsPaths[i] + autoSettingsPaths[i].substr(last_backslash_pos) + L'\0';
				std::wstring newFolderName = autoSettingsPaths[i] + L'\0';
				if (SUCCEEDED(PathCchRemoveFileSpec(newFolderName.data(), newFolderName.size())))
				{
					sfo.pFrom = folderName.c_str();
					sfo.pTo = newFolderName.c_str();
					if (SHFileOperationW(&sfo))
					{
						return ERROR_FILE_NOT_FOUND;
					}
				}
			}
		}
	}
	return ERROR_SUCCESS;
}