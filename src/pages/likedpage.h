#ifndef LIKEDPAGE_H
#define LIKEDPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include "trackmodel.h"

class LikedPage : public QWidget {
    Q_OBJECT
public:
    explicit LikedPage(TrackModel *model, QWidget *parent = nullptr);
    void refresh(int currentTrackId, bool isPlaying);

signals:
    void playRequested(const Track &track);
    void likeToggled(int id);
    void navigateBack();

private:
    TrackModel *m_model;
    QVBoxLayout *m_contentLayout;
};

#endif // LIKEDPAGE_H