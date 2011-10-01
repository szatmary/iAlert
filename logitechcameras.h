#ifndef LOGITECHCAMERAS_H
#define LOGITECHCAMERAS_H

#include "camera.h"

#include <QUuid>
#include <QThread>
#include <QStringList>
#include <QHostAddress>
#include <QSharedPointer>
#include <QSocketNotifier>
#include <QTemporaryFile>
#include <QCryptographicHash>

#include <gloox/client.h>
#include <gloox/siprofileft.h>
#include <gloox/adhochandler.h>
#include <gloox/messagehandler.h>
#include <gloox/presencehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/siprofilefthandler.h>
#include <gloox/bytestreamdatahandler.h>
#include <gloox/message.h>

class LogitechHandler
{
public:
    virtual void handleNewRecording(QString id)=0;
};

class LogitechRecordingSearchResult : public gloox::StanzaExtension
{
public:
    LogitechRecordingSearchResult(LogitechHandler *handler);
    LogitechRecordingSearchResult(const gloox::Tag *tag, LogitechHandler *handler);
    LogitechRecordingSearchResult(const LogitechRecordingSearchResult &that);
    virtual const std::string &filterString () const;
    virtual StanzaExtension *newInstance (const gloox::Tag *tag) const;
    virtual gloox::Tag *tag () const;
    virtual StanzaExtension *clone () const;
private:
    LogitechHandler *m_handler;
};

// TODO make this interuptable somehow
class LogitechBytestreamDataHandler : public QThread, gloox::BytestreamDataHandler
{
Q_OBJECT
public:
    LogitechBytestreamDataHandler(QString cameraName, const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes);
    void beginTransfer(gloox::Bytestream *bs);

    virtual void handleBytestreamData (gloox::Bytestream *bs, const std::string &data);
    virtual void handleBytestreamError (gloox::Bytestream *bs, const gloox::IQ &iq);
    virtual void handleBytestreamOpen (gloox::Bytestream *bs);
    virtual void handleBytestreamClose (gloox::Bytestream *bs);
signals:
    void downloadComplete(QString,bool);
protected:
    virtual void run();
private:
    QString cameraName;
    QTemporaryFile file;
    QCryptographicHash fileHash;

    gloox::Bytestream *bs;

    const gloox::JID from;
    const gloox::JID to;
    const std::string sid;
    const std::string name;
    long size;
    const std::string hash;
    const std::string date;
    const std::string mimetype;
    const std::string desc;
    int stypes;

};

class Logitech700eCamera : public Camera, LogitechHandler, gloox::ConnectionListener, gloox::PresenceHandler, gloox::SIProfileFTHandler, gloox::AdhocHandler, gloox::MessageHandler
{
Q_OBJECT
public:
    Logitech700eCamera(QHostAddress addr, QString username = "admin", QString password = "Logitech");
    ~Logitech700eCamera();
    virtual int features();
    QString recordings();
    QUrl liveStream();

private:
    virtual void onConnect();
    virtual void onDisconnect(gloox::ConnectionError e);
    virtual bool onTLSConnect(const gloox::CertInfo& info);

    virtual void handlePresence(const gloox::Presence &presence );

    virtual void handleFTRequest (const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes);
    virtual void handleFTRequestError (const gloox::IQ &iq, const std::string &sid);
    virtual void handleFTBytestream (gloox::Bytestream *bs);
    virtual const std::string handleOOBRequestResult (const gloox::JID &from, const gloox::JID &to, const std::string &sid);

    virtual void handleAdhocSupport (const gloox::JID &remote, bool support);
    virtual void handleAdhocCommands (const gloox::JID &remote, const gloox::StringMap &commands);
    virtual void handleAdhocError (const gloox::JID &remote, const gloox::Error *error);
    virtual void handleAdhocExecutionResult (const gloox::JID &remote, const gloox::Adhoc::Command &command);

    // For pubsub
    virtual void handleMessage (const gloox::Message &msg, gloox::MessageSession *session = 0);

    virtual void handleNewRecording(QString id);
    void downloadFile(QString id);
    void basicGet();
    QString cameraName() { return QString(instanceName + " (" + instanceId + ")"); }
signals:
    void connected(bool);
    void disconnected();
public slots:
    void downloadComplete(QString, bool);
private slots:
    void readyRead();
    void Logitech700eCameraImpl(QString username, QString password);
private:
    QUuid           uuid; // THIS should be static!
    QThread         thread;
    QHostAddress    hostAddress;


    // camera info
    QString instanceId;
    QString instanceName;
    QString instanceType;
    QString softwareVersion;
    QString softwareVersionReleaseDate;
    QString softwareInstallDate;
    QString operatingSystemFullName;
    QString operatingSystemVersion;
    QString systemUpTime;


    QSharedPointer<gloox::Client>      client;

    QSharedPointer<QSocketNotifier>    socketNotifier;
    gloox::Adhoc *adHoc;
    gloox::SIProfileFT *fileTransfer;

    QHash< QString, QSharedPointer<LogitechBytestreamDataHandler> > transfers;
    QStringList pendingTransfers;
};

#endif // LOGITECHCAMERAS_H
