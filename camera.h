#ifndef CAMERA_H
#define CAMERA_H

#include <QUrl>
#include <QHash>
#include <QPixmap>
#include <QDateTime>
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

    Camera(QString id, QObject *parent = 0);
    virtual ~Camera();
    QString id() { return m_id; }
    virtual int features() = 0;
    virtual QUrl liveStream() = 0;
    virtual QString recordings() = 0;

    virtual QDateTime lastRecordingDateTime() { return QDateTime(); }
    virtual QPixmap   lastRecordingSnapshot() { return QPixmap(); }

signals:
    void newRecording();
    void recordingEnded();
private:
    QString m_id;
};


typedef QHash<QString,QSharedPointer<Camera> > CameraRoster;

#endif // CAMERA_H
