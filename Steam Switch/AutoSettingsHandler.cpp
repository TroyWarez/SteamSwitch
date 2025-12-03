#include "AutoSettingsHandler.h"
AutoSettingsHandler::AutoSettingsHandler()
{
	//HANDLE gameSettingsEvent = CreateEventW(nullptr, FALSE, FALSE, L"GameSettingsEvent");
	std::array<WCHAR, MAX_PATH>  programFileAutoSettingsPath= { L'\0' };
	ExpandEnvironmentStringsW(settingsSearchFolderPathEnv, programFileAutoSettingsPath.data(), MAX_PATH);
	settingsSearchPath = programFileAutoSettingsPath.data();
	WIN32_FIND_DATAW ffd = { 0 };
	HANDLE hFind = FindFirstFileW(settingsSearchPath.c_str(), &ffd);
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
				foundFilePath = settingsSearchPath.substr(0, settingsSearchPath.size() - 1); // Remove the '*' at the end
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
							HANDLE hTestAutoSettingsFile = CreateFileW(programFileAutoSettingsFilePathStr.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
							if (hTestAutoSettingsFile != INVALID_HANDLE_VALUE)
							{
								CloseHandle(hTestAutoSettingsFile);
								autoSettingsPaths.emplace_back(std::wstring(programFileAutoSettingsFilePath.data()));
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
}
AutoSettingsHandler::~AutoSettingsHandler()
{
	settingsSearchPath = L"";
}
void AutoSettingsHandler::SetAllBPModeSettings()
{

	for (size_t i = 0; i < autoSettingsPaths.size(); i++)
	{
		HANDLE hAutoSettingsFile = CreateFileW(autoSettingsPaths[i].c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hAutoSettingsFile != INVALID_HANDLE_VALUE)
		{
			DWORD fileSize = GetFileSize(hAutoSettingsFile, nullptr);
			BOOL ret = MoveFileExW(L"BP.txt", autoSettingsPaths[i].c_str(), MOVEFILE_REPLACE_EXISTING);
			CloseHandle(hAutoSettingsFile);
		}
	}
}
void AutoSettingsHandler::SetAllDESKModeSettings()
{

	for (size_t i = 0; i < autoSettingsPaths.size(); i++)
	{
		HANDLE hAutoSettingsFile = CreateFileW(autoSettingsPaths[i].c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hAutoSettingsFile != INVALID_HANDLE_VALUE)
		{
			DWORD fileSize = GetFileSize(hAutoSettingsFile, nullptr);
			BOOL ret = MoveFileExW(autoSettingsPaths[i].c_str(), L"DESK.txt", MOVEFILE_REPLACE_EXISTING);
			CloseHandle(hAutoSettingsFile);
		}
	}
}