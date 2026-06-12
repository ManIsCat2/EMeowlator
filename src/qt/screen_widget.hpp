#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>

class ScreenWidget : public QWidget {
public:
    QImage image;

    ScreenWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(256*2, 240*2);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        if (romIsLoaded) p.drawImage(rect(), image);
    }
};