#pragma once
#include <stdio.h>

#ifdef _DEBUG
#define DEBUG_PRINTF(fmt, ...) \
    printf("[DEBUG] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif
