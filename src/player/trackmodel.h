#ifndef TRACKMODEL_H
#define TRACKMODEL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QUrl>
#include <QDateTime>
#include <QColor>
#include "theme.h"

struct Track {
    int     id        = 0;
    QString title;
    QString artist;
    QString folder;
    int     folderId  = 0;
    qint64  durationMs = 0;
    int     playCount  = 0;
    Theme::GradientPair cover;
    bool    liked     = false;
    qint64  addedAt   = 0;
    QUrl    audioUrl;

    static Track create(const QString &title, const QString &artist,
                        const QString &folder, const QUrl &url) {
        Track t;
        t.title   = title;
        t.artist  = artist;
        t.folder  = folder;
        t.cover   = Theme::randomPalette();
        t.addedAt = QDateTime::currentMSecsSinceEpoch();
        t.audioUrl = url;
        return t;
    }
};

struct Folder {
    int     id = 0;
    QString name;
    Theme::GradientPair cover;
};

class TrackModel : public QObject {
    Q_OBJECT
public:
    explicit TrackModel(QObject *parent = nullptr);

    QList<Track> &tracks();
    const QList<Track> &tracks() const;

    void addTrack(const Track &track);
    void removeTrack(int id);
    void toggleLike(int id);
    void setDuration(int id, qint64 ms);

    Track *findTrack(int id);
    QList<Folder> folders() const;
    QList<Track> tracksInFolder(const QString &folderName) const;
    QList<Track> standaloneTracks() const;
    QList<Track> likedTracks() const;
    QList<Track> recentTracks(int count = 8) const;

    // Playlist CRUD
    int  createPlaylist(const QString &name, const QColor &c1, const QColor &c2);
    void renamePlaylist(int id, const QString &newName);
    void updatePlaylistCover(int id, const QColor &c1, const QColor &c2);
    void deletePlaylist(int id);
    void moveTrackToPlaylist(int trackId, int playlistId, const QString &playlistName);

    int nextIndex(int currentIndex, bool shuffle) const;
    int prevIndex(int currentIndex) const;

signals:
    void tracksChanged();

private:
    QList<Track> m_tracks;
};

#endif // TRACKMODEL_H
