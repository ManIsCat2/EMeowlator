#include "nes_controller.hpp"

Controller controllers[2];

Keybind nesKeyBinds[8] = {
    {"A", Qt::Key_J},
    {"B", Qt::Key_K},

    {"Up", Qt::Key_W},
    {"Down", Qt::Key_S},
    {"Left", Qt::Key_A},
    {"Right", Qt::Key_D},

    {"Start", Qt::Key_Return},
    {"Select", Qt::Key_Shift}
};

Keybind nesKeyBindsDefault[8] = {
    {"A", Qt::Key_J},
    {"B", Qt::Key_K},

    {"Up", Qt::Key_W},
    {"Down", Qt::Key_S},
    {"Left", Qt::Key_A},
    {"Right", Qt::Key_D},

    {"Start", Qt::Key_Return},
    {"Select", Qt::Key_Shift}
};

//actual input code in qt/input_manager.hpp