#pragma once

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <cstdint>
#include "key_capture.hpp"
#include "nes.hpp"
#include "../nes/nes_controller.hpp"

extern Keybind nesKeyBinds[];

class InputManager : public QObject {
public:
    InputManager(QObject *parent=nullptr) : QObject(parent) {}

    void install(QWidget *target) {
        target->installEventFilter(this);
        target->setFocusPolicy(Qt::StrongFocus);
    }
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            auto *k = (QKeyEvent*)(event);
            updateControllers(k->key(), true);
            return false;
        }
        else if (event->type() == QEvent::KeyRelease) {
            auto *k = (QKeyEvent*)(event);
            updateControllers(k->key(), false);
            return false;
        }
        return QObject::eventFilter(obj, event);
    }

private:
    void updateControllers(int qtKey, bool pressed) {
        for(int i=0;i<2;i++) {
            // oof
            Controller &c = controllers[i];
            if (qtKey == nesKeyBinds[0].key) pressed ? c.state|=A_BUTTON : c.state&=~A_BUTTON;
            if (qtKey == nesKeyBinds[1].key) pressed ? c.state|=B_BUTTON : c.state&=~B_BUTTON;
            if (qtKey == nesKeyBinds[2].key) pressed ? c.state|=STICK_UP : c.state&=~STICK_UP;
            if (qtKey == nesKeyBinds[3].key) pressed ? c.state|=STICK_DOWN : c.state&=~STICK_DOWN;
            if (qtKey == nesKeyBinds[4].key) pressed ? c.state|=STICK_LEFT : c.state&=~STICK_LEFT;
            if (qtKey == nesKeyBinds[5].key) pressed ? c.state|=STICK_RIGHT : c.state&=~STICK_RIGHT;
            if (qtKey == nesKeyBinds[6].key) pressed ? c.state|=START_BUTTON : c.state&=~START_BUTTON;
            if (qtKey == nesKeyBinds[7].key) pressed ? c.state|=SELECT_BUTTON : c.state&=~SELECT_BUTTON;
        }
    }
};