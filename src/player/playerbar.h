#ifndef PLAYERBAR_H
#define PLAYERBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "clickableslider.h"
#include "vinylwidget.h"
#include "trackmodel.h"

class PlayerBar : public QWidget {
    Q_OBJECT
public:
    explicit PlayerBar(TrackModel *model, QWidget *parent = nullptr);

    void playTrack(const Track &track);
    void togglePlay();
    void next();
    void prev();
    void setShuffle(bool on);
    void setRepeat(bool on);

    bool isPlaying() const;
    int  currentTrackId() const;

signals:
    void trackChanged(int trackId);
    void playingChanged(bool playing);

private slots:
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    void updateControls();
    QString buttonStyle(bool active = false) const;
    QString sliderStyle(const QString &accentColor) const;

    TrackModel *m_model;
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    VinylWidget *m_vinyl;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QLabel *m_timeLabel;
    QLabel *m_durationLabel;
    QLabel *m_emptyLabel;

    QPushButton *m_playBtn;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_shuffleBtn;
    QPushButton *m_repeatBtn;

    ClickableSlider *m_progressSlider;
    ClickableSlider *m_volumeSlider;
    QPushButton     *m_volIcon;

    QWidget *m_controlsContainer;

    int  m_currentTrackId = 0;
    bool m_shuffle = false;
    bool m_repeat  = false;

    void updateVolIcon();
};

#endif // PLAYERBAR_H