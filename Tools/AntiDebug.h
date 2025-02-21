#pragma once
#include <TlHelp32.h>

#define ARRAY_SIZE 30
#define FLG_HEAP_ENABLE_TAIL_CHECK   0x10
#define FLG_HEAP_ENABLE_FREE_CHECK   0x20
#define FLG_HEAP_VALIDATE_PARAMETERS 0x40

WCHAR* g_MyList[ARRAY_SIZE] = {
		L"x64dbg.exe",
		L"x32dbg.exe",
		L"x96dbg.exe",
		L"ida.exe",
		L"ida64.exe",
		L"VsDebugConsole.exe",
		L"msvsmon.exe",
		L"ollydbg.exe",
		L"windbg.exe",
		L"ImmunityDebugger.exe",
		L"ProcessHacker.exe",
		L"procmon.exe",
		L"procexp.exe",
		L"idaq.exe",
		L"idaq64.exe",
		L"HTTPDebuggerSvc.exe",
		L"scylla_x64.exe",
		L"scylla_x86.exe",
		L"HTTPDebugger.exe",
		L"ProcessExplorer.exe",
		L"ghidraRun.exe",
		L"dnSpy-x86.exe",
		L"SystemInformer.exe",
		L"dnSpy.exe",
		L"PE-bear.exe",
		L"pestudio.exe",
		L"dllexp.exe",
		L"DbgX.Shell.exe",
		L"ghidra.exe",
		L"idapro.exe"
};

// ProcessCheck function
BOOL ProcessCheck() {

	HANDLE				hSnapShot = NULL;
	PROCESSENTRY32W		ProcEntry = { .dwSize = sizeof(PROCESSENTRY32W) };
	BOOL				bSTATE = FALSE;


	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapShot == INVALID_HANDLE_VALUE) {
		printf("\n\t[!] CreateToolhelp32Snapshot Failed With Error : %d \n", GetLastError());
		goto _EndOfFunction;
	}

	if (!Process32FirstW(hSnapShot, &ProcEntry)) {
		printf("\n\t[!] Process32FirstW Failed With Error : %d \n", GetLastError());
		goto _EndOfFunction;
	}

	do {

		for (int i = 0; i < ARRAY_SIZE; i++) {
			if (wcscmp(ProcEntry.szExeFile, g_MyList[i]) == 0) {
				bSTATE = TRUE;
				break; // breaking from the for loop
			}
		}

		if (bSTATE)
			break; // breaking from the do-while loop

	} while (Process32Next(hSnapShot, &ProcEntry));


_EndOfFunction:
	if (hSnapShot != NULL)
		CloseHandle(hSnapShot);
	return bSTATE;
}

// HBPCheck function
BOOL HBPCheck() {

	CONTEXT		Ctx = { .ContextFlags = CONTEXT_DEBUG_REGISTERS };

	if (!GetThreadContext(GetCurrentThread(), &Ctx)) {
		printf("\n\t[!] GetThreadContext Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	// if one of these registers is not '0', then a hardware bp is installed
	if (Ctx.Dr0 != NULL || Ctx.Dr1 != NULL || Ctx.Dr2 != NULL || Ctx.Dr3 != NULL)
		return TRUE;

	return FALSE;
}

// PerformanceCheck function
BOOL PerformanceCheck() {

	LARGE_INTEGER	Time1 = { 0 },
		Time2 = { 0 };

	if (!QueryPerformanceCounter(&Time1)) {
		printf("\n\t[!] QueryPerformanceCounter [1] Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	if (ProcessCheck()) {
		return TRUE;
	}

	if (HBPCheck()) {
		return TRUE;
	}

	if (!QueryPerformanceCounter(&Time2)) {
		printf("\n\t[!] QueryPerformanceCounter [2] Failed With Error : %d \n", GetLastError());
		return FALSE;
	}

	if ((Time2.QuadPart - Time1.QuadPart) > 100000) {
		return TRUE;
	}

	return FALSE;
}

// TimeCheck function
BOOL TimeCheck() {

	DWORD	dwTime1 = NULL,
		dwTime2 = NULL;

	dwTime1 = GetTickCount64();

	if (ProcessCheck()) {
		return TRUE;
	}

	if (HBPCheck()) {
		return TRUE;
	}

	dwTime2 = GetTickCount64();

	if ((dwTime2 - dwTime1) > 50) {
		return TRUE;
	}

	return FALSE;
}
