#pragma once

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <cstdint>
#include "key_capture.hpp"
#include "nes.hpp"
#include "../nes/nes_controller.hpp"


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
        } else if (event->type() == QEvent::KeyRelease) {
            auto *k = (QKeyEvent*)(event);
            updateControllers(k->key(), false);
            return false;
        }
        return QObject::eventFilter(obj, event);
    }

private:
    void updateControllers(int qtKey, bool pressed) {
        for (int i=0;i<2;i++) {
            emuConsole->handleController(i, qtKey, pressed);
        }
    }
};