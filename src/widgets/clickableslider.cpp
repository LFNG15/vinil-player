#include "clickableslider.h"

ClickableSlider::ClickableSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent) {}

void ClickableSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        setValue(valueFromPos(event->pos()));
        emit sliderMoved(value());
    }
    QSlider::mousePressEvent(event);
}

void ClickableSlider::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        setValue(valueFromPos(event->pos()));
        emit sliderMoved(value());
    }
    QSlider::mouseMoveEvent(event);
}

int ClickableSlider::valueFromPos(const QPoint &pos) const {
    int span = (orientation() == Qt::Horizontal) ? width() : height();
    int pixelPos = (orientation() == Qt::Horizontal) ? pos.x() : (height() - pos.y());
    double ratio = static_cast<double>(pixelPos) / span;
    ratio = qBound(0.0, ratio, 1.0);
    return minimum() + static_cast<int>(ratio * (maximum() - minimum()));
}