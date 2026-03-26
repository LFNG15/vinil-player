#include "folderdetailpage.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>

FolderDetailPage::FolderDetailPage(TrackModel *model, QWidget *parent)
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
    m_contentLayout->setSpacing(8);

    scroll->setWidget(content);
    outerLayout->addWidget(scroll);
}

void FolderDetailPage::setFolder(const QString &folderName) {
    m_folderName = folderName;
}

void FolderDetailPage::refresh(int currentTrackId, bool isPlaying) {
    QLayoutItem *item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // Back button
    auto *backBtn = new QPushButton("\uE0A6");
    backBtn->setFixedSize(34, 34);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFont(Theme::iconFont(12));
    backBtn->setStyleSheet(QString(
        "QPushButton { background: rgba(255,255,255,0.05); color: %1; border: none; border-radius: 17px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.1); }"
    ).arg(Theme::text().name()));
    connect(backBtn, &QPushButton::clicked, this, &FolderDetailPage::navigateBack);
    m_contentLayout->addWidget(backBtn, 0, Qt::AlignLeft);

    auto tracks = m_model->tracksInFolder(m_folderName);
    qint64 total = 0;
    for (auto &t : tracks) total += t.durationMs;

    // Header
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(20);

    auto *cover = new QWidget();
    cover->setFixedSize(140, 140);
    if (tracks.size() >= 4) {
        auto *coverGrid = new QGridLayout(cover);
        coverGrid->setSpacing(2);
        coverGrid->setContentsMargins(0, 0, 0, 0);
        for (int i = 0; i < 4; ++i) {
            auto *cell = new QWidget();
            cell->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border-radius: 4px;")
                .arg(tracks[i].cover.c1.name(), tracks[i].cover.c2.name()));
            coverGrid->addWidget(cell, i / 2, i % 2);
        }
    } else {
        auto g = tracks.isEmpty() ? Theme::randomPalette() : tracks[0].cover;
        cover->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border-radius: 10px;")
            .arg(g.c1.name(), g.c2.name()));
    }
    headerLayout->addWidget(cover);

    auto *infoLayout = new QVBoxLayout();
    infoLayout->addStretch();
    auto *typeLabel = new QLabel("PASTA");
    typeLabel->setFont(Theme::bodyFont(10));
    typeLabel->setStyleSheet(QString("color: %1; background: transparent; font-weight: bold; letter-spacing: 1px;").arg(Theme::textMuted().name()));
    infoLayout->addWidget(typeLabel);

    auto *nameLabel = new QLabel(m_folderName);
    nameLabel->setFont(Theme::titleFont(32));
    nameLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::text().name()));
    infoLayout->addWidget(nameLabel);

    auto *statsLabel = new QLabel(QString("%1 faixa%2%3")
        .arg(tracks.size())
        .arg(tracks.size() != 1 ? "s" : "")
        .arg(total > 0 ? QString(" • %1").arg(Theme::formatTime(total)) : ""));
    statsLabel->setFont(Theme::bodyFont(12));
    statsLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textSoft().name()));
    infoLayout->addWidget(statsLabel);
    infoLayout->addStretch();

    headerLayout->addLayout(infoLayout, 1);

    auto *headerWidget = new QWidget();
    headerWidget->setLayout(headerLayout);
    headerWidget->setStyleSheet("background: transparent;");
    m_contentLayout->addWidget(headerWidget);
    m_contentLayout->addSpacing(12);

    // Play button
    if (!tracks.isEmpty()) {
        auto *playBtn = new QPushButton("\uE102");
        playBtn->setFixedSize(48, 48);
        playBtn->setCursor(Qt::PointingHandCursor);
        playBtn->setFont(Theme::iconFont(16));
        playBtn->setStyleSheet(QString(
            "QPushButton { background: %1; color: %2; border: none; border-radius: 24px; font-family: \"Segoe MDL2 Assets\"; font-size: 16px; }"
            "QPushButton:hover { background: %3; }"
        ).arg(Theme::accent().name(), Theme::bg().name(), Theme::accent().lighter(110).name()));
        Track first = tracks[0];
        connect(playBtn, &QPushButton::clicked, [this, first]() { emit playRequested(first); });
        m_contentLayout->addWidget(playBtn, 0, Qt::AlignLeft);
        m_contentLayout->addSpacing(8);
    }

    // Track list
    for (int i = 0; i < tracks.size(); ++i) {
        auto &track = tracks[i];
        bool active = (track.id == currentTrackId);

        auto *row = new QWidget();
        row->setFixedHeight(52);
        row->setCursor(Qt::PointingHandCursor);
        row->setStyleSheet(QString(
            "QWidget#trackRow { background: %1; border-radius: 8px; border-left: 3px solid %2; }"
        ).arg(active ? "rgba(232,164,74,0.12)" : "transparent",
              active ? Theme::accent().name() : "transparent"));
        row->setObjectName("trackRow");

        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(12, 4, 12, 4);
        layout->setSpacing(12);

        auto *idx = new QLabel(QString("%1").arg(i + 1, 2, 10, QChar('0')));
        idx->setFont(Theme::monoFont(12));
        idx->setFixedWidth(28);
        idx->setAlignment(Qt::AlignCenter);
        idx->setStyleSheet(QString("color: %1; background: transparent;").arg(active ? Theme::accent().name() : Theme::textMuted().name()));
        layout->addWidget(idx);

        auto *swatch = new QWidget();
        swatch->setFixedSize(38, 38);
        swatch->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border-radius: 6px;")
            .arg(track.cover.c1.name(), track.cover.c2.name()));
        layout->addWidget(swatch);

        auto *titleLabel = new QLabel(track.title);
        titleLabel->setFont(Theme::bodyFont(13));
        titleLabel->setStyleSheet(QString("color: %1; background: transparent; font-weight: 600;").arg(active ? Theme::accent().name() : Theme::text().name()));
        layout->addWidget(titleLabel, 1);

        auto *likeBtn = new QPushButton(track.liked ? "\uE00B" : "\uE006");
        likeBtn->setFixedSize(28, 28);
        likeBtn->setCursor(Qt::PointingHandCursor);
        likeBtn->setStyleSheet(QString("QPushButton { background: transparent; color: %1; border: none; font-size: 14px; font-family: \"Segoe MDL2 Assets\"; }").arg(
            track.liked ? Theme::accent().name() : Theme::textMuted().name()));
        int lid = track.id;
        connect(likeBtn, &QPushButton::clicked, [this, lid]() { emit likeToggled(lid); });
        layout->addWidget(likeBtn);

        auto *dur = new QLabel(Theme::formatTime(track.durationMs));
        dur->setFont(Theme::monoFont(11));
        dur->setFixedWidth(40);
        dur->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dur->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
        layout->addWidget(dur);

        auto *delBtn = new QPushButton("\uE107");
        delBtn->setFixedSize(24, 24);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setStyleSheet(QString("QPushButton { background: transparent; color: %1; border: none; font-size: 11px; font-family: \"Segoe MDL2 Assets\"; } QPushButton:hover { color: %2; }").arg(
            Theme::textMuted().name(), Theme::danger().name()));
        int did = track.id;
        connect(delBtn, &QPushButton::clicked, [this, did]() { emit deleteRequested(did); });
        layout->addWidget(delBtn);

        // Click to play
        Track t = track;
        // Workaround: use a transparent button overlay
        auto *overlay = new QPushButton(row);
        overlay->setGeometry(0, 0, 9999, 52);
        overlay->setStyleSheet("background: transparent; border: none;");
        overlay->setCursor(Qt::PointingHandCursor);
        overlay->lower();
        connect(overlay, &QPushButton::clicked, [this, t]() { emit playRequested(t); });

        m_contentLayout->addWidget(row);
    }

    m_contentLayout->addStretch();
}