#include "config.hpp"
#include "qt/input_manager.hpp"
#include "main.hpp"

Config::Entry ConfigEntries[] = {
    {"a_button", &nesKeyBinds[0].key, Config::Type::KBIND},
    {"b_button", &nesKeyBinds[1].key, Config::Type::KBIND},
    {"stick_up", &nesKeyBinds[2].key, Config::Type::KBIND},
    {"stick_down", &nesKeyBinds[3].key, Config::Type::KBIND},
    {"stick_left", &nesKeyBinds[4].key, Config::Type::KBIND},
    {"stick_right", &nesKeyBinds[5].key, Config::Type::KBIND},
    {"start_button", &nesKeyBinds[6].key, Config::Type::KBIND},
    {"select_button", &nesKeyBinds[7].key, Config::Type::KBIND},

    {"debug_logs", &showDebugLogs, Config::Type::BOOL},
};

void Config::Load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char name[256];
        int value;

        if (line[0] == '\n') continue;
        if (sscanf(line, "%255s %d", name, &value) != 2) continue;

        for (auto &entry : ConfigEntries) {
            if (name == entry.name) {
                if (entry.type == Type::BOOL) {
                    *(bool*)entry.ptr = value != 0; 
                } else if (entry.type == Type::INT) {
                    *(int*)entry.ptr = value; 
                } else if (entry.type == Type::KBIND) {
                    *(Qt::Key*)entry.ptr = static_cast<Qt::Key>(value);
                } else {
                    //should never happen
                    printf("Unknown Config type: %d\n", entry.type);
                    break;
                }
            }
        }
    }

    fclose(f);
    if (showDebugLogs) printf("Loaded config file from '%s'\n", path);
}

void Config::Write(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return;

    for (auto &entry : ConfigEntries) {
        if (entry.type == Type::BOOL) {
            fprintf(f, "%s %d\n", entry.name.c_str(), *(bool*)entry.ptr);
        } else if (entry.type == Type::INT) {
            fprintf(f, "%s %d\n", entry.name.c_str(), *(int*)entry.ptr);
        } else if (entry.type == Type::KBIND) {
            fprintf(f, "%s %d\n", entry.name.c_str(), *(Qt::Key*)entry.ptr);
        } else {
            //should never happen
            printf("Unknown Config type: %d\n", entry.type);
            break;
        }
    }

    fclose(f);
    if (showDebugLogs) printf("Wrote config file to '%s'\n", path);
}