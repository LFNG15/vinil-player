#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include "trackmodel.h"

class Database {
public:
    static Database &instance();

    bool open();
    QString lastError() const { return m_lastError; }

    int  findOrCreateFolder(const QString &name, const QColor &c1, const QColor &c2);
    int  createFolder(const QString &name, const QColor &c1, const QColor &c2);
    QList<Folder> allFolders();
    void renameFolder(int id, const QString &newName);
    void updateFolderCover(int id, const QColor &c1, const QColor &c2);
    void deleteFolder(int id);

    int  insertTrack(const Track &t, int folderId);
    void deleteTrack(int id);
    void setLiked(int id, bool liked);
    void setDuration(int id, qint64 ms);
    void incrementPlayCount(int id);
    void moveTrackToFolder(int trackId, int folderId);
    QList<Track> allTracks();

    struct PlaybackState {
        int    trackId  = 0;
        qint64 posMs    = 0;
        double volume   = 0.7;
        bool   shuffle  = false;
        bool   repeat   = false;
    };
    PlaybackState loadState();
    void          saveState(const PlaybackState &s);

private:
    Database() = default;
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    void  applySchema();
    Track rowToTrack(const class QSqlQuery &q) const;

    QString m_lastError;
};

#endif // DATABASE_H
