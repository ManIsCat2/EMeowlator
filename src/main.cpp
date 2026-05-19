#include "nes/nes_cpu.hpp"
#include "nes/nes_apu.hpp"
#include "nes/nes_rom.hpp"
#include "nes/audio.hpp"

#include "main.hpp"
#include "qt/screen_widget.hpp"
#include "qt/input_manager.hpp"
#include "qt/palette_editor.hpp"
#include "config.hpp"
#include "savestate.hpp"

#ifdef _WIN32
#include <direct.h>
#define makeDirectory(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define makeDirectory(dir) mkdir(dir, 0777)
#endif

NesROM globalROM;

bool romIsLoaded = false;
static bool fullscreen = false;
static bool unlimitFPS = false;
int hoveredPaletteIndex = -1;

void *globalQTWin;

unsigned char MeowNESIcon[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00,
    0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xae, 0xce, 0x1c, 0xe9, 0x00, 0x00,
    0x00, 0x04, 0x67, 0x41, 0x4d, 0x41, 0x00, 0x00, 0xb1, 0x8f, 0x0b, 0xfc,
    0x61, 0x05, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00,
    0x0e, 0xc3, 0x00, 0x00, 0x0e, 0xc3, 0x01, 0xc7, 0x6f, 0xa8, 0x64, 0x00,
    0x00, 0x00, 0x73, 0x49, 0x44, 0x41, 0x54, 0x38, 0x4f, 0xcd, 0x91, 0xcb,
    0x09, 0xc0, 0x30, 0x0c, 0x43, 0xb3, 0x93, 0x27, 0xf1, 0x28, 0x99, 0x29,
    0x83, 0xa6, 0xe8, 0x90, 0x62, 0xcb, 0xce, 0xaf, 0xa7, 0x0a, 0x1e, 0x94,
    0xf0, 0xa4, 0xb4, 0xb4, 0x94, 0x5f, 0xa7, 0xd6, 0xda, 0xc1, 0xe9, 0x79,
    0x08, 0xa4, 0xde, 0x9a, 0x93, 0x47, 0xf9, 0x7a, 0x20, 0x83, 0xfd, 0x10,
    0x2b, 0x8b, 0x48, 0x78, 0x66, 0x3f, 0xe4, 0x68, 0x60, 0xf5, 0x3a, 0x9f,
    0x07, 0x6c, 0x71, 0x45, 0x3a, 0xc0, 0xd2, 0x0c, 0x7b, 0xa1, 0x1b, 0x61,
    0x71, 0x5a, 0xb2, 0x61, 0x11, 0xa8, 0xea, 0xcb, 0x76, 0x00, 0xe1, 0x81,
    0x19, 0xdc, 0x73, 0x61, 0x39, 0x83, 0x3b, 0x2e, 0x2c, 0x03, 0xfb, 0x09,
    0xdb, 0x11, 0xfc, 0xdf, 0x0c, 0x8c, 0xf0, 0xd9, 0xe8, 0x3c, 0x77, 0x43,
    0x1b, 0x22, 0x0c, 0x2c, 0xbb, 0xb7, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
    0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

std::string joinLines(std::vector<std::string> lines) {
    std::string result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result += lines[i];
        if (i != lines.size() - 1) result += "\n";
    }
    return result;
}

QAction *makeQBool(const QString &text, QObject *parent, bool defaultBool) {
    QAction *newAction = new QAction(text, parent);
    newAction->setCheckable(true);
    newAction->setChecked(defaultBool);
    return newAction;
}

QImage *rawOutputImage = nullptr;
QImage *filteredOutputImage = nullptr;
QTimer cpuTimer;

void startCPUTimer(void) {
    if (globalROM.Region == ConsoleRegion::NTSC) {
        cpuTimer.start(16);
    } else {
        cpuTimer.start(20);
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    QMainWindow window;
    globalQTWin = (void*)&window;

    makeDirectory("saves");

    Config::Load("meowconf.txt");

    QMenuBar *menuBar = window.menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QMenu *gameMenu = menuBar->addMenu("Game");
    QMenu *settingsMenu = menuBar->addMenu("Options");
    QMenu *miscMenu = menuBar->addMenu("Misc");

    //file
    QAction *openAction = new QAction("Open ROM", &window);
    QAction *closeAction = new QAction("Close ROM", &window);
    QAction *saveSaveStateAction = new QAction("Save Savestate", &window);
    QAction *loadSaveStateAction = new QAction("Load Savestate", &window);
    //game
    QAction *gameResetAction = new QAction("Reset", &window);
    QAction *gamePauseAction = new QAction("Pause", &window);
    // options
   // QAction *paletteEditorAction = new QAction("Palette Editor", &window);
    QAction *keyEditAction = new QAction("Input Config", &window);
    QAction *audioConfAction = new QAction("Audio Config", &window);
    QAction *displayConfAction = new QAction("Display Config", &window);
    // misc
    QAction *romInfoAction = new QAction("ROM Info", &window);
    QAction *exitAction = new QAction("Exit", &window);

    fileMenu->addAction(openAction);
    fileMenu->addAction(closeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveSaveStateAction);
    fileMenu->addAction(loadSaveStateAction);

    gameMenu->addAction(gameResetAction);
    gameMenu->addAction(gamePauseAction);

    //settingsMenu->addAction(VRAMCorruptAction);
    //settingsMenu->addAction(disableSpritesAction);
    /*QMenu *videoFilterMenu = settingsMenu->addMenu("Video Filter");

    std::array<std::string, 4> videoFiltersStr[] = {"None", "NTSC", "Chroma", "Grayscale"};
	for (int i = 0; i < (int)videoFiltersStr->size(); i++) {
	    QAction *vfilterAction = new QAction(QString::fromStdString(videoFiltersStr->data()[i]), &window);

		videoFilterMenu->addAction(vfilterAction);

		QObject::connect(vfilterAction, &QAction::triggered, [i]{ ppu.InitFilter((VideoFilter)i); DebugPrintLog("SETTINGS", "Set vFilter to %u", i); } );
	}*/

    //settingsMenu->addAction(paletteEditorAction);
    //settingsMenu->addSeparator();
    settingsMenu->addAction(keyEditAction);
    settingsMenu->addAction(audioConfAction);
    settingsMenu->addAction(displayConfAction);

    miscMenu->addAction(romInfoAction);
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
                startCPUTimer();
            }
        }
    });
    QObject::connect(closeAction, &QAction::triggered, [&]() {
        if (romIsLoaded) {
            globalROM.mapper->saveSRAM();
            romIsLoaded = false;
            cpu.reset();
        }
    });
    QObject::connect(gameResetAction, &QAction::triggered, [&]() {
        cpu.reset();
        DebugPrintLog("GAME", "Reset Game");
    });
    QObject::connect(gamePauseAction, &QAction::triggered, [&]() {
        cpu.CPUPaused = !cpu.CPUPaused;
        DebugPrintLog("GAME", cpu.CPUPaused ? "Paused Game" : "Unpaused Game");
    });
    QObject::connect(keyEditAction, &QAction::triggered, [&]() {
        QDialog *dialog = new QDialog(&window);
        dialog->setWindowTitle("Input Config");
        dialog->setFixedSize(300, 340);

        QVBoxLayout *layout = new QVBoxLayout(dialog);

        std::vector<KeyCaptureButton*> buttons[sizeof(nesKeyBinds) / sizeof(nesKeyBinds[0])];
        for (auto &bind : nesKeyBinds) {
            QHBoxLayout *row = new QHBoxLayout();
            QLabel *label = new QLabel(bind.name);

            KeyCaptureButton *button = new KeyCaptureButton(
                QKeySequence(bind.key).toString(),
                &bind.key,
                dialog
            );
            buttons->push_back(button);

            QObject::connect(button, &QPushButton::clicked, [button]() {
                button->setText("Press key...");
                button->waitingForKey = true;
                button->setFocus();
            });

            row->addWidget(label);
            row->addWidget(button);
            layout->addLayout(row);
        }

        QPushButton *resetButton = new QPushButton("Reset Binds", dialog);
        QObject::connect(resetButton, &QPushButton::clicked, dialog, [&]() {
            memcpy(nesKeyBinds, nesKeyBindsDefault, sizeof(nesKeyBinds));
            for (size_t i = 0; i < buttons->size(); i++) {
                KeyCaptureButton *button = buttons->data()[i];
                button->setText(QKeySequence(nesKeyBindsDefault[i].key).toString());
            }
        });

        layout->addStretch();
        layout->addWidget(resetButton);

        dialog->setLayout(layout);
        dialog->exec();
    });
    QObject::connect(audioConfAction, &QAction::triggered, [&]() {
        QDialog *dialog = new QDialog(&window);
        dialog->setWindowTitle("Audio Config");
        dialog->setFixedSize(500, 340);

        QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
        QGroupBox *volumeBox = new QGroupBox("Volume", dialog);
        QHBoxLayout *volumeLayout = new QHBoxLayout(volumeBox);
        volumeLayout->setSpacing(5);
        volumeLayout->setContentsMargins(10,10,10,10);

        const std::string volumeNames[] = {"Square 1", "Square 2", "Triangle", "Noise", "DMC", "Master"};
        float *volumePtrs[] = { &apu.pulse1Volume, &apu.pulse2Volume, &apu.triangleVolume, &apu.noiseVolume, &apu.dmcVolume, &apu.masterVolume };

        for (int i = 0; i < 6; ++i) {
            QWidget *pairWidget = new QWidget(volumeBox);
            QVBoxLayout *pairLayout = new QVBoxLayout(pairWidget);
            pairLayout->setContentsMargins(0,0,0,0);
            pairLayout->setSpacing(5);

            QLabel *label = new QLabel(QString::fromStdString(volumeNames[i]), pairWidget);
            label->setAlignment(Qt::AlignHCenter);
            pairLayout->addWidget(label);

            QSlider *slider = new QSlider(Qt::Vertical, pairWidget);
            slider->setRange(0, 50);
            slider->setValue((int)(*volumePtrs[i]));
            slider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            slider->setTickPosition(QSlider::TicksRight);
            slider->setTickInterval(10);
            pairLayout->addWidget(slider);

            QObject::connect(slider, &QSlider::valueChanged, [volumePtrs, i](int value) {
                *volumePtrs[i] = (float)(value);
            });

            volumeLayout->addWidget(pairWidget);
        }

        volumeBox->setLayout(volumeLayout);
        mainLayout->addWidget(volumeBox);
        dialog->setLayout(mainLayout);

        dialog->exec();
    });
    QObject::connect(displayConfAction, &QAction::triggered, [&]() {
        QDialog *dialog = new QDialog(&window);
        dialog->setWindowTitle("Display Config");
        dialog->setFixedSize(560, 420);

        QGridLayout *mainLayout = new QGridLayout(dialog);
        QGroupBox *settingsBox = new QGroupBox("Settings", dialog);
        QGridLayout *settingsLayout = new QGridLayout(settingsBox);

        settingsBox->setMinimumSize(270, 120);
        //printf("dihh %u %u\n", settingsBox->size().width(), settingsBox->size().height());

        //region
        QGroupBox *regionBox = new QGroupBox("Region", dialog);
        QVBoxLayout *regionLayout = new QVBoxLayout(regionBox);
        QComboBox *tvtypeComboBox = new QComboBox(regionBox);

       // regionBox->setFixedSize(90, 90);
        std::array<std::string, 3> regionStrs = {"NTSC", "PAL", "Dendy"};

        for (int i = 0; i < regionStrs.size(); ++i) {
            tvtypeComboBox->addItem(QString::fromStdString(regionStrs[i]), i);
        }

        tvtypeComboBox->setCurrentIndex((int)(globalROM.Region));

        QComboBox::connect(tvtypeComboBox, &QComboBox::currentIndexChanged, [&](int index) {
            globalROM.Region = (ConsoleRegion)index;
            startCPUTimer();
            DebugPrintLog("SETTINGS", "Set Region to %d", index);
        });

        regionLayout->addWidget(tvtypeComboBox);
        //regionLayout->addStretch();
        
        //ppu settings
        QGroupBox *ppuSettingsBox = new QGroupBox("PPU Settings", dialog);
        QGridLayout *ppuSettingsLayout = new QGridLayout(ppuSettingsBox);
        QCheckBox *VRAMCorruptCheckBox = new QCheckBox("VRAM Corruption", ppuSettingsBox);

        ppuSettingsBox->setFixedSize(270, 160);

        VRAMCorruptCheckBox->setChecked(ppu.VRAMCorruption);

        QCheckBox *disableSpritesCheckBox = new QCheckBox("Disable Sprites", ppuSettingsBox);
        disableSpritesCheckBox->setChecked(ppu.DisableSprites);

        ppuSettingsLayout->addWidget(VRAMCorruptCheckBox, 0, 0, Qt::AlignLeft | Qt::AlignTop);
        ppuSettingsLayout->addWidget(disableSpritesCheckBox, 0, 1, Qt::AlignLeft | Qt::AlignTop);

        QObject::connect(VRAMCorruptCheckBox, &QCheckBox::toggled, [&](bool checked) {
            ppu.VRAMCorruption = checked;
        });
        QObject::connect(disableSpritesCheckBox, &QCheckBox::toggled, [&](bool checked) {
            ppu.DisableSprites = checked;
        });

        //nt

        QGroupBox *ntmirrorBox = new QGroupBox("NT Mirroring", dialog);
        QVBoxLayout *ntmirrorLayout = new QVBoxLayout(ntmirrorBox);
        QComboBox *ntmirrorComboBox = new QComboBox(ntmirrorBox);

       // regionBox->setFixedSize(90, 90);
        std::array<std::string, 5> ntmirrorStrs = {"Horizontal", "Vertical", "Screen A", "Screen B", "Fourscreen"};

        for (int i = 0; i < ntmirrorStrs.size(); ++i) {
            ntmirrorComboBox->addItem(QString::fromStdString(ntmirrorStrs[i]), i);
        }

        ntmirrorComboBox->setCurrentIndex((int)(ppu.Mirroring));

        QComboBox::connect(ntmirrorComboBox, &QComboBox::currentIndexChanged, [&](int index) {
            ppu.Mirroring = (MirrorMode)index;
            DebugPrintLog("SETTINGS", "Set NT Mirroring to %d", index);
        });

        ntmirrorLayout->addWidget(ntmirrorComboBox);
        ppuSettingsLayout->addWidget(ntmirrorBox);

        //palettes
        QGroupBox *paletteBox = new QGroupBox("Palette", dialog);
        QGridLayout *paletteLayout = new QGridLayout(paletteBox);
        PaletteButton* buttons[64];

        paletteBox->setFixedHeight(400);

        auto updateAllButtonsColor = [&]() {
            for (int i = 0; i < 64; i++) {
                QColor color(
                    (nesPalette[i] >> 16) & 0xFF,
                    (nesPalette[i] >> 8) & 0xFF,
                    nesPalette[i] & 0xFF
                );
                buttons[i]->setStyleSheet(QString("background-color: %1").arg(color.name()));
                if (ppu.filtering == VideoFilter::NTSC) {
                    ppu.vfilter->initialize();
                }
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
                        (newColor.red() << 16) |
                        (newColor.green() << 8) |
                        newColor.blue();
                    updateButtonColor();
                    if (ppu.filtering == VideoFilter::NTSC) {
                        ppu.vfilter->initialize();
                    }
                }
            });

            paletteLayout->addWidget(buttons[i], i / 8, i % 8);
        }

        QPushButton *resetButton = new QPushButton("Reset Palette", dialog);
        QPushButton *randomButton = new QPushButton("Randomize Palette", dialog);
        QPushButton *exportButton = new QPushButton("Export Palette", dialog);
        QPushButton *importButton = new QPushButton("Import Palette", dialog);
        paletteLayout->addWidget(resetButton, 8, 0, 1, 8);
        paletteLayout->addWidget(randomButton, 9, 0, 1, 8);
        paletteLayout->addWidget(exportButton, 10, 0, 1, 8);
        paletteLayout->addWidget(importButton, 11, 0, 1, 8);
        resetButton->setFixedHeight(25);
        randomButton->setFixedHeight(25);
        exportButton->setFixedHeight(25);
        importButton->setFixedHeight(25);
        QObject::connect(resetButton, &QPushButton::clicked, [&]() {
            memcpy(nesPalette, nesPaletteDefault, sizeof(nesPaletteDefault));
            updateAllButtonsColor();
        });
        QObject::connect(randomButton, &QPushButton::clicked, [&]() {
            for (int i = 0; i < 64; i++) {
                nesPalette[i] = 0xFF000000 | (rand() << 16) | (rand() << 8) | rand();
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

        QGroupBox *filterBox = new QGroupBox("Video Filter", dialog);
        QVBoxLayout *filterLayout = new QVBoxLayout(filterBox);
        QComboBox *filterComboBox = new QComboBox(filterBox);

        std::array<std::string, 4> videoFiltersStr = {"None", "NTSC", "Chroma", "Grayscale"};

        for (int i = 0; i < videoFiltersStr.size(); ++i) {
            filterComboBox->addItem(QString::fromStdString(videoFiltersStr[i]), i);
        }

        filterComboBox->setCurrentIndex((int)(ppu.filtering));

        QComboBox::connect(filterComboBox, &QComboBox::currentIndexChanged, [&](int index) {
            ppu.InitFilter((VideoFilter)(index));
            DebugPrintLog("SETTINGS", "Set vFilter to %d", index);
        });

        filterLayout->addWidget(filterComboBox);
        filterLayout->addStretch();

        settingsBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        ppuSettingsBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QVBoxLayout *leftLayout = new QVBoxLayout();
        leftLayout->setAlignment(Qt::AlignTop);

        leftLayout->addWidget(settingsBox);
        leftLayout->addWidget(ppuSettingsBox);
        leftLayout->addStretch();

        mainLayout->addLayout(leftLayout, 0, 0);
        mainLayout->addWidget(paletteBox, 0, 1);

        settingsLayout->addWidget(regionBox, 2, 0);
        settingsLayout->addWidget(filterBox, 2, 1);
        dialog->setLayout(mainLayout);
        //mainLayout->addStretch();
        dialog->exec();
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
            savestate.Write(file.toStdString().c_str());
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
            savestate.Load(file.toStdString().c_str());
        }
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
        sprintf(PRGSizeStr, "PRG Size: %uKiB (%u x 16KiB)", globalROM.PRGNumPages * 16, globalROM.PRGNumPages);
        char CHRSizeStr[128];
        sprintf(CHRSizeStr, "CHR Size: %uKiB (%u x 8KiB)", globalROM.CHRNumPages * 8, globalROM.CHRNumPages);
        std::string mapperStr = "Mapper: " + std::string(globalROM.mapper ? globalROM.mapper->getName() : (globalROM.MapperID ? "Unknown" : "NROM")) + " (Mapper " + std::to_string(globalROM.MapperID)+")";
        std::string subMapperStr = "Sub Mapper: " + std::to_string(globalROM.SubMapperID);
        std::string mirrorStr = "Mirroring: " + std::string(globalROM.Mirroring == MirrorMode::HORIZONTAL ? "Horizontal" : "Vertical");
        std::string batteryStr = "Battery: " + std::string(globalROM.hasBattery ? "Yes" : "No");
        std::string CHRRamStr = "CHR-RAM: " + std::string(globalROM.CHRRomSize == 0 ? "Yes" : "No");
        char batterySizeStr[128];
        size_t SRAMSize = globalROM.hasBattery ? globalROM.mapper->getSRAMSize() : 0x0000; 
        sprintf(batterySizeStr, "SRAM/Battery Size: 0x%zx (%zu)", SRAMSize, SRAMSize);
        char RESETVecStr[128];
        sprintf(RESETVecStr, "RESET Vector: 0x%x", globalROM.ResetVec);
        
        QDialog* dialog = new QDialog(&window);
        dialog->setWindowTitle("ROM Info");
        dialog->setFixedSize(350, 250);

        std::string fullInfo = joinLines({fileStr, HeaderHexStr, headVerStr, PRGSizeStr, CHRSizeStr, mapperStr, subMapperStr, mirrorStr, batteryStr, CHRRamStr, batterySizeStr, RESETVecStr});
        if (!romIsLoaded) fullInfo = "ROM isn't loaded!";
        QLabel* label = new QLabel(QString::fromStdString(fullInfo), dialog);
        label->setAlignment(Qt::AlignCenter);

        QVBoxLayout* layout = new QVBoxLayout(dialog);
        layout->addWidget(label);

        dialog->setLayout(layout);
        dialog->exec();
    });

    ScreenWidget *screen = new ScreenWidget(&window);
    screen->image = *rawOutputImage;
    window.setCentralWidget(screen);
    InputManager inputMgr;
    inputMgr.install(&window);

    ppu.Init();
    audioSystem.init();
    cpuTimer.setTimerType(Qt::PreciseTimer);
    QObject::connect(&cpuTimer, &QTimer::timeout, [&]() {
        if (romIsLoaded) {
            uint32_t speed = CYCLES_PER_FRAME_NTSC;
            if (globalROM.Region == ConsoleRegion::PAL) {
                speed = CYCLES_PER_FRAME_PAL;
            }
            cpu.run(speed);
            if (ppu.filtering == VideoFilter::NTSC) {
                screen->image = *filteredOutputImage;
            } else {
                screen->image = *rawOutputImage;
            }
            screen->update();
            rainbowHoverPhase += 0.002f;
            if (ppu.filtering == VideoFilter::NTSC) {
                ppu.vfilter->initialize();
            }
            if (rainbowHoverPhase > 1.0f) rainbowHoverPhase -= 1.0f;
        }
    });

    window.setFixedSize(NES_WIDTH*3, NES_HEIGHT*3);

    QPixmap pixmap;
    pixmap.loadFromData(MeowNESIcon, sizeof(MeowNESIcon) / sizeof(MeowNESIcon[0]));
    window.setWindowTitle("MeowNES");
    window.setWindowIcon(QIcon(pixmap));
    window.show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
        globalROM.mapper->saveSRAM();
        audioSystem.close();
        Config::Write("meowconf.txt");
    });

    if (argc > 1) {
        if (globalROM.LoadNES(argv[1])) {
            cpu.reset();
            romIsLoaded = true;
            startCPUTimer();
        }
    }

    return app.exec();
}