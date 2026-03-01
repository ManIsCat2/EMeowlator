#pragma once

#define DebugPrintLog(ctx, ...) \
    printf("[%s] ", ctx); \
    printf(__VA_ARGS__); \
    printf("\n");
