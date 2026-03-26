#include "playerbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

PlayerBar::PlayerBar(TrackModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    setFixedHeight(90);
    setStyleSheet(QString("PlayerBar { background-color: %1; border-top: 1px solid %2; }")
        .arg(Theme::surface().name(), Theme::border().name()));

    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.7);

    // ── Layout ──────────────────────────────────────────────
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    mainLayout->setSpacing(12);

    // Left: vinyl + track info
    auto *leftLayout = new QHBoxLayout();
    leftLayout->setSpacing(12);

    m_vinyl = new VinylWidget(52, this);
    leftLayout->addWidget(m_vinyl);

    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(1);
    m_titleLabel = new QLabel("", this);
    m_titleLabel->setFont(Theme::bodyFont(13));
    m_titleLabel->setStyleSheet(QString("color: %1; font-weight: 600; background: transparent;").arg(Theme::text().name()));
    m_titleLabel->setMaximumWidth(180);

    m_artistLabel = new QLabel("", this);
    m_artistLabel->setFont(Theme::bodyFont(11));
    m_artistLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textSoft().name()));
    m_artistLabel->setMaximumWidth(180);

    infoLayout->addStretch();
    infoLayout->addWidget(m_titleLabel);
    infoLayout->addWidget(m_artistLabel);
    infoLayout->addStretch();

    leftLayout->addLayout(infoLayout);
    leftLayout->addStretch();

    auto *leftWidget = new QWidget(this);
    leftWidget->setLayout(leftLayout);
    leftWidget->setFixedWidth(240);
    leftWidget->setStyleSheet("background: transparent;");
    mainLayout->addWidget(leftWidget);

    // Center: controls + progress
    m_controlsContainer = new QWidget(this);
    m_controlsContainer->setStyleSheet("background: transparent;");
    auto *centerLayout = new QVBoxLayout(m_controlsContainer);
    centerLayout->setContentsMargins(0, 8, 0, 8);
    centerLayout->setSpacing(4);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(14);
    btnLayout->setAlignment(Qt::AlignCenter);

    m_shuffleBtn = new QPushButton("\uE14B", this);
    m_prevBtn    = new QPushButton("\uE100", this);
    m_playBtn    = new QPushButton("\uE102", this);
    m_nextBtn    = new QPushButton("\uE101", this);
    m_repeatBtn  = new QPushButton("\uE1CD", this);

    for (auto *btn : {m_shuffleBtn, m_prevBtn, m_nextBtn, m_repeatBtn}) {
        btn->setFixedSize(32, 32);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(buttonStyle(false));
    }

    m_playBtn->setFixedSize(38, 38);
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 19px; font-size: 16px; font-family: \"Segoe MDL2 Assets\"; }"
        "QPushButton:hover { background-color: %3; }"
    ).arg(Theme::accent().name(), Theme::bg().name(), Theme::accent().lighter(110).name()));

    btnLayout->addWidget(m_shuffleBtn);
    btnLayout->addWidget(m_prevBtn);
    btnLayout->addWidget(m_playBtn);
    btnLayout->addWidget(m_nextBtn);
    btnLayout->addWidget(m_repeatBtn);

    centerLayout->addLayout(btnLayout);

    // Progress row
    auto *progressLayout = new QHBoxLayout();
    progressLayout->setSpacing(8);

    m_timeLabel = new QLabel("0:00", this);
    m_timeLabel->setFont(Theme::monoFont(10));
    m_timeLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
    m_timeLabel->setFixedWidth(36);
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_progressSlider = new ClickableSlider(Qt::Horizontal, this);
    m_progressSlider->setRange(0, 1000);
    m_progressSlider->setValue(0);
    m_progressSlider->setStyleSheet(sliderStyle(Theme::accent().name()));

    m_durationLabel = new QLabel("0:00", this);
    m_durationLabel->setFont(Theme::monoFont(10));
    m_durationLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
    m_durationLabel->setFixedWidth(36);
    m_durationLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    progressLayout->addWidget(m_timeLabel);
    progressLayout->addWidget(m_progressSlider);
    progressLayout->addWidget(m_durationLabel);

    centerLayout->addLayout(progressLayout);

    mainLayout->addWidget(m_controlsContainer, 1);

    // Right: volume
    auto *volLayout = new QHBoxLayout();
    volLayout->setSpacing(8);

    auto *volIcon = new QLabel("\uE15D", this);
    volIcon->setFont(Theme::iconFont(14));
    volIcon->setStyleSheet("background: transparent;");

    m_volumeSlider = new ClickableSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setStyleSheet(sliderStyle(Theme::textSoft().name()));

    volLayout->addStretch();
    volLayout->addWidget(volIcon);
    volLayout->addWidget(m_volumeSlider);

    auto *rightWidget = new QWidget(this);
    rightWidget->setLayout(volLayout);
    rightWidget->setFixedWidth(160);
    rightWidget->setStyleSheet("background: transparent;");
    mainLayout->addWidget(rightWidget);

    // Empty state label
    m_emptyLabel = new QLabel("Adicione músicas para começar a ouvir", this);
    m_emptyLabel->setFont(Theme::bodyFont(12));
    m_emptyLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    m_controlsContainer->hide();
    leftWidget->hide();
    rightWidget->hide();

    // ── Connections ─────────────────────────────────────────
    connect(m_playBtn, &QPushButton::clicked, this, &PlayerBar::togglePlay);
    connect(m_prevBtn, &QPushButton::clicked, this, &PlayerBar::prev);
    connect(m_nextBtn, &QPushButton::clicked, this, &PlayerBar::next);

    connect(m_shuffleBtn, &QPushButton::clicked, this, [this]() {
        m_shuffle = !m_shuffle;
        m_shuffleBtn->setStyleSheet(buttonStyle(m_shuffle));
    });

    connect(m_repeatBtn, &QPushButton::clicked, this, [this]() {
        m_repeat = !m_repeat;
        m_repeatBtn->setStyleSheet(buttonStyle(m_repeat));
    });

    connect(m_progressSlider, &QSlider::sliderMoved, this, [this](int val) {
        qint64 dur = m_player->duration();
        if (dur > 0) {
            m_player->setPosition(dur * val / 1000);
        }
    });

    connect(m_volumeSlider, &QSlider::sliderMoved, this, [this](int val) {
        m_audioOutput->setVolume(val / 100.0);
    });

    connect(m_player, &QMediaPlayer::positionChanged, this, &PlayerBar::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &PlayerBar::onDurationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &PlayerBar::onMediaStatusChanged);
}

void PlayerBar::playTrack(const Track &track) {
    if (m_currentTrackId == track.id) {
        togglePlay();
        return;
    }

    m_currentTrackId = track.id;
    m_player->setSource(track.audioUrl);
    m_player->play();

    m_titleLabel->setText(track.title);
    m_artistLabel->setText(track.artist);
    m_vinyl->setGradient(track.cover);
    m_vinyl->setSpinning(true);

    m_playBtn->setText("\uE103");

    m_emptyLabel->hide();
    m_controlsContainer->show();
    // Show all child widgets (reveals left/right panels hidden in constructor)
    for (auto *child : findChildren<QWidget *>()) {
        child->show();
    }
    m_emptyLabel->hide();

    updateControls();
    emit trackChanged(m_currentTrackId);
    emit playingChanged(true);
}

void PlayerBar::togglePlay() {
    if (m_currentTrackId.isEmpty()) return;

    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
        m_playBtn->setText("\uE102");
        m_vinyl->setSpinning(false);
        emit playingChanged(false);
    } else {
        m_player->play();
        m_playBtn->setText("\uE103");
        m_vinyl->setSpinning(true);
        emit playingChanged(true);
    }
}

void PlayerBar::next() {
    if (m_currentTrackId.isEmpty()) return;
    auto &tracks = m_model->tracks();
    int idx = -1;
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].id == m_currentTrackId) { idx = i; break; }
    }
    if (idx < 0) return;

    int nextIdx;
    if (m_repeat) {
        nextIdx = idx;
    } else {
        nextIdx = m_model->nextIndex(idx, m_shuffle);
    }
    if (nextIdx >= 0 && nextIdx < tracks.size()) {
        m_currentTrackId.clear(); // force reload
        playTrack(tracks[nextIdx]);
    }
}

void PlayerBar::prev() {
    if (m_currentTrackId.isEmpty()) return;
    if (m_player->position() > 3000) {
        m_player->setPosition(0);
        return;
    }
    auto &tracks = m_model->tracks();
    int idx = -1;
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].id == m_currentTrackId) { idx = i; break; }
    }
    if (idx < 0) return;

    int prevIdx = m_model->prevIndex(idx);
    if (prevIdx >= 0 && prevIdx < tracks.size()) {
        m_currentTrackId.clear();
        playTrack(tracks[prevIdx]);
    }
}

void PlayerBar::setShuffle(bool on) { m_shuffle = on; }
void PlayerBar::setRepeat(bool on) { m_repeat = on; }
bool PlayerBar::isPlaying() const { return m_player->playbackState() == QMediaPlayer::PlayingState; }
QString PlayerBar::currentTrackId() const { return m_currentTrackId; }

void PlayerBar::onPositionChanged(qint64 pos) {
    m_timeLabel->setText(Theme::formatTime(pos));
    qint64 dur = m_player->duration();
    if (dur > 0 && !m_progressSlider->isSliderDown()) {
        m_progressSlider->setValue(static_cast<int>(pos * 1000 / dur));
    }
}

void PlayerBar::onDurationChanged(qint64 dur) {
    m_durationLabel->setText(Theme::formatTime(dur));
    if (!m_currentTrackId.isEmpty()) {
        m_model->setDuration(m_currentTrackId, dur);
    }
}

void PlayerBar::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        if (m_repeat) {
            m_player->setPosition(0);
            m_player->play();
        } else {
            next();
        }
    }
}

void PlayerBar::updateControls() {
    m_shuffleBtn->setStyleSheet(buttonStyle(m_shuffle));
    m_repeatBtn->setStyleSheet(buttonStyle(m_repeat));
}

QString PlayerBar::buttonStyle(bool active) const {
    QString color = active ? Theme::accent().name() : Theme::textMuted().name();
    QString hoverColor = active ? Theme::accent().lighter(110).name() : Theme::textSoft().name();
    return QString(
        "QPushButton { background: transparent; color: %1; border: none; border-radius: 16px; font-size: 14px; font-family: \"Segoe MDL2 Assets\"; }"
        "QPushButton:hover { color: %2; background: rgba(255,255,255,0.05); }"
    ).arg(color, hoverColor);
}

QString PlayerBar::sliderStyle(const QString &accentColor) const {
    return QString(R"(
        QSlider::groove:horizontal {
            height: 4px;
            background: rgba(255,255,255,0.08);
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: %2;
            width: 12px; height: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
        QSlider::handle:horizontal:hover {
            background: #f0ece4;
        }
        QSlider::sub-page:horizontal {
            background: %1;
            border-radius: 2px;
        }
    )").arg(accentColor, Theme::text().name());
}