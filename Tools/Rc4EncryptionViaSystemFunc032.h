#pragma once

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

    // Load the DLL and get the function address
    HMODULE hAdvapi32 = LoadLibraryA("Advapi32");
    if (!hAdvapi32) {
        printf("[!] Failed to load Advapi32.dll. Error: %d\n", GetLastError());
        return FALSE;
    }

    // Get the procedure address
    auto SystemFunction032 = reinterpret_cast<fnSystemFunction032>(
        GetProcAddress(hAdvapi32, "SystemFunction032")
        );

    if (!SystemFunction032) {
        printf("[!] Failed to get SystemFunction032 address. Error: %d\n", GetLastError());
        FreeLibrary(hAdvapi32);
        return FALSE;
    }

    // Call SystemFunction032 and check result
    STATUS = SystemFunction032(&Img, &Key);

    // Free the library
    FreeLibrary(hAdvapi32);

    if (STATUS != 0x0) {
        printf("[!] SystemFunction032 FAILED With Error: 0x%0.8X\n", STATUS);
        return FALSE;
    }

    return TRUE;
}
