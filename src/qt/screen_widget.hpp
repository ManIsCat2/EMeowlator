#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>

class ScreenWidget : public QWidget {
public:
    QImage *image = nullptr;

    ScreenWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        if (romIsLoaded && image) p.drawImage(rect(), *image);
    }

    QSize sizeHint() const override {
        if (romIsLoaded && image) {
            return image->size()*2;
        }
        return QSize(256*2, 240*2);
    }
};