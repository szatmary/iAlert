#ifndef LOGITECHCAMERAS_H
#define LOGITECHCAMERAS_H

#include "qxmpp.h"
#include "camera.h"

//class LogitechHandler
//{
//public:
//    virtual void handleNewRecording(QString id)=0;
//};

//class LogitechRecordingSearchResult : public gloox::StanzaExtension
//{
//public:
//    LogitechRecordingSearchResult(LogitechHandler *handler);
//    LogitechRecordingSearchResult(const gloox::Tag *tag, LogitechHandler *handler);
//    LogitechRecordingSearchResult(const LogitechRecordingSearchResult &that);
//    virtual const std::string &filterString () const;
//    virtual StanzaExtension *newInstance (const gloox::Tag *tag) const;
//    virtual gloox::Tag *tag () const;
//    virtual StanzaExtension *clone () const;
//private:
//    LogitechHandler *m_handler;
//};

//// TODO make this interuptable somehow
//class LogitechBytestreamDataHandler : public QThread, gloox::BytestreamDataHandler
//{
//Q_OBJECT
//public:
//    LogitechBytestreamDataHandler(QString cameraName, const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes);
//    void beginTransfer(gloox::Bytestream *bs);

//    virtual void handleBytestreamData (gloox::Bytestream *bs, const std::string &data);
//    virtual void handleBytestreamError (gloox::Bytestream *bs, const gloox::IQ &iq);
//    virtual void handleBytestreamOpen (gloox::Bytestream *bs);
//    virtual void handleBytestreamClose (gloox::Bytestream *bs);
//signals:
//    void downloadComplete(QString,bool);
//protected:
//    virtual void run();
//private:
//    QString cameraName;
//    QTemporaryFile file;
//    QCryptographicHash fileHash;

//    gloox::Bytestream *bs;

//    const gloox::JID from;
//    const gloox::JID to;
//    const std::string sid;
//    const std::string name;
//    long size;
//    const std::string hash;
//    const std::string date;
//    const std::string mimetype;
//    const std::string desc;
//    int stypes;

//};



class Logitech700eCamera : public Camera
{
Q_OBJECT
public:
    Logitech700eCamera(QString id, QHostAddress addr, QString username = "admin", QString password = "Logitech");
    ~Logitech700eCamera();
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
