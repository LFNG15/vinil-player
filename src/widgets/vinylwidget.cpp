#include "vinylwidget.h"
#include <QConicalGradient>
#include <QRadialGradient>

VinylWidget::VinylWidget(int size, QWidget *parent)
    : QWidget(parent), m_size(size)
{
    setFixedSize(m_size, m_size);
    m_gradient = { Theme::accent(), Theme::danger() };

    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_angle += 2.0;
        if (m_angle >= 360.0) m_angle -= 360.0;
        update();
    });
}

void VinylWidget::setGradient(const Theme::GradientPair &gradient) {
    m_gradient = gradient;
    update();
}

void VinylWidget::setSpinning(bool spinning) {
    m_spinning = spinning;
    if (spinning && !m_timer.isActive()) {
        m_timer.start(30);  // ~33fps
    } else if (!spinning) {
        m_timer.stop();
    }
    update();
}

QSize VinylWidget::sizeHint() const { return QSize(m_size, m_size); }
QSize VinylWidget::minimumSizeHint() const { return QSize(m_size, m_size); }

void VinylWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPointF center(m_size / 2.0, m_size / 2.0);
    qreal radius = m_size / 2.0;

    p.save();
    p.translate(center);
    p.rotate(m_angle);
    p.translate(-center);

    // Vinyl body
    p.setBrush(Theme::vinylBlack());
    p.setPen(Qt::NoPen);
    p.drawEllipse(center, radius - 1, radius - 1);

    // Grooves
    QPen groovePen;
    groovePen.setWidthF(0.5);
    for (int i = 0; i < 10; ++i) {
        qreal r = radius * (0.25 + i * 0.065);
        groovePen.setColor(QColor(255, 255, 255, i % 2 == 0 ? 15 : 8));
        p.setPen(groovePen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(center, r, r);
    }

    // Center label
    qreal labelR = radius * 0.18;
    QRadialGradient labelGrad(center, labelR);
    labelGrad.setColorAt(0, m_gradient.c1);
    labelGrad.setColorAt(1, m_gradient.c2);
    p.setPen(Qt::NoPen);
    p.setBrush(labelGrad);
    p.drawEllipse(center, labelR, labelR);

    // Center hole
    qreal holeR = 2.5;
    p.setBrush(Theme::vinylBlack());
    p.drawEllipse(center, holeR, holeR);

    p.restore();
}