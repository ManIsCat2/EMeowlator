#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>
#include <QIcon>

#include "nes_cpu.hpp"
#include "nes_rom.hpp"

#include "main.hpp"
#include "qt/screen_widget.hpp"
#include "qt/input_manager.hpp"
#include "savestate.hpp"

NesROM globalROM;

bool romIsLoaded = false;
bool showDebugLogs = false;
static bool fullscreen = false;
static float CPUSpeed = 1.f;
static bool unlimitFPS = false;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;

    QMenuBar *menuBar = window.menuBar();
    QMenu *fileMenu = menuBar->addMenu("File");
    QMenu *CPUMenu = menuBar->addMenu("CPU");
    QMenu *PPUMenu = menuBar->addMenu("PPU");
    QMenu *miscMenu = menuBar->addMenu("Misc");

    QAction *openAction = new QAction("Open ROM", &window);
    QAction *closeAction = new QAction("Close ROM", &window);
    QAction *exitAction = new QAction("Exit", &window);

    fileMenu->addAction(openAction);
    fileMenu->addAction(closeAction);
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

    ScreenWidget *screen = new ScreenWidget(ppu.frameBuffer);
    window.setCentralWidget(screen);
    InputManager inputMgr;
    inputMgr.install(&window);

    QTimer cpuTimer;
    QObject::connect(&cpuTimer, &QTimer::timeout, [&]() {
        if (romIsLoaded) {
            ppu.Init();
            cpu.run((uint32_t)(89342 * CPUSpeed));
            ppu.Render();
            screen->update();
        }
    });
    cpuTimer.start(16);

    window.resize(NES_WIDTH*3, NES_HEIGHT*3);
    window.setWindowIcon(QIcon("gui/ico.png"));
    window.show();

    return app.exec();
}