#include "qxmpp.h"

#include "registry.h"

#include <QDebug>
#include <QMetaType>

#include <gloox/message.h>
#include <gloox/socks5bytestream.h>
#include <gloox/connectiontcpclient.h>

QXmppFileTransfer::QXmppFileTransfer(QString from, QString to, QString sid, QString name, qint64 size, QString hash, QDateTime date, QString mimetype, QString desc)
: m_from(from)
, m_to(to)
, m_sid(sid)
, m_name(name)
, m_size(size)
, m_hash(hash.toLower())
, m_date(date)
, m_mimetype(mimetype)
, m_desc(desc)
, m_fileHash( QCryptographicHash::Md5 )
{
    m_timer.start( XMPPFILETRANSFER_TIMEOUT );
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(timeout()));
}

void QXmppFileTransfer::beginTransfer(gloox::Bytestream *bs)
{
    m_bs = bs;
    if( gloox::Bytestream::S5B != bs->type() )
    {
        qDebug() << "Only socks 5 bytes streams are suported!";
        emit finished(false);
        return;
    }

    bs->registerBytestreamDataHandler( this );
    bs->connect();
    int sock = static_cast<gloox::ConnectionTCPClient*>( ((gloox::SOCKS5Bytestream*)bs)->connectionImpl() )->socket();
    m_socketNotifier = QSharedPointer<QSocketNotifier>( new QSocketNotifier(sock,QSocketNotifier::Read) );
    connect(m_socketNotifier.data(),SIGNAL(activated(int)),this,SLOT(readyRead()));
    m_socketNotifier->setEnabled( true );
    readyRead(); // Just in case something came in while we were setting stuff up
}

void QXmppFileTransfer::readyRead()
{
    m_timer.start( XMPPFILETRANSFER_TIMEOUT ); // time out broken transfers
    m_bs->recv(1);
}

void QXmppFileTransfer::timeout()
{
    qDebug() << "Transfer timedout";
    m_socketNotifier->setEnabled( false );
    emit finished(false);
}

void QXmppFileTransfer::handleBytestreamData (gloox::Bytestream *bs, const std::string &data)
{
    m_fileHash.addData( data.c_str(), data.length() );
    m_file.write( data.c_str(), data.length() );
}

void QXmppFileTransfer::handleBytestreamError (gloox::Bytestream *bs, const gloox::IQ &iq)
{
    m_socketNotifier->setEnabled( false );
    emit finished(false);
}

void QXmppFileTransfer::handleBytestreamOpen (gloox::Bytestream *bs)
{
    m_file.open();
}

void QXmppFileTransfer::handleBytestreamClose (gloox::Bytestream *bs)
{
    m_socketNotifier->setEnabled( false );
    if( m_file.size() != m_size || m_fileHash.result().toHex().toLower() != m_hash.toLower() )
    {
        qDebug() << "File hash mismatch";
        emit finished(false);
        m_file.close();
        return;
    }

    emit finished(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
QXmpp::QXmpp(QHostAddress addr, quint16 port, QString username, QString password, QObject *parent)
: QObject(parent)
, nextStanzeExtType(gloox::ExtUser + 1)
{
    // The UUID should be set befor we get here
    QString uuid = Registry::getValue("uuid").toString();
    QString clientJidString = username + "@" + addr.toString() + "/Commander/" + uuid.mid(1,36);
    m_clientJid = gloox::JID( clientJidString.toUtf8().constData() );

    QString serverJidString("server@127.0.0.1/NvrCore");
    m_serverJid = gloox::JID( serverJidString.toUtf8().constData() );

    m_glooxClient = QSharedPointer<gloox::Client>( new gloox::Client( m_clientJid, password.toUtf8().constData() ) );

    m_glooxClient->registerConnectionListener( this );
    m_glooxClient->registerPresenceHandler( this );
    m_glooxClient->registerMessageHandler( this );

    // I dont understand why, but this has to be here for handleMessage to be called!
    m_glooxClient->registerStanzaExtension( new gloox::PubSub::Event(0) );


    m_fileTransfer  = new gloox::SIProfileFT( m_glooxClient.data(), this );
    m_adHoc         = new gloox::Adhoc( m_glooxClient.data() );
    m_pubSubManager = new gloox::PubSub::Manager( m_glooxClient.data() );


    moveToThread( &m_thread );
    m_thread.start();

    qRegisterMetaType< QSharedPointer<gloox::Adhoc::Command> >("QSharedPointer<gloox::Adhoc::Command>"); // Is the the correct spot to do this?
    qRegisterMetaType< QSharedPointer<gloox::PubSub::Event> >("QSharedPointer<gloox::PubSub::Event>"); // Is the the correct spot to do this?
    qRegisterMetaType< QSharedPointer<QXmppFileTransfer> >("QSharedPointer<QXmppFileTransfer>"); // Is the the correct spot to do this?
    qRegisterMetaType< QSharedPointer<gloox::Tag> >("QSharedPointer<gloox::Tag>"); // Is the the correct spot to do this?

    QMetaObject::invokeMethod( this, "connectToHost" );
}

QXmpp::~QXmpp()
{
    m_thread.quit();
    m_thread.wait();
}

void QXmpp::sendCommand(gloox::Adhoc::Command *cmd)
{
    m_adHoc->execute( m_serverJid, cmd, this);
}

void QXmpp::subscribe(QString node)
{
    m_pubSubManager->subscribe( m_serverJid, node.toUtf8().constData(), this);
}

class QXmppCustomIq : public gloox::IQ
{
private:
    gloox::Tag *m_tag;
public:
    QXmppCustomIq(gloox::IQ::IqType type, gloox::JID &to, gloox::Tag *tag, const std::string &id)
    : gloox::IQ(type,to,id), m_tag(tag) {}

    ~QXmppCustomIq() { delete m_tag; }

    gloox::Tag *tag() const
    {
        gloox::Tag *t = gloox::IQ::tag();
        t->addChild( m_tag->clone() );
        return t;
    }
};

void QXmpp::sendCustomIq(gloox::IQ::IqType type, gloox::Tag *tag)
{
    QXmppCustomIq iq(type, m_serverJid, tag, m_glooxClient->getID() );
    m_glooxClient->send( iq );
}

void QXmpp::connectToHost()
{
    if ( m_glooxClient->connect( false ) )
    {
        int sock = static_cast<gloox::ConnectionTCPClient*>( m_glooxClient->connectionImpl() )->socket();
        m_socketNotifier = QSharedPointer<QSocketNotifier>( new QSocketNotifier(sock,QSocketNotifier::Read) );
        connect(m_socketNotifier.data(),SIGNAL(activated(int)),this,SLOT(readyRead()));
        m_socketNotifier->setEnabled( true );
        readyRead(); // Just in case something came in while we were setting stuff up
    } else {
        // will this cause a double disconnect emit?
        emit disconnected();
    }
}

void QXmpp::readyRead()
{
    m_glooxClient->recv(1);
}

void QXmpp::registerCustomStanza(QString filterString)
{
    m_glooxClient->registerStanzaExtension( new QXmppCustomStanza( filterString.toUtf8().constData(), nextStanzeExtType, this ) );
    ++nextStanzeExtType;
}

void QXmpp::handleCustom(std::string filterString, int type, const gloox::Tag *tag)
{
    Q_UNUSED(filterString); Q_UNUSED(type);
    if( tag ) { emit customStanza( QSharedPointer<gloox::Tag>( tag->clone() ) ); }
}

void QXmpp::handleFTRequest (const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes)
{
    QXmppFileTransfer *ft = new QXmppFileTransfer( from.full().c_str(), to.full().c_str(), sid.c_str(), name.c_str(), size, hash.c_str(), QDateTime::fromString(date.c_str(),Qt::ISODate), mimetype.c_str(), desc.c_str() );
    connect(ft,SIGNAL(finished(bool)),this,SLOT(transferDone(bool)));

    // TODO check stypes and reject any non socks transfers
    m_activeTransfers.insert( sid.c_str(), QSharedPointer<QXmppFileTransfer>( ft ) );
    m_fileTransfer->acceptFT( from, sid ); // We will accept all transfer reuests for now
}

void QXmpp::handleFTRequestError (const gloox::IQ &iq, const std::string &sid)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleFTBytestream (gloox::Bytestream *bs)
{
    QHash< QString,QSharedPointer<QXmppFileTransfer> >::iterator i = m_activeTransfers.find( bs->sid().c_str() );
    if ( i != m_activeTransfers.end() )
    {
        (*i)->beginTransfer(bs);
    }
}

void QXmpp::transferDone(bool ok)
{
    // remove it from the active transfers
    QXmppFileTransfer *transfer = (QXmppFileTransfer*)sender();
    QHash< QString,QSharedPointer<QXmppFileTransfer> >::iterator i = m_activeTransfers.find( transfer->sid() );
    if ( i != m_activeTransfers.end() )
    {
        emit transferComplete( (*i), ok );
        m_activeTransfers.erase( i );
    }
}

const std::string QXmpp::handleOOBRequestResult (const gloox::JID &from, const gloox::JID &to, const std::string &sid)
{
    qDebug() << __FUNCTION__;
    return "";
}

void QXmpp::handleAdhocSupport (const gloox::JID &remote, bool support, int context)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAdhocCommands (const gloox::JID &remote, const gloox::StringMap &commands, int context)
{
    for(gloox::StringMap::const_iterator i = commands.begin() ; i != commands.end() ; ++i )
    {
        qDebug() << i->first.c_str() << i->second.c_str();
    }
}

void QXmpp::handleAdhocError (const gloox::JID &remote, const gloox::Error *error, int context)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAdhocExecutionResult (const gloox::JID &remote, const gloox::Adhoc::Command &command, int context)
{
    QSharedPointer<gloox::Adhoc::Command> cmd( (gloox::Adhoc::Command*)command.clone() );
    emit commandResult( cmd );
}

void QXmpp::handleMessage (const gloox::Message &msg, gloox::MessageSession *session)
{
    const gloox::PubSub::Event* pse = msg.findExtension<gloox::PubSub::Event>( gloox::ExtPubSubEvent );
    if( pse )
    {
        QSharedPointer<gloox::PubSub::Event> event( (gloox::PubSub::Event*)pse->clone() );
        emit publishEvent( event );
    }
}

void QXmpp::handleItem (const gloox::JID &service, const std::string &node, const gloox::Tag *entry)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleItems (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleItemPublication (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleItemDeletion (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleSubscriptionResult (const std::string &id, const gloox::JID &service, const std::string &node, const std::string &sid, const gloox::JID &jid, const gloox::PubSub::SubscriptionType subType, const gloox::Error *error)
{
    // Check for errors
    // qDebug() << __FUNCTION__;
}

void QXmpp::handleUnsubscriptionResult (const std::string &id, const gloox::JID &service, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleSubscriptionOptions (const std::string &id, const gloox::JID &service, const gloox::JID &jid, const std::string &node, const gloox::DataForm *options, const std::string& sid, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleSubscriptionOptionsResult (const std::string &id, const gloox::JID &service, const gloox::JID &jid, const std::string &node, const std::string& sid, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleSubscribers (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::SubscriberList *list, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleSubscribersResult (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::SubscriberList *list, const gloox::Error *error)

{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAffiliates (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::AffiliateList *list, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAffiliatesResult (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::AffiliateList *list, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleNodeConfig (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::DataForm *config, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleNodeConfigResult (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleNodeCreation (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleNodeDeletion (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleNodePurge (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleSubscriptions (const std::string &id, const gloox::JID &service, const gloox::PubSub::SubscriptionMap &subMap, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAffiliations (const std::string &id, const gloox::JID &service, const gloox::PubSub::AffiliationMap &affMap, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleDefaultNodeConfig (const std::string &id, const gloox::JID &service, const gloox::DataForm *config, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}


