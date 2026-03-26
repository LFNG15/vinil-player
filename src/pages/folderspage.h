#ifndef FOLDERSPAGE_H
#define FOLDERSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "trackmodel.h"

class FoldersPage : public QWidget {
    Q_OBJECT
public:
    explicit FoldersPage(TrackModel *model, QWidget *parent = nullptr);
    void refresh();

signals:
    void folderSelected(const QString &folderName);

private:
    TrackModel *m_model;
    QVBoxLayout *m_contentLayout;
};

#endif // FOLDERSPAGE_H