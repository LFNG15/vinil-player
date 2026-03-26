#include "trackmodel.h"
#include <QRandomGenerator>
#include <algorithm>

TrackModel::TrackModel(QObject *parent) : QObject(parent) {}

QList<Track> &TrackModel::tracks() { return m_tracks; }
const QList<Track> &TrackModel::tracks() const { return m_tracks; }

void TrackModel::addTrack(const Track &track) {
    m_tracks.append(track);
    emit tracksChanged();
}

void TrackModel::removeTrack(const QString &id) {
    m_tracks.erase(std::remove_if(m_tracks.begin(), m_tracks.end(),
        [&](const Track &t) { return t.id == id; }), m_tracks.end());
    emit tracksChanged();
}

void TrackModel::toggleLike(const QString &id) {
    for (auto &t : m_tracks) {
        if (t.id == id) {
            t.liked = !t.liked;
            emit tracksChanged();
            return;
        }
    }
}

void TrackModel::setDuration(const QString &id, qint64 ms) {
    for (auto &t : m_tracks) {
        if (t.id == id) {
            t.durationMs = ms;
            emit tracksChanged();
            return;
        }
    }
}

Track *TrackModel::findTrack(const QString &id) {
    for (auto &t : m_tracks) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

QList<Folder> TrackModel::folders() const {
    QStringList seen;
    QList<Folder> result;
    for (auto &t : m_tracks) {
        if (!seen.contains(t.folder)) {
            seen.append(t.folder);
            result.append({ t.folder });
        }
    }
    return result;
}

QList<Track> TrackModel::tracksInFolder(const QString &folderName) const {
    QList<Track> result;
    for (auto &t : m_tracks) {
        if (t.folder == folderName) result.append(t);
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

int TrackModel::nextIndex(int currentIndex, bool shuffle) const {
    if (m_tracks.isEmpty()) return -1;
    if (shuffle) return QRandomGenerator::global()->bounded(m_tracks.size());
    return (currentIndex + 1) % m_tracks.size();
}

int TrackModel::prevIndex(int currentIndex) const {
    if (m_tracks.isEmpty()) return -1;
    return (currentIndex - 1 + m_tracks.size()) % m_tracks.size();
}