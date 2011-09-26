#ifndef CAMERA_H
#define CAMERA_H

#include <QUrl>
#include <QHash>
#include <QSharedPointer>

class Camera : public QObject
{
Q_OBJECT
public:

    // What happenes if we go past 32 features?
    enum CameraFeatures
    {
        LiveVideo      = 0x00000001,
        Infrared       = 0x00000002,
        LocalRecording = 0x00000004,
        Audio          = 0x00000008
    };

    Camera();
    virtual ~Camera();
    virtual int features() = 0;
    virtual QUrl liveStream() = 0;
    virtual QString recordings() = 0;
signals:
    void newRecording();
};


typedef QHash<QString,QSharedPointer<Camera> > CameraRoster;

#endif // CAMERA_H
