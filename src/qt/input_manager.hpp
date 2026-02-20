#pragma once

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <cstdint>

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
            Controller &c = controllers[i];
            switch(qtKey) {
                case Qt::Key_Z:      pressed ? c.state|=A_BUTTON      : c.state&=~A_BUTTON; break;
                case Qt::Key_X:      pressed ? c.state|=B_BUTTON      : c.state&=~B_BUTTON; break;
                case Qt::Key_Shift:  pressed ? c.state|=SELECT_BUTTON : c.state&=~SELECT_BUTTON; break;
                case Qt::Key_Return: pressed ? c.state|=START_BUTTON  : c.state&=~START_BUTTON; break;
                case Qt::Key_Up:     pressed ? c.state|=STICK_UP      : c.state&=~STICK_UP; break;
                case Qt::Key_Down:   pressed ? c.state|=STICK_DOWN    : c.state&=~STICK_DOWN; break;
                case Qt::Key_Left:   pressed ? c.state|=STICK_LEFT    : c.state&=~STICK_LEFT; break;
                case Qt::Key_Right:  pressed ? c.state|=STICK_RIGHT   : c.state&=~STICK_RIGHT; break;
            }
        }
    }
};