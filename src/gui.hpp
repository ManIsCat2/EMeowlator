#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace MeowGUI {
    struct AppWindow {
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        uint32_t id = 0;
        const char *name;
        void (*updFunc)(SDL_Renderer *renderer);
    };

    void CreateWin(const char *name, uint32_t width, uint32_t height, void (*updFunc)(SDL_Renderer *renderer)=nullptr);
    void ProcessSDLEvents(SDL_Event *event);
    void ProcessWindows(void);
    void DrawText(SDL_Renderer* r, int x, int y, SDL_Color color, std::string text, int *incY=nullptr);
} // namespace MeowGUI

extern void UpdRomInfoWin(SDL_Renderer *r);