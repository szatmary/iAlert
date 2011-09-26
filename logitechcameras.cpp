#include "logitechcameras.h"

#include <QFileInfo>
#include <QSocketNotifier>
#include <QDesktopServices>

#include <gloox/connectiontcpclient.h>

#define LogitechRecordingSearchResultExtType (gloox::ExtUser + 1)
LogitechRecordingSearchResult::LogitechRecordingSearchResult(LogitechHandler *handler)
: StanzaExtension( LogitechRecordingSearchResultExtType )
, m_handler(handler)
{}

LogitechRecordingSearchResult::LogitechRecordingSearchResult(const gloox::Tag *tag, LogitechHandler *handler)
: StanzaExtension(LogitechRecordingSearchResultExtType)
, m_handler(handler)
{
    if ( ! tag )
        return;

    // TODO check all tags for NULL
    gloox::Tag *set   = tag->findChild("set");
    gloox::Tag *count = set->findChild("count");

    gloox::Tag *results = tag->findChild("Results");
    const gloox::TagList children = results->children();
    qDebug() << "Received" << children.size() << "of" << count->cdata().c_str();
    for( gloox::TagList::const_iterator i = children.begin() ; i != children.end() ; ++i )
    {
        // TODO sanitycheck Name is 'Item'
//        qDebug() << "Id:" << (*i)->findAttribute("Id").c_str() << "StartTime:" << (*i)->findAttribute("StartTime").c_str() << "Duration:" << (*i)->findAttribute("Duration").c_str();
        handler->handleNewRecording( QString( (*i)->findAttribute("Id").c_str() ) );
    }
}

LogitechRecordingSearchResult::LogitechRecordingSearchResult(const LogitechRecordingSearchResult &that)
: StanzaExtension(LogitechRecordingSearchResultExtType)
, m_handler(that.m_handler)
{
}

const std::string &LogitechRecordingSearchResult::filterString () const
{
    static const std::string filter = "/iq/Query[@xmlns='urn:logitech-com:logitech-alert:device:media:recording:search']";
    return filter;
}

gloox::StanzaExtension *LogitechRecordingSearchResult::newInstance (const gloox::Tag *tag) const
{
    return new LogitechRecordingSearchResult(tag,m_handler);
}

gloox::Tag *LogitechRecordingSearchResult::tag () const
{
    return 0; // TODO, But I dont thing we really need it
}

gloox::StanzaExtension *LogitechRecordingSearchResult::clone () const
{
    return new LogitechRecordingSearchResult( *this );
}

class LogitechRecordingSearchIQ : public gloox::IQ
{
public:
    LogitechRecordingSearchIQ( gloox::JID &to, const std::string id = gloox::EmptyString)
        : gloox::IQ(Get,to,id)
    {}

    gloox::Tag *tag() const
    {
        gloox::Tag *t = gloox::IQ::tag();

        std::string remoteMac = "00-12-AB-1B-13-63";
        std::string MinInclusiveStr = "2011-09-14T14:38:01.990273-04:00";

        gloox::Tag *Query = new gloox::Tag(t, "Query");
        Query->setXmlns("urn:logitech-com:logitech-alert:device:media:recording:search");

        gloox::Tag *set = new gloox::Tag(Query,"set");
        set->setXmlns("http://jabber.org/protocol/rsm");

        gloox::Tag *index = new gloox::Tag(set,"index","0");
        gloox::Tag *max = new gloox::Tag(set,"max","100000");

        gloox::Tag *Fields = new gloox::Tag(Query,"Fields");
        gloox::Tag *Field1 = new gloox::Tag(Fields,"Field", "name", "Id");
        gloox::Tag *Field2 = new gloox::Tag(Fields,"Field", "name", "StartTime");
        gloox::Tag *Field3 = new gloox::Tag(Fields,"Field", "name", "Duration");

        gloox::Tag *Criteria = new gloox::Tag(Query,"Criteria");
        gloox::Tag *Field4 = new gloox::Tag(Criteria,"Field", "name", "DeviceId");
        Field4->addAttribute("type","list-multi");
        gloox::Tag *Value = new gloox::Tag(Field4,"Value", remoteMac);

        gloox::Tag *Field5 = new gloox::Tag(Criteria,"Field", "name", "StartTime");
        Field5->addAttribute("type","dateTime");

        gloox::Tag *Sort = new gloox::Tag(Field5, "Sort");
        Sort->addAttribute("allowed","1");
        Sort->addAttribute("order","1");
        Sort->addAttribute("direction","ASC");

//        Tag *MinInclusive = new Tag(Field5,"MinInclusive", MinInclusiveStr);
        gloox::Tag *MinInclusive = new gloox::Tag(Field5,"MinInclusive");
        gloox::Tag *MaxInclusive = new gloox::Tag(Field5,"MaxInclusive");

        qDebug() << t->xml().c_str();
        return t;
    }
};

class LogitechRecordingTransferRequestIQ : public gloox::IQ
{
public:
    LogitechRecordingTransferRequestIQ( gloox::JID &to, QString recordingId, const std::string id = gloox::EmptyString)
    : gloox::IQ(Set,to,id)
    , m_recordingId(recordingId)
    {}

    gloox::Tag *tag() const
    {
        gloox::Tag *t = gloox::IQ::tag();

        gloox::Tag *Transfer = new gloox::Tag(t, "Transfer");
        Transfer->setXmlns("urn:logitech-com:logitech-alert:device:media:recording:file");

        gloox::Tag *MediaRecording = new gloox::Tag(Transfer, "MediaRecording");
        MediaRecording->addAttribute("id",m_recordingId.toUtf8().constData());

        gloox::Tag *FileTransfer = new gloox::Tag(Transfer, "FileTransfer");
        FileTransfer->setXmlns("urn:logitech-com:logitech-alert:file-transfer");

        gloox::Tag *TransferMethod = new gloox::Tag(FileTransfer, "TransferMethod");
        TransferMethod->addAttribute("type","http://jabber.org/protocol/bytestreams");

        return t;
    }
private:
    QString m_recordingId;
};

LogitechBytestreamDataHandler::LogitechBytestreamDataHandler(const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes)
: QThread()
, fileHash( QCryptographicHash::Md5 )
, gloox::BytestreamDataHandler()
, from(from)
, to(to)
, sid(sid)
, name(name)
, size(size)
, hash(hash)
, date(date)
, mimetype(mimetype)
, desc(desc)
, stypes(stypes)
{
    qDebug() << __FUNCTION__;
}

void LogitechBytestreamDataHandler::beginTransfer(gloox::Bytestream *bs)
{
    qDebug() << __FUNCTION__;
    this->bs = bs;
    bs->registerBytestreamDataHandler( this );
    start();
}

void LogitechBytestreamDataHandler::run()
{
    qDebug() << __FUNCTION__;
    // TODO make this interuptable somehow
    gloox::ConnectionError error = gloox::ConnNoError;
    bs->connect();
    while( gloox::ConnNoError == error )
    {
        // TODO we need to put a timeout in here
        error = bs->recv();
    }
}

void LogitechBytestreamDataHandler::handleBytestreamData (gloox::Bytestream *bs, const std::string &data)
{
    file.write( data.c_str(), data.size() ); // will this handle binary data ok?
    fileHash.addData( data.c_str(), data.size() );
}

void LogitechBytestreamDataHandler::handleBytestreamError (gloox::Bytestream *bs, const gloox::IQ &iq)
{
    qDebug() << __FUNCTION__;
    file.close();
    emit downloadComplete( QString(sid.c_str()), false );
    // error, juet let the file delete itself
}

void LogitechBytestreamDataHandler::handleBytestreamOpen (gloox::Bytestream *bs)
{
    qDebug() << __FUNCTION__ << QFileInfo( file ).filePath();
    file.open();
}

void LogitechBytestreamDataHandler::handleBytestreamClose (gloox::Bytestream *bs)
{
    if ( QString( hash.c_str() ).toLower() != fileHash.result().toHex().toLower() )
    {
        qDebug() << "Hash mismatch" << QString( hash.c_str() ).toLower() << "!=" << fileHash.result().toHex().toLower();
        return;
    }

    QString newName = QDesktopServices::storageLocation( QDesktopServices::MoviesLocation ) + QString("/") + QString( name.c_str() );

    // TODO set file metadata
    file.setAutoRemove( false );
    file.rename( newName );
    file.close();
    qDebug() << "moved to" << newName;
    emit downloadComplete( QString(sid.c_str()), true );
}

Logitech700eCamera::Logitech700eCamera(QHostAddress addr, QString username, QString password)
: hostAddress(addr)
{
    uuid = QUuid::createUuid(); // TODO this should be stored and reused
    moveToThread( &thread );
    thread.start();
    QMetaObject::invokeMethod( this, "Logitech700eCameraImpl", Qt::QueuedConnection, Q_ARG( QString, username ), Q_ARG( QString, password ) );
}

Logitech700eCamera::~Logitech700eCamera()
{
    socketNotifier->setEnabled( false );

    thread.quit();
    thread.wait();
}

void Logitech700eCamera::Logitech700eCameraImpl(QString username, QString password)
{
    qDebug() << __FUNCTION__;

    QString jidStr = username + "@" + hostAddress.toString() + "/Commander/" + uuid.toString().mid(1,36);
    gloox::JID jid( jidStr.toUtf8().constData() );
    client = QSharedPointer<gloox::Client>( new gloox::Client( jid, password.toUtf8().constData() ) );

    client->registerConnectionListener( this );
    client->registerPresenceHandler( this );

    fileTransfer = QSharedPointer<gloox::SIProfileFT>( new gloox::SIProfileFT( client.data(), this ) );

    LogitechRecordingSearchResult *recordingSearch = new LogitechRecordingSearchResult( this );
    extensions.append( QSharedPointer<gloox::StanzaExtension>(recordingSearch) );
    client->registerStanzaExtension( recordingSearch );

    if ( client->connect( false ) )
    {
        int sock = static_cast<gloox::ConnectionTCPClient*>( client->connectionImpl() )->socket();
        socketNotifier = QSharedPointer<QSocketNotifier>( new QSocketNotifier(sock,QSocketNotifier::Read) );
        connect(socketNotifier.data(),SIGNAL(activated(int)),this,SLOT(readyRead()));
        socketNotifier->setEnabled( true );
        readyRead();
    } else {
        // will this cause a double disconnect emit?
        emit disconnected();
    }
}

int Logitech700eCamera::features()
{
    qDebug() << __FUNCTION__;
    return LiveVideo | Infrared | LocalRecording | Audio;
}

QUrl Logitech700eCamera::liveStream()
{
    qDebug() << __FUNCTION__;
    return QUrl("rtsp://" + hostAddress.toString() + "/HighResolutionVideo");
}

QString Logitech700eCamera::recordings()
{
    qDebug() << __FUNCTION__;
    return "";
}

void Logitech700eCamera::handleNewRecording(QString id)
{
    pendingTransfers.append( id );
    if ( 1 == pendingTransfers.size() )
        downloadFile(id);
}

void Logitech700eCamera::downloadFile(QString id)
{
    QString serverAddr("server@127.0.0.1/NvrCore");
    gloox::JID serverJid( serverAddr.toUtf8().constData() );
    LogitechRecordingTransferRequestIQ iq(serverJid, id.toUtf8().constData(), client->getID() );
    client->send( iq );
}

void Logitech700eCamera::downloadComplete(QString sid, bool success)
{
    qDebug() << __FUNCTION__;
    QHash< QString, QSharedPointer<LogitechBytestreamDataHandler> >::iterator dataHandler = transfers.find( sid );
    if( dataHandler != transfers.end() )
    {
        (*dataHandler)->deleteLater();
        transfers.erase( dataHandler );
    } else {
        qDebug() << "Could not find transfer" << sid;
    }

    // remove the transfer from the list
    pendingTransfers.pop_front();
    if ( 0 < pendingTransfers.size() )
        downloadFile( pendingTransfers.first() );
}


void Logitech700eCamera::readyRead()
{
    client->recv( 1 );
}

void Logitech700eCamera::onConnect()
{
    qDebug() << "connected";
    emit connected(false);

    QString serverAddr("server@127.0.0.1/NvrCore");
    gloox::JID serverJid( serverAddr.toUtf8().constData() );
    LogitechRecordingSearchIQ iq(serverJid, client->getID() );
    client->send( iq );
}

void Logitech700eCamera::onDisconnect(gloox::ConnectionError e)
{
    qDebug() << "disconnected" << e;
    emit disconnected();
}

bool Logitech700eCamera::onTLSConnect( const gloox::CertInfo& info )
{
    qDebug() << "connected (TLS)";
    emit connected(true);
    return true;
}

void Logitech700eCamera::handlePresence( const gloox::Presence &presence )
{
    // presence info
    qDebug() << "handlePresence" << presence.subtype();
}

//void Logitech700eCamera::handleLogitechTag(const gloox::Tag *tag)
//{
//    qDebug() << __FUNCTION__ << tag->xml().c_str();
//}

void Logitech700eCamera::handleFTRequest (const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes)
{
    QString fileName = QDesktopServices::storageLocation( QDesktopServices::MoviesLocation ) + QString("/") + QString( name.c_str() );
    if( QFileInfo( fileName ).exists() )
    { // we already have this file downloaded
        fileTransfer->declineFT( from, sid, gloox::SIManager::RequestRejected );

        qDebug()  << "File exists:" << fileName;
        pendingTransfers.pop_front();
        if ( 0 < pendingTransfers.size() )
            downloadFile( pendingTransfers.first() );
    } else {
        qDebug()  << "Downloading:" << fileName;
        LogitechBytestreamDataHandler *dataHandler = new LogitechBytestreamDataHandler(from, to, sid, name, size, hash, date, mimetype, desc, stypes);
        connect(dataHandler,SIGNAL(downloadComplete(QString,bool)), this, SLOT(downloadComplete(QString,bool)));
        transfers.insert( QString(sid.c_str()), QSharedPointer<LogitechBytestreamDataHandler>(dataHandler) );
        fileTransfer->acceptFT( from, sid );
    }
}

void Logitech700eCamera::handleFTRequestError (const gloox::IQ &iq, const std::string &sid)
{
    qDebug() << __FUNCTION__;
}

void Logitech700eCamera::handleFTBytestream (gloox::Bytestream *bs)
{
    qDebug() << __FUNCTION__;
    QHash< QString, QSharedPointer<LogitechBytestreamDataHandler> >::iterator dataHandler = transfers.find( QString( bs->sid().c_str() ) );
    if( dataHandler != transfers.end() )
    {
        (*dataHandler)->beginTransfer( bs );
    } else {
        qDebug() << "Could not find transfer" << bs->sid().c_str();
    }
}

const std::string Logitech700eCamera::handleOOBRequestResult (const gloox::JID &from, const gloox::JID &to, const std::string &sid)
{
    qDebug() << __FUNCTION__;
    return "";
}

