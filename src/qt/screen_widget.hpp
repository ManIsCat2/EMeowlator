#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>

class ScreenWidget : public QWidget {
public:
    QImage image;

    ScreenWidget(uint32_t *fb, QWidget *parent = nullptr) : QWidget(parent), image((uint8_t*)(fb), NES_WIDTH, NES_HEIGHT, QImage::Format_ARGB32) {
        setFixedSize(NES_WIDTH*3, NES_HEIGHT*3);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.drawImage(rect(), image);
    }
};