#pragma once

#define UNUSED __attribute__((unused))

#define DebugPrintLog(title, ...) \
    printf("[%s] ", title); \
    printf(__VA_ARGS__); \
    printf("\n");
