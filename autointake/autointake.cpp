// autointake.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

unsigned int game_pointer_memory;
unsigned int game_memory_offset;
unsigned int game_pointer_memory_offset;
unsigned int game_pointer_jmpback;
unsigned int game_pointer_prejumpback;
unsigned int game_function_calc_cap;
unsigned int game_function_calc_holding;

int * prisoner_capacities = new int[8];
int * prisoner_holding = new int[8];
int max_cap = 0;

int intake_minsec = 0;
int intake_medsec = 0;
int intake_maxsec = 0;

int * p_intake_midsec = (int*)p_intake_midsec;

void steam_check() {

	HANDLE hProcess = GetCurrentProcess();
	DWORD steamcheck_pointer = 0x592154;
	DWORD steamcheck;
	DWORD hook_address;

	ReadProcessMemory(hProcess, (void*)steamcheck_pointer, &steamcheck, sizeof(steamcheck), 0);

	// Stand alone
	if (steamcheck == 0x0A84C181) {
		game_pointer_memory = 0x917044;
		game_memory_offset = 0x120;
		game_function_calc_cap = 0x0613080;
		game_function_calc_holding = 0x0611E90;
		hook_address = 0x7C4D40;
		game_pointer_jmpback = 0x07C4D4A;
		game_pointer_prejumpback = (int)&standalone_restore;
	}
	// Steam
	else {
		game_pointer_memory = 0x944784;
		game_memory_offset = 0x138;
		game_pointer_jmpback = 0x07C4D4A;
		game_function_calc_cap = 0x062C830;
		game_function_calc_holding = 0x062B640;
		hook_address = 0x7EB700;
		game_pointer_jmpback = 0x7EB70A;
		game_pointer_prejumpback = (int)&steam_restore;
	}
	
	DWORD dwOldProtectionValue, dwNewProtectionValue, jump_call_dest;
	VirtualProtect((LPDWORD)hook_address, 10, PAGE_EXECUTE_READWRITE, &dwOldProtectionValue);
	jump_call_dest = (DWORD)&hook_render - hook_address - 5;
	memcpy((LPVOID)hook_address, "\xE9", sizeof(char));
	memcpy((LPVOID)(hook_address + 1), &jump_call_dest, sizeof(LPVOID));
	memcpy((LPVOID)(hook_address + 1 + sizeof(LPVOID)), "\x90\x90\x90\x90\x90", 5);
	VirtualProtect((LPDWORD)hook_address, 10, dwOldProtectionValue, &dwNewProtectionValue);
}

void calculate_new_intake() {
	intake_maxsec = prisoner_capacities[3] - prisoner_holding[2];
	intake_maxsec = intake_maxsec < 0 ? 0 : intake_maxsec;

	intake_medsec = prisoner_capacities[2] - prisoner_holding[1];
	intake_medsec = intake_medsec < 0 ? 0 : intake_medsec;

	intake_minsec = prisoner_capacities[1] - prisoner_holding[0];
	intake_minsec = intake_minsec < 0 ? 0 : intake_minsec;

	int total_prisoners = prisoner_holding[2] + prisoner_holding[1] + prisoner_holding[0];
	int total_room = prisoner_capacities[3] + prisoner_capacities[2] + prisoner_capacities[1];
	int to_many_prisoners = total_prisoners - total_room;
	int extra_shared_room = prisoner_capacities[0] - to_many_prisoners - intake_minsec - intake_medsec - intake_maxsec;

	if (extra_shared_room > 0) {
		intake_minsec = intake_minsec + extra_shared_room;
	}
}

void set_new_intake() {
	__asm
	{
		mov eax, dword ptr[game_pointer_memory_offset]		
		mov ecx, dword ptr[eax + 0x10D0]
		mov dword ptr[max_cap], ecx
		mov eax, dword ptr[eax + 0xA84]
		add eax, 0x10
		mov ecx, dword ptr[intake_minsec]
		mov dword ptr[eax], ecx
		add eax, 0x1C
		mov ecx, dword ptr[intake_medsec]
		mov dword ptr[eax], ecx
		add eax, 0x1C
		mov ecx, dword ptr[intake_maxsec]
		mov dword ptr[eax], ecx
	}
}

void calculate_capacities() {
	__asm
	{
		push prisoner_capacities
		mov ecx, dword ptr[game_pointer_memory_offset]
		call[game_function_calc_cap]
	}
}

void calculate_holding() {
	__asm
	{
		push prisoner_holding
		mov ecx, dword ptr[game_pointer_memory_offset]
		call[game_function_calc_holding]
	}
}

_declspec(naked) void standalone_restore() {
	__asm
	{
		mov dword ptr[ebp - 0x4B0], 0x089B544
		jmp[game_pointer_jmpback]
	}
}

_declspec(naked) void steam_restore() {
	__asm
	{		
		mov dword ptr[ebp - 0x4B0], 0x8C70FC
		jmp[game_pointer_jmpback]
	}
}

_declspec(naked) void hook_render() {
	__asm 
	{
		pushad
		mov eax, [game_pointer_memory]
		mov eax, dword ptr[eax]
		mov ecx, dword ptr[game_memory_offset]
		mov eax, dword ptr[eax + ecx]
		mov dword ptr[game_pointer_memory_offset], eax
		call[calculate_capacities]
		call[calculate_holding]
		call[calculate_new_intake]
		call[set_new_intake]
		popad
		jmp[game_pointer_prejumpback]
	}
}