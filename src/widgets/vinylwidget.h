#ifndef VINYLWIDGET_H
#define VINYLWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include "theme.h"

class VinylWidget : public QWidget {
    Q_OBJECT
public:
    explicit VinylWidget(int size = 52, QWidget *parent = nullptr);

    void setGradient(const Theme::GradientPair &gradient);
    void setSpinning(bool spinning);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_size;
    Theme::GradientPair m_gradient;
    bool m_spinning = false;
    qreal m_angle = 0;
    QTimer m_timer;
};

#endif // VINYLWIDGET_H