#define PFD_USING_IMPLEMENTATION
#include "portable-file-dialogs.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"

#include "main.hpp"
#include "gui.hpp"

#include "savestate.hpp"

NesROM globalROM;

bool romIsLoaded = false;
bool showDebugLogs = false;
static bool fullscreen = false;
static float CPUSpeed = 1.f;
static bool unlimitFPS = false;

static const char* NesPalettes[] = { "NTSC", "PAL" };

SDL_Surface *icon = IMG_Load("gui/ico.png");
TTF_Font *gSDLFont = nullptr;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("MeowNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, NES_WIDTH*3, NES_HEIGHT*3, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!icon) {
        std::cout << "Failed to set window icon\n";
    } else {
       SDL_SetWindowIcon(window, icon);
    }

    if (!ppu.InitSDL(renderer)) return 1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    gSDLFont = TTF_OpenFont("gui/ProggyClean.ttf", 16);
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    bool running = true;
    std::string romPath;
    SDL_Event event;

    if (argc > 1) {
        if (globalROM.LoadNES(argv[1])) {
            cpu.reset();
            romIsLoaded = true;
        }
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            MeowGUI::ProcessSDLEvents(&event);
            if (event.type == SDL_QUIT) running = false;
        }
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open ROM")) {
                    auto selection = pfd::open_file(
                        "Select NES ROM",
                        "",
                        { "NES ROMs", "*.nes" },
                        pfd::opt::none
                    ).result();

                    if (!selection.empty()) {
                        romPath = selection.front();
                        if (globalROM.LoadNES(romPath)) {
                            cpu.reset();
                            romIsLoaded = true;
                        } else {
                            std::cerr << "Failed to load ROM: " << romPath << "\n";
                        }
                    }
                }

                if (romIsLoaded) {
                    if (ImGui::MenuItem("Close ROM")) {
                        romIsLoaded = false;
                        cpu.reset();
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("CPU")) {
                if (ImGui::MenuItem("Pause")) {
                    cpu.CPUPaused = true;
                }
                if (ImGui::MenuItem("Continue")) {
                    cpu.CPUPaused = false;
                }
                ImGui::SetNextItemWidth(120.f);
                ImGui::SliderFloat("Speed", &CPUSpeed, 0, 16, "%.3f", ImGuiSliderFlags_ClampOnInput);
                if (ImGui::MenuItem("Reset")) {
                    cpu.reset();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("PPU")) {
                if (ImGui::BeginMenu("Palette")) {
                    ImGui::Text("Palette Editor");
                    for (int i = 0; i < 64; i++) {
                        ImGui::PushID(i);
                        uint32_t C = nesPalette[i];
                        uint8_t A = (C >> 24) & 0xFF;
                        uint8_t R = (C >> 16) & 0xFF;
                        uint8_t G = (C >>  8) & 0xFF;
                        uint8_t B = (C >>  0) & 0xFF;
                        float Col[4] = {
                            R / 255.0f,
                            G / 255.0f,
                            B / 255.0f,
                            A / 255.0f
                        };
                        if (ImGui::ColorEdit4("##pal", Col,
                            ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_DisplayRGB)) {
                            uint8_t NewR = (uint8_t)(Col[0] * 255.0f);
                            uint8_t NewG = (uint8_t)(Col[1] * 255.0f);
                            uint8_t NewB = (uint8_t)(Col[2] * 255.0f);
                            uint8_t NewA = (uint8_t)(Col[3] * 255.0f);
                            nesPalette[i] =
                                (NewA << 24) |
                                (NewR << 16) |
                                (NewG << 8)  |
                                (NewB << 0);
                        }

                        ImGui::PopID();
                        // 8 colomn
                        if ((i % 8) != 7) {
                            ImGui::SameLine();
                        }
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Reset")) {
                        memcpy(nesPalette, nesPaletteDefault, sizeof(uint32_t)*64);
                    }
                    ImGui::EndMenu();
                }
                ImGui::Checkbox("VRAM Corruption", &ppu.VRAMCorruption);
                ImGui::Checkbox("Disable X Scroll", &ppu.DisableXScroll);
                ImGui::Checkbox("Disable Y Scroll", &ppu.DisableYScroll);
                ImGui::Checkbox("Disable Sprites", &ppu.DisableSprites);
                ImGui::SetNextItemWidth(70.f);
                ImGui::InputInt("Max Sprites", &ppu.MaxSprites);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Savestate")) {
                if (ImGui::MenuItem("Save to file")) {
                    auto save = pfd::save_file(
                        "Save state",
                        "savestate.nya",
                        { "Savestate Files (.nya)", "*.nya",
                        "All Files", "*" }
                    );
                    std::string path = save.result();
                    if (!path.empty()) {
                        SaveStateFile file;
                        file.WriteSaveStateToFile(path.c_str());
                    }
                }
                if (ImGui::MenuItem("Load from file")) {
                    auto open = pfd::open_file(
                        "Load state",
                        "",
                        { "Savestate Files (.nya)", "*.nya",
                        "All Files", "*" }
                    );
                    auto results = open.result();
                    if (!results.empty()) {
                        SaveStateFile file;
                        file.LoadSaveStateFromFile(results[0].c_str());
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("ROM Info")) {
                    MeowGUI::CreateWin("ROM Info", 290, 200, UpdRomInfoWin);
                }
                ImGui::Checkbox("Show Debug Logs", &showDebugLogs);
                ImGui::EndMenu();
            }


            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::BeginMenu("Graphics")) {
                    if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
                        if (fullscreen) SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        else SDL_SetWindowFullscreen(window, 0);
                    }
                    ImGui::Checkbox("Unlimited FPS", &unlimitFPS);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Misc")) {
                if (ImGui::MenuItem("Exit")) {
                    running = false;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        MeowGUI::ProcessWindows();

        SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xff);
        SDL_RenderClear(renderer);

        if (romIsLoaded) {
            UpdateControllers();
            ppu.Render(renderer);
            cpu.run((uint32_t)(89342*CPUSpeed));
        }

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        if (!unlimitFPS) {
            SDL_Delay(16);
        }
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    ppu.ShutdownSDL();
    SDL_FreeSurface(icon);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
