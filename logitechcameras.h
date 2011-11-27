#ifndef LOGITECHCAMERAS_H
#define LOGITECHCAMERAS_H

#include "qxmpp.h"
#include "camera.h"

class Logitech700eCamera : public Camera
{
Q_OBJECT
public:
    Logitech700eCamera(QHostAddress addr, QString username = "admin", QString password = "Logitech");
    ~Logitech700eCamera();
    virtual QString id();
    virtual int features();
    QString recordings();
    QUrl liveStream();

    void commandNvrBasicGet();
    void subscribeRecordingEnded();
    void queueRecordingTransfer(QString);

    void getRecordingList();

    virtual QDateTime lastRecordingDateTime() { return m_lastEventDateTime; }
    virtual QPixmap   lastRecordingSnapshot() { return m_lastEventSnapshot; }
signals:
    void newRecording(QString);
public slots:
    void xmppConnected();
    void xmppDisconnected();
    void xmppCommandResult(QSharedPointer<gloox::Adhoc::Command>);
    void xmppPublishEvent(QSharedPointer<gloox::PubSub::Event> event);
    void xmppTransferComplete(QSharedPointer<QXmppFileTransfer>,bool);
    void xmppCustomStanza(QSharedPointer<gloox::Tag>);
private slots:
private:
    void doRecordingTransfer(QString);

    QHostAddress m_addr;
    QSharedPointer<QXmpp> m_xmppClient;

    // camera info
    QString m_instanceId;
    QString m_instanceName;
    QString m_instanceType;
    QString m_softwareVersion;
    QString m_softwareVersionReleaseDate;
    QString m_softwareInstallDate;
    QString m_operatingSystemFullName;
    QString m_operatingSystemVersion;
    QString m_systemUpTime;

    QDateTime   m_lastEventDateTime;
    QPixmap     m_lastEventSnapshot;
    QStringList m_downloadQueue;
};

#endif // LOGITECHCAMERAS_H
