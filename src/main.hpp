#pragma once

#include <cstdint>
#include <iostream>
#include <array>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

#include "nes.hpp"
#include "nes/nes_cpu.hpp"
#include "nes/nes_controller.hpp"

#include "nes/mappers/mappers.hpp"

#include "nes/nes_rom.hpp"

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>
#include <QIcon>
#include <QMessageBox>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QStyleFactory>
#include <QSlider>
#include <QGroupBox>

extern bool romIsLoaded;
extern NesROM globalROM;
extern void *globalQTWin;
extern int hoveredPaletteIndex;

struct Keybind {
    const char* name;
    Qt::Key key;
};
extern Keybind nesKeyBinds[];
extern QImage *rawOutputImage;
extern QImage *filteredOutputImage;