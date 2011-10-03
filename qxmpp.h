#ifndef QXmpp_H
#define QXmpp_H

#include <QUuid>
#include <QObject>
#include <QThread>
#include <QHostAddress>
#include <QSharedPointer>
#include <QSocketNotifier>

#include <gloox/client.h>
#include <gloox/siprofileft.h>
#include <gloox/adhochandler.h>
#include <gloox/messagehandler.h>
#include <gloox/presencehandler.h>
#include <gloox/siprofilefthandler.h>
#include <gloox/connectionlistener.h>


// Seperate out xmpp stuff to simplify the Camera code
class QXmpp : public QObject, gloox::ConnectionListener, gloox::PresenceHandler, gloox::SIProfileFTHandler, gloox::AdhocHandler
{
    Q_OBJECT
public:
    explicit QXmpp(QHostAddress addr, quint16 port = 5222, QString username = QString("admin"), QString password = QString("Logitech"), QObject *parent = 0);
    ~QXmpp();

    //gloox::ConnectionListener
    virtual void onConnect() { emit connected(); }
    virtual void onDisconnect (gloox::ConnectionError e) { Q_UNUSED(e); emit disconnected(); } // TODO convert error to QAbstractSocket::Error and emit it
    virtual bool onTLSConnect (const gloox::CertInfo &info) { Q_UNUSED(info); return false; } // We dont support TLS

    //gloox::PresenceHandler
    virtual void handlePresence (const gloox::Presence &presence) { Q_UNUSED(presence); } // Do we care about presence?

    //gloox::SIProfileFTHandler
    virtual void handleFTRequest (const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes);
    virtual void handleFTRequestError (const gloox::IQ &iq, const std::string &sid);
    virtual void handleFTBytestream (gloox::Bytestream *bs);
    virtual const std::string handleOOBRequestResult (const gloox::JID &from, const gloox::JID &to, const std::string &sid);

    //gloox::AdhocHandler
    virtual void handleAdhocSupport (const gloox::JID &remote, bool support);
    virtual void handleAdhocCommands (const gloox::JID &remote, const gloox::StringMap &commands);
    virtual void handleAdhocError (const gloox::JID &remote, const gloox::Error *error);
    virtual void handleAdhocExecutionResult (const gloox::JID &remote, const gloox::Adhoc::Command &command);

signals:
    void connected();
    void disconnected();
public slots:

private slots:
    void connectToHost();
    void readyRead();

private:
    QThread      m_thread;
    QString      m_password;

    QSharedPointer<QSocketNotifier> m_socketNotifier;

    QSharedPointer<gloox::Client> m_glooxClient;
    gloox::JID                    m_clientJid;
    gloox::JID                    m_serverJid;
    gloox::Adhoc                 *m_adHoc;
    gloox::SIProfileFT           *m_fileTransfer;

};
#endif // QXmpp_H
