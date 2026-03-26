#ifndef TRACKMODEL_H
#define TRACKMODEL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QUrl>
#include <QUuid>
#include <QDateTime>
#include "theme.h"

struct Track {
    QString id;
    QString title;
    QString artist;
    QString folder;
    qint64 durationMs = 0;
    Theme::GradientPair cover;
    bool liked = false;
    qint64 addedAt = 0;
    QUrl audioUrl;

    static Track create(const QString &title, const QString &artist,
                        const QString &folder, const QUrl &url) {
        Track t;
        t.id = QUuid::createUuid().toString(QUuid::Id128).left(10);
        t.title = title;
        t.artist = artist;
        t.folder = folder;
        t.cover = Theme::randomPalette();
        t.addedAt = QDateTime::currentMSecsSinceEpoch();
        t.audioUrl = url;
        return t;
    }
};

struct Folder {
    QString name;
};

class TrackModel : public QObject {
    Q_OBJECT
public:
    explicit TrackModel(QObject *parent = nullptr);

    QList<Track> &tracks();
    const QList<Track> &tracks() const;

    void addTrack(const Track &track);
    void removeTrack(const QString &id);
    void toggleLike(const QString &id);
    void setDuration(const QString &id, qint64 ms);

    Track *findTrack(const QString &id);
    QList<Folder> folders() const;
    QList<Track> tracksInFolder(const QString &folderName) const;
    QList<Track> likedTracks() const;
    QList<Track> recentTracks(int count = 8) const;

    int nextIndex(int currentIndex, bool shuffle) const;
    int prevIndex(int currentIndex) const;

signals:
    void tracksChanged();

private:
    QList<Track> m_tracks;
};

#endif // TRACKMODEL_H