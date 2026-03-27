#include "trackmodel.h"
#include "database.h"
#include <QRandomGenerator>
#include <algorithm>

TrackModel::TrackModel(QObject *parent) : QObject(parent) {
    Database::instance().open();
    m_tracks = Database::instance().allTracks();
}

QList<Track> &TrackModel::tracks() { return m_tracks; }
const QList<Track> &TrackModel::tracks() const { return m_tracks; }

void TrackModel::addTrack(const Track &track) {
    int folderId = 0;
    if (!track.folder.isEmpty()) {
        folderId = Database::instance().findOrCreateFolder(
            track.folder, track.cover.c1, track.cover.c2);
    }

    int newId = Database::instance().insertTrack(track, folderId);

    Track t    = track;
    t.id       = newId;
    t.folderId = folderId;
    m_tracks.prepend(t);
    emit tracksChanged();
}

void TrackModel::removeTrack(int id) {
    Database::instance().deleteTrack(id);
    m_tracks.erase(std::remove_if(m_tracks.begin(), m_tracks.end(),
        [id](const Track &t) { return t.id == id; }), m_tracks.end());
    emit tracksChanged();
}

void TrackModel::toggleLike(int id) {
    for (auto &t : m_tracks) {
        if (t.id == id) {
            t.liked = !t.liked;
            Database::instance().setLiked(id, t.liked);
            emit tracksChanged();
            return;
        }
    }
}

void TrackModel::setDuration(int id, qint64 ms) {
    for (auto &t : m_tracks) {
        if (t.id == id) {
            t.durationMs = ms;
            Database::instance().setDuration(id, ms);
            return;
        }
    }
}

Track *TrackModel::findTrack(int id) {
    for (auto &t : m_tracks) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

QList<Folder> TrackModel::folders() const {
    return Database::instance().allFolders();
}

QList<Track> TrackModel::tracksInFolder(const QString &folderName) const {
    QList<Track> result;
    for (auto &t : m_tracks) {
        if (t.folder == folderName) result.append(t);
    }
    return result;
}

QList<Track> TrackModel::standaloneTracks() const {
    QList<Track> result;
    for (auto &t : m_tracks) {
        if (t.folderId == 0 && t.folder.isEmpty()) result.append(t);
    }
    return result;
}

QList<Track> TrackModel::likedTracks() const {
    QList<Track> result;
    for (auto &t : m_tracks) {
        if (t.liked) result.append(t);
    }
    return result;
}

QList<Track> TrackModel::recentTracks(int count) const {
    QList<Track> sorted = m_tracks;
    std::sort(sorted.begin(), sorted.end(),
        [](const Track &a, const Track &b) { return a.addedAt > b.addedAt; });
    return sorted.mid(0, count);
}

int TrackModel::createPlaylist(const QString &name, const QColor &c1, const QColor &c2) {
    int id = Database::instance().createFolder(name, c1, c2);
    emit tracksChanged();
    return id;
}

void TrackModel::renamePlaylist(int id, const QString &newName) {
    Database::instance().renameFolder(id, newName);
    for (auto &t : m_tracks) {
        if (t.folderId == id) t.folder = newName;
    }
    emit tracksChanged();
}

void TrackModel::updatePlaylistCover(int id, const QColor &c1, const QColor &c2) {
    Database::instance().updateFolderCover(id, c1, c2);
    emit tracksChanged();
}

void TrackModel::deletePlaylist(int id) {
    Database::instance().deleteFolder(id);
    for (auto &t : m_tracks) {
        if (t.folderId == id) {
            t.folderId = 0;
            t.folder   = "";
        }
    }
    emit tracksChanged();
}

void TrackModel::moveTrackToPlaylist(int trackId, int playlistId, const QString &playlistName) {
    Database::instance().moveTrackToFolder(trackId, playlistId);
    for (auto &t : m_tracks) {
        if (t.id == trackId) {
            t.folderId = playlistId;
            t.folder   = playlistName;
        }
    }
    emit tracksChanged();
}

int TrackModel::nextIndex(int currentIndex, bool shuffle) const {
    if (m_tracks.isEmpty()) return -1;
    if (shuffle) return QRandomGenerator::global()->bounded(m_tracks.size());
    return (currentIndex + 1) % m_tracks.size();
}

int TrackModel::prevIndex(int currentIndex) const {
    if (m_tracks.isEmpty()) return -1;
    return (currentIndex - 1 + m_tracks.size()) % m_tracks.size();
}
