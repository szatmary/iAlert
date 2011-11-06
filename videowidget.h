#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>

#include <phonon/videoplayer.h>
#include <phonon/seekslider.h>
#include <phonon/audiooutput.h>
#include <phonon/volumeslider.h>
#include <phonon/mediaobject.h>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    void play(QString);
    void stop();
signals:

public slots:
private:
    // TODO delete these when done
    QVBoxLayout          m_layout;
    Phonon::VideoPlayer  m_player;
    Phonon::SeekSlider   m_seek;
    Phonon::VolumeSlider m_volume;
};

#endif // VIDEOWIDGET_H
