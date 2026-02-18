#include "gui.hpp"
#include "nes_rom.hpp"

extern NesROM globalROM;
extern SDL_Surface* icon;
extern TTF_Font *gSDLFont;

std::vector<MeowGUI::AppWindow> AppWindows;

namespace MeowGUI {
    void CreateWin(const char *name, uint32_t width, uint32_t height, void (*updFunc)(SDL_Renderer *renderer)) {
        for (auto &win : AppWindows) {
            if (!strcmp(name, win.name)) {
                return;
            }
        }
        AppWindow w;
        w.window = SDL_CreateWindow(
            name,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN
        );

        if (!icon) {
            std::cout << "Failed to set window icon\n";
        } else {
            SDL_SetWindowIcon(w.window, icon);
        }

        w.renderer = SDL_CreateRenderer(
            w.window,
            -1,
            SDL_RENDERER_ACCELERATED
        );

        w.id = SDL_GetWindowID(w.window);
        w.name = name;
        w.updFunc = updFunc;

        AppWindows.push_back(w);
    }

    void ProcessSDLEvents(SDL_Event *event) {
        if (event->type == SDL_WINDOWEVENT) {
            if (event->window.event == SDL_WINDOWEVENT_CLOSE) {
                uint32_t closeID = event->window.windowID;
                for (auto win = AppWindows.begin(); win != AppWindows.end(); win++) {
                    if (win->id == closeID) {
                        SDL_DestroyRenderer(win->renderer);
                        SDL_DestroyWindow(win->window);
                        AppWindows.erase(win);
                        break;
                    }
                }
            }
        }
    }

    void ProcessWindows(void) {
        for (auto& w : AppWindows) {
            SDL_SetRenderDrawColor(w.renderer, 0x20, 0x20, 0x20, 0xff);
            SDL_RenderClear(w.renderer);
            if (w.updFunc) w.updFunc(w.renderer);

            SDL_RenderPresent(w.renderer);
        }
    }

    void DrawText(SDL_Renderer* r, int x, int y, SDL_Color color, std::string text, int *incY) {
        SDL_Surface* surf = TTF_RenderText_Blended(gSDLFont, text.c_str(), color);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_Rect dst = { x, incY ? *incY : y, surf->w, surf->h };
        SDL_RenderCopy(r, tex, NULL, &dst);

        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
        if (incY) *incY+=16;
    }
} // namespace MeowGUI

void UpdRomInfoWin(SDL_Renderer *r) {
    int yOff = 6;
    int xOff = 6;
    SDL_Color whiteCol =  {0xff, 0xff, 0xff, 0xff};
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "File: " + globalROM.Name, &yOff);
    yOff += 5;

    std::string HeaderHex;
    for (int i = 0; i < 8; i++) {
        char Buf[4];
        snprintf(Buf, sizeof(Buf), "%02X", globalROM.Header[i]);
        HeaderHex += Buf;
        if (i < 7) HeaderHex += " ";
    }
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "Header: " + HeaderHex, &yOff);
    std::string headVer = globalROM.Version == HeaderVersion::NES2_0 ? "NES2.0" : "INES";
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "Header Version: " + headVer, &yOff);

    yOff += 5;

    char PRGSizeStr[64];
    sprintf(PRGSizeStr, "0x%zx (%zu)", globalROM.PRGRomSize, globalROM.PRGRomSize);
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "PRG Size: " + std::string(PRGSizeStr), &yOff);
    char CHRSizeStr[64];
    sprintf(CHRSizeStr, "0x%zx (%zu)", globalROM.CHRRomSize, globalROM.CHRRomSize);
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "CHR Size: " + std::string(CHRSizeStr), &yOff);

    yOff += 5;

    std::string mapperStr = globalROM.mapper ? globalROM.mapper->getName() : (globalROM.MapperID ? "Unknown" : "NROM");
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "Mapper: " + mapperStr + " (Mapper " + std::to_string(globalROM.MapperID) + ")", &yOff);
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "Sub Mapper: " + std::to_string(globalROM.SubMapperID), &yOff);
    std::string CHRRam = globalROM.CHRRomSize == 0 ? "Yes" : "No";
    MeowGUI::DrawText(r, xOff, 0, whiteCol, "CHR-RAM: " + CHRRam, &yOff);
}