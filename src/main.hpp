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
#include "nes_cpu.hpp"
#include "nes_controller.hpp"

#include "mappers/mappers.hpp"

#include "nes_rom.hpp"

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

extern bool romIsLoaded;
extern NesROM globalROM;
extern bool showDebugLogs;
extern void *globalQTWin;