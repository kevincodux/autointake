// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"



extern "C" BOOL APIENTRY DllMain(HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	DWORD hook_address = 0x7C4D40;
	char *strUnPatch = "\xC7\x85\x50\xFB\xFF\xFF\x44\xB5\x89\x00";
	HANDLE hProcess = GetCurrentProcess();

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		DWORD dwOldProtectionValue, dwNewProtectionValue, jump_call_dest;

		VirtualProtect((LPDWORD)hook_address, 10, PAGE_EXECUTE_READWRITE, &dwOldProtectionValue);

		jump_call_dest = (DWORD)&hook_render - hook_address - 5;

		memcpy((LPVOID)hook_address, "\xE9", sizeof(char));
		memcpy((LPVOID)(hook_address + 1), &jump_call_dest, sizeof(LPVOID));
		memcpy((LPVOID)(hook_address + 1 + sizeof(LPVOID)), "\x90\x90\x90\x90\x90", 5);

		VirtualProtect((LPDWORD)hook_address, 10, dwOldProtectionValue, &dwNewProtectionValue);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

