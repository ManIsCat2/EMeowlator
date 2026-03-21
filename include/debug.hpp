#pragma once

#define DebugPrintLog(title, ...) \
    printf("[%s] ", title); \
    printf(__VA_ARGS__); \
    printf("\n");
