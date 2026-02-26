#pragma once

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QPushButton>

class KeyCaptureButton : public QPushButton {
public:
    Qt::Key *boundKey;
    bool waitingForKey = false;

    KeyCaptureButton(const QString &text, Qt::Key *keyPtr, QWidget *parent = nullptr) : QPushButton(text, parent), boundKey(keyPtr) {

    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if (waitingForKey) {
            *boundKey = (Qt::Key)event->key();
            setText(QKeySequence(event->key()).toString());
            waitingForKey = false;
            return;
        }
        QPushButton::keyPressEvent(event);
    }
};