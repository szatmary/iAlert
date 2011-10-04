#ifndef QXmpp_H
#define QXmpp_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QHostAddress>
#include <QTemporaryFile>
#include <QSharedPointer>
#include <QSocketNotifier>
#include <QCryptographicHash>

#include <gloox/client.h>
#include <gloox/siprofileft.h>
#include <gloox/pubsubevent.h>
#include <gloox/adhochandler.h>
#include <gloox/pubsubmanager.h>
#include <gloox/messagehandler.h>
#include <gloox/presencehandler.h>
#include <gloox/siprofilefthandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/pubsubresulthandler.h>
#include <gloox/bytestreamdatahandler.h>

#define XMPPFILETRANSFER_TIMEOUT 30000 // 30 seconds
class XMppFileTransfer : public QObject, gloox::BytestreamDataHandler
{
Q_OBJECT
public:
    XMppFileTransfer(QString from, QString to, QString sid, QString name, qint64 size, QString hash, QDateTime date, QString mimetype, QString desc);
    void beginTransfer(gloox::Bytestream *bs);

    QString   from() { return m_from; }
    QString   to() { return m_to; }
    QString   sid() { return m_sid; }
    QString   name() { return m_name; }
    qint64    size() { return m_size; }
    QString   hash() { return m_hash; }
    QDateTime date() { return m_date; }
    QString   mimetype() { return m_mimetype; }
    QString   desc() { return m_desc; }
    bool      moveFile(QString name) { m_file.setAutoRemove( false ); return m_file.rename( name ); }

    //gloox::BytestreamDataHandler
    virtual void handleBytestreamData (gloox::Bytestream *bs, const std::string &data);
    virtual void handleBytestreamError (gloox::Bytestream *bs, const gloox::IQ &iq);
    virtual void handleBytestreamOpen (gloox::Bytestream *bs);
    virtual void handleBytestreamClose (gloox::Bytestream *bs);
signals:
    void finished(bool);
private slots:
    void readyRead();
    void timeout();
private:
    QString   m_from;
    QString   m_to;
    QString   m_sid;
    QString   m_name;
    qint64    m_size;
    QString   m_hash;
    QDateTime m_date;
    QString   m_mimetype;
    QString   m_desc;

    gloox::Bytestream *m_bs;
    QSharedPointer<QSocketNotifier> m_socketNotifier;

    QTimer             m_timer;
    QTemporaryFile     m_file;
    QCryptographicHash m_fileHash;
};


// Seperate out xmpp stuff to simplify the Camera code
class QXmpp : public QObject, gloox::ConnectionListener, gloox::PresenceHandler, gloox::SIProfileFTHandler, gloox::AdhocHandler, gloox::MessageHandler, gloox::PubSub::ResultHandler
{
    Q_OBJECT
public:
    explicit QXmpp(QHostAddress addr, quint16 port = 5222, QString username = QString("admin"), QString password = QString("Logitech"), QObject *parent = 0);
    ~QXmpp();

    void sendCommand(gloox::Adhoc::Command *cmd); // This will take ownership of cmd
    void sendCustomIq(gloox::IQ::IqType type, gloox::Tag *tag);// This will take ownership of tag
    void subscribe(QString node);

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

    //gloox::MessageHandler
    virtual void handleMessage (const gloox::Message &msg, gloox::MessageSession *session=0);

    //gloox::PubSub::ResultHandler
    virtual void handleItem (const gloox::JID &service, const std::string &node, const gloox::Tag *entry);
    virtual void handleItems (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error=0);
    virtual void handleItemPublication (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error=0);
    virtual void handleItemDeletion (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error=0);
    virtual void handleSubscriptionResult (const std::string &id, const gloox::JID &service, const std::string &node, const std::string &sid, const gloox::JID &jid, const gloox::PubSub::SubscriptionType subType, const gloox::Error *error=0);
    virtual void handleUnsubscriptionResult (const std::string &id, const gloox::JID &service, const gloox::Error *error=0);
    virtual void handleSubscriptionOptions (const std::string &id, const gloox::JID &service, const gloox::JID &jid, const std::string &node, const gloox::DataForm *options, const gloox::Error *error=0);
    virtual void handleSubscriptionOptionsResult (const std::string &id, const gloox::JID &service, const gloox::JID &jid, const std::string &node, const gloox::Error *error=0);
    virtual void handleSubscribers (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::SubscriberList *list, const gloox::Error *error=0);
    virtual void handleSubscribersResult (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::SubscriberList *list, const gloox::Error *error=0);
    virtual void handleAffiliates (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::AffiliateList *list, const gloox::Error *error=0);
    virtual void handleAffiliatesResult (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::AffiliateList *list, const gloox::Error *error=0);
    virtual void handleNodeConfig (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::DataForm *config, const gloox::Error *error=0);
    virtual void handleNodeConfigResult (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error=0);
    virtual void handleNodeCreation (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error=0);
    virtual void handleNodeDeletion (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error=0);
    virtual void handleNodePurge (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error=0);
    virtual void handleSubscriptions (const std::string &id, const gloox::JID &service, const gloox::PubSub::SubscriptionMap &subMap, const gloox::Error *error=0);
    virtual void handleAffiliations (const std::string &id, const gloox::JID &service, const gloox::PubSub::AffiliationMap &affMap, const gloox::Error *error=0);
    virtual void handleDefaultNodeConfig (const std::string &id, const gloox::JID &service, const gloox::DataForm *config, const gloox::Error *error=0);

signals:
    void connected();
    void disconnected();
    void commandResult(QSharedPointer<gloox::Adhoc::Command>);
    void publishEvent(QSharedPointer<gloox::PubSub::Event>);
    void transferComplete(QSharedPointer<XMppFileTransfer>,bool);
public slots:

private slots:
    void connectToHost();
    void readyRead();
    void transferDone(bool);

private:
    QThread m_thread;

    QSharedPointer<QSocketNotifier> m_socketNotifier;

    QSharedPointer<gloox::Client> m_glooxClient;
    gloox::JID                    m_clientJid;
    gloox::JID                    m_serverJid;
    gloox::Adhoc                 *m_adHoc;
    gloox::SIProfileFT           *m_fileTransfer;
    gloox::PubSub::Manager       *m_pubSubManager;


    QHash< QString,QSharedPointer<XMppFileTransfer> > m_activeTransfers;
};
#endif // QXmpp_H
