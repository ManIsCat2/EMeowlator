#pragma once

#include <stdio.h>
#include <string>

class Config {
public:
    enum class Type {
        BOOL,
        INT,
        KBIND,
    };
    struct Entry {
        std::string name;
        void *ptr;
        Type type;
    };

    static void Load(const char *path);
    static void Write(const char *path);
};