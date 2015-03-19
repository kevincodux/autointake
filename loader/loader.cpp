// loader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define MAXWAIT 10000  

using namespace std;
DWORD pid;
HWND hwnd;

BOOL inject(std::string dll)
{
	HMODULE hLocKernel32 = GetModuleHandle(L"Kernel32");
	FARPROC hLocLoadLibrary = GetProcAddress(hLocKernel32, "LoadLibraryA");
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, 0, &tkp, sizeof(tkp), NULL, NULL);
	}
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	dll += '\0';
	LPVOID hRemoteMem = VirtualAllocEx(hProc, NULL, dll.size(), MEM_COMMIT, PAGE_READWRITE);
	DWORD numBytesWritten;
	WriteProcessMemory(hProc, hRemoteMem, dll.c_str(), dll.size(), &numBytesWritten);
	HANDLE hRemoteThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)hLocLoadLibrary, hRemoteMem, 0, NULL);
	cout << hRemoteThread << endl;
	bool res = false;
	if (hRemoteThread)
		res = (bool)WaitForSingleObject(hRemoteThread, MAXWAIT) != WAIT_TIMEOUT;
	VirtualFreeEx(hProc, hRemoteMem, dll.size(), MEM_RELEASE);
	CloseHandle(hProc);

	return res;
}

int _tmain(int argc, _TCHAR* argv[])
{
	hwnd = FindWindow(NULL, L"Prison Architect"); //Finds Prison Architect

	if (!hwnd)
	{
		cout << "Prison Architect not found!\n";
		system("PAUSE");
		return EXIT_SUCCESS;
	}

	GetWindowThreadProcessId(hwnd, &pid);

	if (!inject("autointake.dll")){
		cout << "Could not load AutoIntake\n";
	}
	else{
		cout << "AutoIntake loaded!\n";
	}

	system("PAUSE");

	return 0;
}

