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

    void Load(const char *path);
    void Write(const char *path);
};