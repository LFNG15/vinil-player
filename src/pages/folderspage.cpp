#include "folderspage.h"
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>

FoldersPage::FoldersPage(TrackModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto *content = new QWidget();
    content->setStyleSheet("background: transparent;");
    m_contentLayout = new QVBoxLayout(content);
    m_contentLayout->setContentsMargins(32, 28, 32, 28);
    m_contentLayout->setSpacing(12);

    scroll->setWidget(content);
    outerLayout->addWidget(scroll);
}

void FoldersPage::refresh() {
    QLayoutItem *item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto *title = new QLabel("Pastas");
    title->setFont(Theme::titleFont(28));
    title->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::text().name()));
    m_contentLayout->addWidget(title);
    m_contentLayout->addSpacing(8);

    auto folders = m_model->folders();

    if (folders.isEmpty()) {
        auto *empty = new QLabel("Nenhuma pasta ainda\nPastas são criadas ao adicionar músicas");
        empty->setFont(Theme::bodyFont(14));
        empty->setStyleSheet(QString("color: %1; background: transparent; padding-top: 60px;").arg(Theme::textMuted().name()));
        empty->setAlignment(Qt::AlignCenter);
        m_contentLayout->addWidget(empty);
        m_contentLayout->addStretch();
        return;
    }

    auto *grid = new QGridLayout();
    grid->setSpacing(16);
    int col = 0, row = 0;

    for (auto &f : folders) {
        auto ft = m_model->tracksInFolder(f.name);

        auto *card = new QPushButton();
        card->setFixedSize(180, 210);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QString(
            "QPushButton { background: %1; border: none; border-radius: 12px; text-align: left; }"
            "QPushButton:hover { background: %2; }"
        ).arg(Theme::card().name(), Theme::cardHover().name()));

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(14, 14, 14, 14);
        cardLayout->setSpacing(8);

        // Cover mosaic
        auto *coverWidget = new QWidget();
        coverWidget->setFixedSize(152, 120);
        if (ft.size() >= 4) {
            auto *coverGrid = new QGridLayout(coverWidget);
            coverGrid->setSpacing(2);
            coverGrid->setContentsMargins(0, 0, 0, 0);
            for (int i = 0; i < 4; ++i) {
                auto *cell = new QWidget();
                cell->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border-radius: 4px;")
                    .arg(ft[i].cover.c1.name(), ft[i].cover.c2.name()));
                coverGrid->addWidget(cell, i / 2, i % 2);
            }
        } else {
            auto gradient = ft.isEmpty() ? Theme::randomPalette() : ft[0].cover;
            coverWidget->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border-radius: 8px;")
                .arg(gradient.c1.name(), gradient.c2.name()));
        }
        cardLayout->addWidget(coverWidget);

        auto *nameLabel = new QLabel(f.name);
        nameLabel->setFont(Theme::bodyFont(14));
        nameLabel->setStyleSheet(QString("color: %1; background: transparent; font-weight: bold;").arg(Theme::text().name()));
        cardLayout->addWidget(nameLabel);

        auto *countLabel = new QLabel(QString("%1 faixa%2").arg(ft.size()).arg(ft.size() != 1 ? "s" : ""));
        countLabel->setFont(Theme::bodyFont(11));
        countLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
        cardLayout->addWidget(countLabel);

        QString folderName = f.name;
        connect(card, &QPushButton::clicked, [this, folderName]() { emit folderSelected(folderName); });

        grid->addWidget(card, row, col);
        col++;
        if (col >= 4) { col = 0; row++; }
    }

    auto *gridWidget = new QWidget();
    gridWidget->setLayout(grid);
    gridWidget->setStyleSheet("background: transparent;");
    m_contentLayout->addWidget(gridWidget);
    m_contentLayout->addStretch();
}