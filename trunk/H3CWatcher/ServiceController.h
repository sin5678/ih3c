#pragma once
#include <windows.h>
#include <string>
#pragma comment(lib, "advapi32.lib")

class ServiceController
{
	BOOL RunProcess(LPTSTR szCmdLine, LPDWORD lpExitCode)
	{
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};

		si.cb = sizeof(si);
	 
		if( CreateProcess( NULL,   // No module name (use command line)
			szCmdLine,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi )           // Pointer to PROCESS_INFORMATION structure
		)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			if(!GetExitCodeProcess(pi.hProcess, lpExitCode))
			{
				return FALSE;
			}
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

public:
	~ServiceController()
	{
	}

	bool RestartService(const wstring& szSvcName)
	{
		using namespace std;
		wstring stopSvcCmd = L"net stop \""+szSvcName+L"\"";
		wstring startSvcCmd = L"net start \""+szSvcName+L"\"";
		DWORD stopExitCode, startExitCode;
		RunProcess(const_cast<wchar_t*>(stopSvcCmd.c_str()), &stopExitCode);
		RunProcess(const_cast<wchar_t*>(startSvcCmd.c_str()), &startExitCode);
		return startExitCode==0;
	}
};