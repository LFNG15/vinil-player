#ifndef FOLDERDETAILPAGE_H
#define FOLDERDETAILPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include "trackmodel.h"

class FolderDetailPage : public QWidget {
    Q_OBJECT
public:
    explicit FolderDetailPage(TrackModel *model, QWidget *parent = nullptr);
    void setFolder(const QString &folderName);
    void refresh(const QString &currentTrackId, bool isPlaying);

signals:
    void playRequested(const Track &track);
    void likeToggled(const QString &id);
    void deleteRequested(const QString &id);
    void navigateBack();

private:
    TrackModel *m_model;
    QString m_folderName;
    QVBoxLayout *m_contentLayout;
};

#endif // FOLDERDETAILPAGE_H