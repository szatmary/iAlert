#include "qxmpp.h"

#include <QDebug>

#include <gloox/connectiontcpclient.h>

QXmpp::QXmpp(QHostAddress addr, quint16 port, QString username, QString password, QObject *parent)
: QObject(parent)
, m_password(password)
{
    //
    QUuid uuid = QUuid::createUuid(); // TODO this should be stored and reused
    QString clientJidString = username + "@" + addr.toString() + "/Commander/" + uuid.toString().mid(1,36);
    m_clientJid = gloox::JID( clientJidString.toUtf8().constData() );

    QString serverJidString("server@127.0.0.1/NvrCore");
    m_serverJid = gloox::JID( serverJidString.toUtf8().constData() );

    moveToThread( &m_thread );
    m_thread.start();

    QMetaObject::invokeMethod( this, "connectToHost" );
}

QXmpp::~QXmpp()
{
    m_thread.quit();
    m_thread.wait();
}

void QXmpp::connectToHost()
{
    m_glooxClient = QSharedPointer<gloox::Client>( new gloox::Client( m_clientJid, m_password.toUtf8().constData() ) );

    m_glooxClient->registerConnectionListener( this );
    m_glooxClient->registerPresenceHandler( this );

    m_fileTransfer = new gloox::SIProfileFT( m_glooxClient.data(), this );
    m_adHoc        = new gloox::Adhoc( m_glooxClient.data() );

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

void QXmpp::handleFTRequest (const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleFTRequestError (const gloox::IQ &iq, const std::string &sid)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleFTBytestream (gloox::Bytestream *bs)
{
    qDebug() << __FUNCTION__;
}

const std::string QXmpp::handleOOBRequestResult (const gloox::JID &from, const gloox::JID &to, const std::string &sid)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAdhocSupport (const gloox::JID &remote, bool support)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAdhocCommands (const gloox::JID &remote, const gloox::StringMap &commands)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAdhocError (const gloox::JID &remote, const gloox::Error *error)
{
    qDebug() << __FUNCTION__;
}

void QXmpp::handleAdhocExecutionResult (const gloox::JID &remote, const gloox::Adhoc::Command &command)
{
    qDebug() << __FUNCTION__;
}


