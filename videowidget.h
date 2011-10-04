#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSharedPointer>
#include <phonon/videoplayer.h>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    void play(QString);
signals:

public slots:
private:
    // TODO delete these when done
    QVBoxLayout         *layout;
    Phonon::VideoPlayer *player;
};

#endif // VIDEOWIDGET_H
