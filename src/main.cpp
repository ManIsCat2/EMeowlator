#include "nes_cpu.hpp"
#include "nes_rom.hpp"

#include "main.hpp"
#include "qt/screen_widget.hpp"
#include "qt/input_manager.hpp"
#include "qt/palette_editor.hpp"
#include "savestate.hpp"

NesROM globalROM;

bool romIsLoaded = false;
bool showDebugLogs = false;
static bool fullscreen = false;
static float CPUSpeed = 1.f;
static bool unlimitFPS = false;
int hoveredPaletteIndex = -1;

void *globalQTWin;

std::string joinLines(std::vector<std::string> lines) {
    std::string result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result += lines[i];
        if (i != lines.size() - 1)
            result += "\n";
    }
    return result;
}

QAction *makeQBool(const QString &text, QObject *parent, bool defaultBool) {
    QAction *newAction = new QAction(text, parent);
    newAction->setCheckable(true);
    newAction->setChecked(defaultBool);
    return newAction;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    globalQTWin = (void*)&window;

    QMenuBar *menuBar = window.menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QMenu *CPUMenu = menuBar->addMenu("CPU");
    QMenu *PPUMenu = menuBar->addMenu("PPU");
    QMenu *saveStateMenu = menuBar->addMenu("Savestate");
    QMenu *debugMenu = menuBar->addMenu("Debug");
    QMenu *miscMenu = menuBar->addMenu("Misc");

    QAction *openAction = new QAction("Open ROM", &window);
    QAction *closeAction = new QAction("Close ROM", &window);
    QAction *CPUPauseAction = makeQBool("Pause", &window, cpu.CPUPaused);
    QAction *openBusAction = makeQBool("Open Bus", &window, cpu.emulateOBus);
    QAction *paletteEditorAction = new QAction("Palette Editor", &window);
    QAction *VRAMCorruptAction = makeQBool("VRAM Corruption", &window, ppu.VRAMCorruption);
    QAction *disableXScrollAction = makeQBool("Disable X Scroll", &window, ppu.DisableXScroll);
    QAction *disableYScrollAction = makeQBool("Disable Y Scroll", &window, ppu.DisableYScroll);
    QAction *disableSpritesAction = makeQBool("Disable Sprites", &window, ppu.DisableSprites);
    QAction *saveSaveStateAction = new QAction("Save to file", &window);
    QAction *loadSaveStateAction = new QAction("Load from file", &window);
    QAction *debugLogsAction = makeQBool("Show Debug Logs", &window, showDebugLogs);
    QAction *romInfoAction = new QAction("ROM Info", &window);
    QAction *exitAction = new QAction("Exit", &window);

    fileMenu->addAction(openAction);
    fileMenu->addAction(closeAction);
    CPUMenu->addAction(CPUPauseAction);
    CPUMenu->addAction(openBusAction);
    PPUMenu->addAction(paletteEditorAction);
    PPUMenu->addAction(VRAMCorruptAction);
    PPUMenu->addAction(disableXScrollAction);
    PPUMenu->addAction(disableYScrollAction);
    PPUMenu->addAction(disableSpritesAction);
    saveStateMenu->addAction(saveSaveStateAction);
    saveStateMenu->addAction(loadSaveStateAction);
    debugMenu->addAction(debugLogsAction);
    debugMenu->addAction(romInfoAction);
    miscMenu->addAction(exitAction);

    QObject::connect(exitAction, &QAction::triggered, &window, &QMainWindow::close);
    QObject::connect(openAction, &QAction::triggered, [&]() {
        QString file = QFileDialog::getOpenFileName(
            &window,
            "Open NES ROM",
            "",
            "NES ROM (*.nes)"
        );

        if (!file.isEmpty()) {
            if (globalROM.LoadNES(file.toStdString().c_str())) {
                cpu.reset();
                romIsLoaded = true;
            }
        }
    });
    QObject::connect(closeAction, &QAction::triggered, [&]() {
        if (romIsLoaded) {
            romIsLoaded = false;
            cpu.reset();
        }
    });
    QObject::connect(CPUPauseAction, &QAction::toggled, [&](bool checked) {
        cpu.CPUPaused = checked;
    });
    QObject::connect(openBusAction, &QAction::toggled, [&](bool checked) {
        cpu.emulateOBus = checked;
    });
    QObject::connect(paletteEditorAction, &QAction::triggered, [&]() {
        QDialog *dialog = new QDialog(&window);
        dialog->setWindowTitle("Palette Editor");
        dialog->setFixedSize(200, 320);

        QGridLayout *grid = new QGridLayout(dialog);

        PaletteButton* buttons[64];

        auto updateAllButtonsColor = [&]() {
            for (int i = 0; i < 64; i++) {
                QColor color(
                    (nesPalette[i] >> 16) & 0xFF,
                    (nesPalette[i] >> 8) & 0xFF,
                    nesPalette[i] & 0xFF
                );
                buttons[i]->setStyleSheet(QString("background-color: %1").arg(color.name()));
            }
        };

        for (int i = 0; i < 64; i++) {
            buttons[i] = new PaletteButton(i, dialog);

            auto updateButtonColor = [i, buttons]() {
                QColor color(
                    (nesPalette[i] >> 16) & 0xFF,
                    (nesPalette[i] >> 8) & 0xFF,
                    nesPalette[i] & 0xFF
                );
                buttons[i]->setStyleSheet(QString("background-color: %1").arg(color.name()));
            };

            updateButtonColor();

            QObject::connect(buttons[i], &QPushButton::clicked, [i, dialog, updateButtonColor]() {
                QColor current(
                    (nesPalette[i] >> 16) & 0xFF,
                    (nesPalette[i] >> 8) & 0xFF,
                    nesPalette[i] & 0xFF
                );

                QColor newColor = QColorDialog::getColor(current, dialog);

                if (newColor.isValid()) {
                    nesPalette[i] =
                        0xFF000000 | 
                        (newColor.red() << 16) |
                        (newColor.green() << 8) |
                        newColor.blue();
                    updateButtonColor();
                }
            });

            grid->addWidget(buttons[i], i / 8, i % 8);
        }

        QPushButton *resetButton = new QPushButton("Reset Palette", dialog);
        QPushButton *randomButton = new QPushButton("Randomize Palette", dialog);
        QPushButton *exportButton = new QPushButton("Export Palette", dialog);
        QPushButton *importButton = new QPushButton("Import Palette", dialog);
        grid->addWidget(resetButton, 8, 0, 1, 8);
        grid->addWidget(randomButton, 9, 0, 1, 8);
        grid->addWidget(exportButton, 10, 0, 1, 8);
        grid->addWidget(importButton, 11, 0, 1, 8);
        resetButton->setFixedHeight(25);
        randomButton->setFixedHeight(25);
        exportButton->setFixedHeight(25);
        importButton->setFixedHeight(25);
        QObject::connect(resetButton, &QPushButton::clicked, [&]() {
            memcpy(nesPalette, nesPaletteDefault, sizeof(nesPalette));
            updateAllButtonsColor();
        });
        QObject::connect(randomButton, &QPushButton::clicked, [&]() {
            for (int i = 0; i < 64; i++) {
                nesPalette[i] = rand() % 0xFFFFFFFF;
            }
            updateAllButtonsColor();
        });
        QObject::connect(exportButton, &QPushButton::clicked, [&]() {
            QString file = QFileDialog::getSaveFileName(
                dialog,
                "Export Palette",
                "",
                "MeowNES Palette (*.paw)"
            );

            if (!file.isEmpty()) {
                FILE* f = fopen(file.toStdString().c_str(), "wb");
                if (!f) return;
                fwrite(nesPalette, 1, sizeof(nesPalette), f);
                fclose(f);
                updateAllButtonsColor();
            }
        });
        QObject::connect(importButton, &QPushButton::clicked, [&]() {
            QString file = QFileDialog::getOpenFileName(
                dialog,
                "Import Palette",
                "",
                "MeowNES Palette (*.paw)"
            );

            if (!file.isEmpty()) {
                FILE* f = fopen(file.toStdString().c_str(), "rb");
                if (!f) return;
                fread(nesPalette, 1, sizeof(nesPalette), f);
                fclose(f);
                updateAllButtonsColor();
            }
        });

        dialog->setLayout(grid);
        dialog->exec();
    });
    QObject::connect(VRAMCorruptAction, &QAction::toggled, [&](bool checked) {
        ppu.VRAMCorruption = checked;
    });
    QObject::connect(disableXScrollAction, &QAction::toggled, [&](bool checked) {
        ppu.DisableXScroll = checked;
    });
    QObject::connect(disableYScrollAction, &QAction::toggled, [&](bool checked) {
        ppu.DisableYScroll = checked;
    });
    QObject::connect(disableSpritesAction, &QAction::toggled, [&](bool checked) {
        ppu.DisableSprites = checked;
    });
    QObject::connect(saveSaveStateAction, &QAction::triggered, [&]() {
        QString file = QFileDialog::getSaveFileName(
            &window,
            "Write Savestate",
            "",
            "MeowNES Savestate (*.nya)"
        );

        if (!file.isEmpty()) {
            SaveStateFile savestate;
            savestate.WriteSaveStateToFile(file.toStdString().c_str());
        }
    });
    QObject::connect(loadSaveStateAction, &QAction::triggered, [&]() {
        QString file = QFileDialog::getOpenFileName(
            &window,
            "Load Savestate",
            "",
            "MeowNES Savestate (*.nya)"
        );

        if (!file.isEmpty()) {
            SaveStateFile savestate;
            savestate.LoadSaveStateFromFile(file.toStdString().c_str());
        }
    });
    QObject::connect(debugLogsAction, &QAction::toggled, [&](bool checked) {
        showDebugLogs = checked;
    });
    QObject::connect(romInfoAction, &QAction::triggered, [&]() {
        std::string fileStr = "File: " + globalROM.Name;
        std::string HeaderHexStr;
        for (int i = 0; i < 8; i++) {
            char Buf[4];
            snprintf(Buf, sizeof(Buf), "%02X", globalROM.Header[i]);
            HeaderHexStr += Buf;
            if (i < 7) HeaderHexStr += " ";
        }
        HeaderHexStr = "Header: " + HeaderHexStr;
        std::string headVerStr = "Header Version: " + std::string(globalROM.Version == HeaderVersion::NES2_0 ? "NES2.0" : "INES");
        char PRGSizeStr[128];
        sprintf(PRGSizeStr, "PRG Size: 0x%zx (%zu)", globalROM.PRGRomSize, globalROM.PRGRomSize);
        char CHRSizeStr[128];
        sprintf(CHRSizeStr, "CHR Size: 0x%zx (%zu)", globalROM.CHRRomSize, globalROM.CHRRomSize);
        std::string mapperStr = "Mapper: " + std::string(globalROM.mapper ? globalROM.mapper->getName() : (globalROM.MapperID ? "Unknown" : "NROM")) + " (Mapper " + std::to_string(globalROM.MapperID)+")";
        std::string subMapperStr = "Sub Mapper: " + std::to_string(globalROM.SubMapperID);
        std::string batteryStr = "Battery: " + std::string(globalROM.hasBattery ? "Yes" : "No");
        std::string CHRRamStr = "CHR-RAM: " + std::string(globalROM.CHRRomSize == 0 ? "Yes" : "No");
        
        QDialog* dialog = new QDialog(&window);
        dialog->setWindowTitle("ROM Info");
        dialog->setFixedSize(320, 180);

        std::string fullInfo = joinLines({fileStr, HeaderHexStr, headVerStr, PRGSizeStr, CHRSizeStr, mapperStr, subMapperStr, batteryStr, CHRRamStr});
        if (!romIsLoaded) fullInfo = "ROM isn't loaded!";
        QLabel* label = new QLabel(QString::fromStdString(fullInfo), dialog);
        label->setAlignment(Qt::AlignCenter);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        layout->addWidget(label);

        dialog->setLayout(layout);
        dialog->exec();
    });

    ScreenWidget *screen = new ScreenWidget(ppu.frameBuffer);
    window.setCentralWidget(screen);
    InputManager inputMgr;
    inputMgr.install(&window);

    if (argc > 1) {
        if (globalROM.LoadNES(argv[1])) {
            cpu.reset();
            romIsLoaded = true;
        }
    }

    ppu.Init();

    QTimer cpuTimer;
    QObject::connect(&cpuTimer, &QTimer::timeout, [&]() {
        if (romIsLoaded) {
            cpu.run((uint32_t)(89342 * CPUSpeed));
            screen->update();
            rainbowHoverPhase += 0.001f;
            if (rainbowHoverPhase > 1.0f) rainbowHoverPhase -= 1.0f;
        }
    });
    cpuTimer.start(16);

    window.setFixedSize(NES_WIDTH*3, NES_HEIGHT*3);
    window.setWindowIcon(QIcon("gui/ico.png"));
    window.show();

    return app.exec();
}