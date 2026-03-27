#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Database &Database::instance() {
    static Database db;
    return db;
}

bool Database::open() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dir + "/vinil.db");

    if (!db.open()) {
        m_lastError = db.lastError().text();
        qWarning() << "Database::open failed:" << m_lastError;
        return false;
    }

    applySchema();
    return true;
}

void Database::applySchema() {
    QSqlQuery q;
    q.exec("PRAGMA journal_mode = WAL");
    q.exec("PRAGMA foreign_keys = ON");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS folders (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            name         TEXT    NOT NULL UNIQUE,
            cover_color1 TEXT    NOT NULL DEFAULT '#e8a44a',
            cover_color2 TEXT    NOT NULL DEFAULT '#d45d5d',
            created_at   INTEGER NOT NULL DEFAULT (strftime('%s','now')*1000)
        )
    )");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS tracks (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            title        TEXT    NOT NULL,
            artist       TEXT    NOT NULL DEFAULT 'Desconhecido',
            folder_id    INTEGER NOT NULL DEFAULT 0,
            file_path    TEXT    NOT NULL,
            duration_ms  INTEGER NOT NULL DEFAULT 0,
            cover_color1 TEXT    NOT NULL DEFAULT '#e8a44a',
            cover_color2 TEXT    NOT NULL DEFAULT '#d45d5d',
            liked        INTEGER NOT NULL DEFAULT 0 CHECK (liked IN (0,1)),
            added_at     INTEGER NOT NULL DEFAULT (strftime('%s','now')*1000),
            play_count   INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (folder_id) REFERENCES folders(id) ON DELETE SET DEFAULT
        )
    )");

    q.exec("CREATE INDEX IF NOT EXISTS idx_tracks_folder ON tracks(folder_id)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_tracks_liked  ON tracks(liked) WHERE liked = 1");
    q.exec("CREATE INDEX IF NOT EXISTS idx_tracks_added  ON tracks(added_at DESC)");
    q.exec("CREATE INDEX IF NOT EXISTS idx_tracks_played ON tracks(play_count DESC) WHERE play_count > 0");

    q.exec(R"(
        CREATE TABLE IF NOT EXISTS playback_state (
            id               INTEGER PRIMARY KEY CHECK (id = 1),
            current_track_id INTEGER NOT NULL DEFAULT 0,
            position_ms      INTEGER NOT NULL DEFAULT 0,
            volume           REAL    NOT NULL DEFAULT 0.7 CHECK (volume >= 0.0 AND volume <= 1.0),
            shuffle          INTEGER NOT NULL DEFAULT 0 CHECK (shuffle IN (0,1)),
            repeat_mode      INTEGER NOT NULL DEFAULT 0 CHECK (repeat_mode IN (0,1))
        )
    )");

    q.exec("INSERT OR IGNORE INTO playback_state (id) VALUES (1)");
    q.exec("INSERT OR IGNORE INTO folders (id, name, cover_color1, cover_color2) VALUES (0, '', '#e8a44a', '#d45d5d')");
}


int Database::findOrCreateFolder(const QString &name, const QColor &c1, const QColor &c2) {
    QSqlQuery q;
    q.prepare("SELECT id FROM folders WHERE name = ?");
    q.addBindValue(name);
    q.exec();
    if (q.next()) return q.value(0).toInt();

    q.prepare("INSERT INTO folders (name, cover_color1, cover_color2) VALUES (?, ?, ?)");
    q.addBindValue(name);
    q.addBindValue(c1.name());
    q.addBindValue(c2.name());
    q.exec();
    return q.lastInsertId().toInt();
}

QList<Folder> Database::allFolders() {
    QList<Folder> result;
    QSqlQuery q("SELECT id, name, cover_color1, cover_color2 FROM folders WHERE id != 0 ORDER BY name");
    while (q.next()) {
        Folder f;
        f.id       = q.value(0).toInt();
        f.name     = q.value(1).toString();
        f.cover.c1 = QColor(q.value(2).toString());
        f.cover.c2 = QColor(q.value(3).toString());
        result.append(f);
    }
    return result;
}

int Database::createFolder(const QString &name, const QColor &c1, const QColor &c2) {
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO folders (name, cover_color1, cover_color2) VALUES (?, ?, ?)");
    q.addBindValue(name);
    q.addBindValue(c1.name());
    q.addBindValue(c2.name());
    q.exec();
    if (q.lastInsertId().toInt() > 0) return q.lastInsertId().toInt();
    // already existed - return its id
    q.prepare("SELECT id FROM folders WHERE name = ?");
    q.addBindValue(name);
    q.exec();
    return q.next() ? q.value(0).toInt() : 0;
}

void Database::renameFolder(int id, const QString &newName) {
    QSqlQuery q;
    q.prepare("UPDATE folders SET name = ? WHERE id = ?");
    q.addBindValue(newName);
    q.addBindValue(id);
    q.exec();
    // Update all tracks that referenced this folder name
    q.prepare("UPDATE tracks SET folder_id = folder_id WHERE folder_id = ?");
    q.addBindValue(id);
    q.exec();
}

void Database::updateFolderCover(int id, const QColor &c1, const QColor &c2) {
    QSqlQuery q;
    q.prepare("UPDATE folders SET cover_color1 = ?, cover_color2 = ? WHERE id = ?");
    q.addBindValue(c1.name());
    q.addBindValue(c2.name());
    q.addBindValue(id);
    q.exec();
}

void Database::deleteFolder(int id) {
    QSqlQuery q;
    q.prepare("UPDATE tracks SET folder_id = 0 WHERE folder_id = ?");
    q.addBindValue(id);
    q.exec();
    q.prepare("DELETE FROM folders WHERE id = ?");
    q.addBindValue(id);
    q.exec();
}

void Database::moveTrackToFolder(int trackId, int folderId) {
    QSqlQuery q;
    q.prepare("UPDATE tracks SET folder_id = ? WHERE id = ?");
    q.addBindValue(folderId);
    q.addBindValue(trackId);
    q.exec();
}


Track Database::rowToTrack(const QSqlQuery &q) const {
    Track t;
    t.id         = q.value(0).toInt();
    t.title      = q.value(1).toString();
    t.artist     = q.value(2).toString();
    t.folderId   = q.value(3).toInt();
    t.audioUrl   = QUrl::fromLocalFile(q.value(4).toString());
    t.durationMs = q.value(5).toLongLong();
    t.cover.c1   = QColor(q.value(6).toString());
    t.cover.c2   = QColor(q.value(7).toString());
    t.liked      = q.value(8).toInt() == 1;
    t.addedAt    = q.value(9).toLongLong();
    t.playCount  = q.value(10).toInt();
    t.folder     = q.value(11).toString();
    return t;
}

int Database::insertTrack(const Track &t, int folderId) {
    QSqlQuery q;
    q.prepare(R"(
        INSERT INTO tracks (title, artist, folder_id, file_path, cover_color1, cover_color2)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    q.addBindValue(t.title);
    q.addBindValue(t.artist);
    q.addBindValue(folderId);
    q.addBindValue(t.audioUrl.toLocalFile());
    q.addBindValue(t.cover.c1.name());
    q.addBindValue(t.cover.c2.name());
    q.exec();
    return q.lastInsertId().toInt();
}

void Database::deleteTrack(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM tracks WHERE id = ?");
    q.addBindValue(id);
    q.exec();
}

void Database::setLiked(int id, bool liked) {
    QSqlQuery q;
    q.prepare("UPDATE tracks SET liked = ? WHERE id = ?");
    q.addBindValue(liked ? 1 : 0);
    q.addBindValue(id);
    q.exec();
}

void Database::setDuration(int id, qint64 ms) {
    QSqlQuery q;
    q.prepare("UPDATE tracks SET duration_ms = ? WHERE id = ?");
    q.addBindValue(ms);
    q.addBindValue(id);
    q.exec();
}

void Database::incrementPlayCount(int id) {
    QSqlQuery q;
    q.prepare("UPDATE tracks SET play_count = play_count + 1 WHERE id = ?");
    q.addBindValue(id);
    q.exec();
}

QList<Track> Database::allTracks() {
    QList<Track> result;
    QSqlQuery q(R"(
        SELECT t.id, t.title, t.artist, t.folder_id, t.file_path,
               t.duration_ms, t.cover_color1, t.cover_color2,
               t.liked, t.added_at, t.play_count,
               COALESCE(f.name, '') AS folder_name
        FROM   tracks t
        LEFT JOIN folders f ON f.id = t.folder_id
        ORDER  BY t.added_at DESC
    )");
    while (q.next()) result.append(rowToTrack(q));
    return result;
}


Database::PlaybackState Database::loadState() {
    PlaybackState s;
    QSqlQuery q("SELECT current_track_id, position_ms, volume, shuffle, repeat_mode "
                "FROM playback_state WHERE id = 1");
    if (q.next()) {
        s.trackId = q.value(0).toInt();
        s.posMs   = q.value(1).toLongLong();
        s.volume  = q.value(2).toDouble();
        s.shuffle = q.value(3).toInt() == 1;
        s.repeat  = q.value(4).toInt() == 1;
    }
    return s;
}

void Database::saveState(const PlaybackState &s) {
    QSqlQuery q;
    q.prepare(R"(
        UPDATE playback_state
        SET current_track_id = ?, position_ms = ?, volume = ?, shuffle = ?, repeat_mode = ?
        WHERE id = 1
    )");
    q.addBindValue(s.trackId);
    q.addBindValue(s.posMs);
    q.addBindValue(s.volume);
    q.addBindValue(s.shuffle ? 1 : 0);
    q.addBindValue(s.repeat  ? 1 : 0);
    q.exec();
}
