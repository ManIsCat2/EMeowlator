#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>

class ScreenWidget : public QWidget {
public:
    QImage image;

    ScreenWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(NES_WIDTH*3, NES_HEIGHT*3);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.drawImage(rect(), image);
    }
};