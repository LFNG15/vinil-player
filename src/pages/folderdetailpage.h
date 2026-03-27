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
    void refresh(int currentTrackId, bool isPlaying);

signals:
    void playRequested(const Track &track);
    void likeToggled(int id);
    void deleteRequested(int id);
    void navigateBack();

private:
    void showEditDialog();
    void showMoveDialog(int trackId);

    TrackModel *m_model;
    QString m_folderName;
    int m_folderId = 0;
    QVBoxLayout *m_contentLayout;
};

#endif // FOLDERDETAILPAGE_H
