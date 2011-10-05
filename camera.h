#ifndef CAMERA_H
#define CAMERA_H

#include <QUrl>
#include <QDir>
#include <QHash>
#include <QString>
#include <QPixmap>
#include <QDateTime>
#include <QSharedPointer>
#include <QDesktopServices>

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

class Recording
{
public:
    QString   m_recordingId;
    QString   m_deviceId;
    QString   m_fileName;
    qint64    m_fileSize;
    QDateTime m_fileTime;
    QString   m_fileHash;

    inline QString filePath() { return path().absoluteFilePath( m_fileName ); }
    inline bool mkpath() { QDir d(path()); return d.exists() ? true : d.mkpath(d.absolutePath()); }
    inline bool exists() { return QFileInfo( filePath() ).exists(); }
    inline QDir path()
    {
        return QDir( QDesktopServices::storageLocation( QDesktopServices::MoviesLocation ) + "/iAlert/" + m_deviceId +
            "/" + m_fileTime.toString("yyyy") + "/" + m_fileTime.toString("MM") + "/" + m_fileTime.toString("dd") );
    }
};

#endif // CAMERA_H
