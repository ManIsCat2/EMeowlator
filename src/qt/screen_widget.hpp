#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>

class ScreenWidget : public QWidget {
public:
    QImage *image = nullptr;

    ScreenWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(256*2, 240*2);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        if (romIsLoaded && image) p.drawImage(rect(), *image);
    }
};