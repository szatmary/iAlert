#include "logitechcameras.h"
#include "registry.h"


#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QSocketNotifier>
#include <QDesktopServices>

#include <gloox/adhoc.h>
#include <gloox/pubsub.h>
#include <gloox/pubsubevent.h>
#include <gloox/connectiontcpclient.h>

#include <utime.h>

//QString recoringPath(QString camera, QDateTime time, QString fileName)
//{
//    QDir dir( QDesktopServices::storageLocation( QDesktopServices::MoviesLocation ) + "/iAlert/" + camera + "/"  + time.toString("yyyy") + "/" + time.toString("MM") + "/" + time.toString("dd") );

//    if ( ! dir.exists() )
//        dir.mkpath( dir.path() );

//    return dir.filePath( fileName );
//}
////////////////////////////////////////////////////////////////////////////
//#define LogitechRecordingSearchResultExtType (gloox::ExtUser + 1)
//LogitechRecordingSearchResult::LogitechRecordingSearchResult(LogitechHandler *handler)
//: StanzaExtension( LogitechRecordingSearchResultExtType )
//, m_handler(handler)
//{}

//LogitechRecordingSearchResult::LogitechRecordingSearchResult(const gloox::Tag *tag, LogitechHandler *handler)
//: StanzaExtension(LogitechRecordingSearchResultExtType)
//, m_handler(handler)
//{
//    qDebug() << __FUNCTION__;
//    if ( ! tag )
//        return;

//    // TODO check all tags for NULL
//    gloox::Tag *set   = tag->findChild("set");
//    gloox::Tag *count = set->findChild("count");

//    gloox::Tag *results = tag->findChild("Results");
//    if( results )
//    {
//        const gloox::TagList children = results->children();
//        qDebug() << "Received" << children.size() << "of" << count->cdata().c_str();
//        for( gloox::TagList::const_iterator i = children.begin() ; i != children.end() ; ++i )
//        {
//            // TODO sanitycheck Name is 'Item'
//    //        qDebug() << "Id:" << (*i)->findAttribute("Id").c_str() << "StartTime:" << (*i)->findAttribute("StartTime").c_str() << "Duration:" << (*i)->findAttribute("Duration").c_str();
//            handler->handleNewRecording( QString( (*i)->findAttribute("Id").c_str() ) );
//        }
//    }
//}

//LogitechRecordingSearchResult::LogitechRecordingSearchResult(const LogitechRecordingSearchResult &that)
//: StanzaExtension(LogitechRecordingSearchResultExtType)
//, m_handler(that.m_handler)
//{}

//const std::string &LogitechRecordingSearchResult::filterString () const
//{
//    static const std::string filter = "/iq/Query[@xmlns='urn:logitech-com:logitech-alert:device:media:recording:search']";
//    return filter;
//}

//gloox::StanzaExtension *LogitechRecordingSearchResult::newInstance (const gloox::Tag *tag) const
//{
//    return new LogitechRecordingSearchResult(tag,m_handler);
//}

//gloox::Tag *LogitechRecordingSearchResult::tag () const
//{
//    return 0; // TODO, But I dont thing we really need it
//}

//gloox::StanzaExtension *LogitechRecordingSearchResult::clone () const
//{
//    return new LogitechRecordingSearchResult( *this );
//}

//////////////////////////////////////////////////////////////////////////////
//class LogitechRecordingSearchIQ : public gloox::IQ
//{
//private:
//    QString instanceId;
//public:
//    LogitechRecordingSearchIQ(QString instanceId, gloox::JID &to, const std::string id = gloox::EmptyString)
//        : gloox::IQ(Get,to,id), instanceId( instanceId )
//    {}

//    gloox::Tag *tag() const
//    {
//        gloox::Tag *t = gloox::IQ::tag();

////        std::string MinInclusiveStr = "2011-09-14T14:38:01.990273-04:00";

//        gloox::Tag *Query = new gloox::Tag(t, "Query");
//        Query->setXmlns("urn:logitech-com:logitech-alert:device:media:recording:search");

//        gloox::Tag *set = new gloox::Tag(Query,"set");
//        set->setXmlns("http://jabber.org/protocol/rsm");

//        gloox::Tag *index = new gloox::Tag(set,"index","0");
//        gloox::Tag *max = new gloox::Tag(set,"max","100000");

//        gloox::Tag *Fields = new gloox::Tag(Query,"Fields");
//        gloox::Tag *Field1 = new gloox::Tag(Fields,"Field", "name", "Id");
//        gloox::Tag *Field2 = new gloox::Tag(Fields,"Field", "name", "StartTime");
//        gloox::Tag *Field3 = new gloox::Tag(Fields,"Field", "name", "Duration");

//        gloox::Tag *Criteria = new gloox::Tag(Query,"Criteria");
//        gloox::Tag *Field4 = new gloox::Tag(Criteria,"Field", "name", "DeviceId");
//        Field4->addAttribute("type","list-multi");
//        gloox::Tag *Value = new gloox::Tag(Field4,"Value", instanceId.toUtf8().constData() );

//        gloox::Tag *Field5 = new gloox::Tag(Criteria,"Field", "name", "StartTime");
//        Field5->addAttribute("type","dateTime");

//        gloox::Tag *Sort = new gloox::Tag(Field5, "Sort");
//        Sort->addAttribute("allowed","1");
//        Sort->addAttribute("order","1");
//        Sort->addAttribute("direction","DESC"); // ANC/DESC

////        Tag *MinInclusive = new Tag(Field5,"MinInclusive", MinInclusiveStr);
//        gloox::Tag *MinInclusive = new gloox::Tag(Field5,"MinInclusive");
//        gloox::Tag *MaxInclusive = new gloox::Tag(Field5,"MaxInclusive");

//        qDebug() << t->xml().c_str();
//        return t;
//    }
//};

//////////////////////////////////////////////////////////////////////////////
//class LogitechRecordingTransferRequestIQ : public gloox::IQ
//{
//public:
//    LogitechRecordingTransferRequestIQ( gloox::JID &to, QString recordingId, const std::string id = gloox::EmptyString)
//    : gloox::IQ(Set,to,id)
//    , m_recordingId(recordingId)
//    {}

//    gloox::Tag *tag() const
//    {
//        qDebug() << __FUNCTION__;

//        gloox::Tag *t = gloox::IQ::tag();

//        gloox::Tag *Transfer = new gloox::Tag(t, "Transfer");
//        Transfer->setXmlns("urn:logitech-com:logitech-alert:device:media:recording:file");

//        gloox::Tag *MediaRecording = new gloox::Tag(Transfer, "MediaRecording");
//        MediaRecording->addAttribute("id",m_recordingId.toUtf8().constData());

//        gloox::Tag *FileTransfer = new gloox::Tag(Transfer, "FileTransfer");
//        FileTransfer->setXmlns("urn:logitech-com:logitech-alert:file-transfer");

//        gloox::Tag *TransferMethod = new gloox::Tag(FileTransfer, "TransferMethod");
//        TransferMethod->addAttribute("type","http://jabber.org/protocol/bytestreams");

//        return t;
//    }
//private:
//    QString m_recordingId;
//};
//////////////////////////////////////////////////////////////////////////////
//LogitechBytestreamDataHandler::LogitechBytestreamDataHandler(QString cameraName, const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes)
//: QThread()
//, cameraName(cameraName)
//, fileHash( QCryptographicHash::Md5 )
//, gloox::BytestreamDataHandler()
//, from(from)
//, to(to)
//, sid(sid)
//, name(name)
//, size(size)
//, hash(hash)
//, date(date)
//, mimetype(mimetype)
//, desc(desc)
//, stypes(stypes)
//{
//    qDebug() << __FUNCTION__;

//}

//void LogitechBytestreamDataHandler::beginTransfer(gloox::Bytestream *bs)
//{
//    qDebug() << __FUNCTION__;
//    this->bs = bs;
//    bs->registerBytestreamDataHandler( this );
//    start();
//}

//void LogitechBytestreamDataHandler::run()
//{
//    // TODO make this interuptable somehow
//    gloox::ConnectionError error = gloox::ConnNoError;
//    bs->connect();
//    while( gloox::ConnNoError == error )
//    {
//        // TODO we need to put a timeout in here
//        error = bs->recv();
//    }
//}

//void LogitechBytestreamDataHandler::handleBytestreamData (gloox::Bytestream *bs, const std::string &data)
//{
//    file.write( data.c_str(), data.size() ); // will this handle binary data ok?
//    fileHash.addData( data.c_str(), data.size() );
//}

//void LogitechBytestreamDataHandler::handleBytestreamError (gloox::Bytestream *bs, const gloox::IQ &iq)
//{
//    qDebug() << __FUNCTION__;
//    file.close();
//    emit downloadComplete( QString(sid.c_str()), false );
//    // error, just let the file delete itself
//}

//void LogitechBytestreamDataHandler::handleBytestreamOpen (gloox::Bytestream *bs)
//{
//    qDebug() << __FUNCTION__ << QFileInfo( file ).filePath();
//    file.open();
//}

//void LogitechBytestreamDataHandler::handleBytestreamClose (gloox::Bytestream *bs)
//{
//    if ( QString( hash.c_str() ).toLower() != fileHash.result().toHex().toLower() )
//    {
//        qDebug() << "Hash mismatch" << QString( hash.c_str() ).toLower() << "!=" << fileHash.result().toHex().toLower();
//        return;
//    }

//    QString oldName = QFileInfo( file ).canonicalFilePath();
//    QString newName = recoringPath( cameraName, QDateTime::fromString(date.c_str(), Qt::ISODate), name.c_str() );


//    file.setAutoRemove( false );
//    file.rename( newName );
//    file.close();

//    qDebug() << "moved to" << newName;

//    // TODO does windows support utimes?
//    QDateTime time = QDateTime::fromString( date.c_str(), Qt::ISODate );
//    struct utimbuf times;
//    times.actime = time.toTime_t();
//    times.modtime = time.toTime_t();
//    utime( newName.toUtf8().constData(), &times );

//    emit downloadComplete( QString(sid.c_str()), true );
//}


////////////////////////////////////////////////////////////////////////////
Logitech700eCamera::Logitech700eCamera(QString id, QHostAddress addr, QString username, QString password)
: Camera(id)
, m_addr(addr)
{
    m_xmppClient = QSharedPointer<QXmpp>( new QXmpp(addr,5222,username,password) );

    m_xmppClient->registerCustomeStanza("/iq/Transfer[@xmlns='urn:logitech-com:logitech-alert:device:media:recording:file']");

    connect(m_xmppClient.data(), SIGNAL(connected   ()), this, SLOT(xmppConnected   ()));
    connect(m_xmppClient.data(), SIGNAL(disconnected()), this, SLOT(xmppDisconnected()));
    connect(m_xmppClient.data(), SIGNAL(commandResult(QSharedPointer<gloox::Adhoc::Command>)), this, SLOT(xmppCommandResult(QSharedPointer<gloox::Adhoc::Command>)));
    connect(m_xmppClient.data(), SIGNAL(publishEvent(QSharedPointer<gloox::PubSub::Event>)), this, SLOT(xmppPublishEvent(QSharedPointer<gloox::PubSub::Event>)));
    connect(m_xmppClient.data(), SIGNAL(transferComplete(QSharedPointer<QXmppFileTransfer>,bool)), this, SLOT(xmppTransferComplete(QSharedPointer<QXmppFileTransfer>,bool)));
    connect(m_xmppClient.data(), SIGNAL(customStanza(QSharedPointer<gloox::Tag>)), this, SLOT(xmppCustomStanza(QSharedPointer<gloox::Tag>)));


//    uuid = QUuid::createUuid(); // TODO this should be stored and reused
//    moveToThread( &thread );
//    thread.start();
//    QMetaObject::invokeMethod( this, "Logitech700eCameraImpl", Qt::QueuedConnection, Q_ARG( QString, username ), Q_ARG( QString, password ) );
}

Logitech700eCamera::~Logitech700eCamera()
{
}

//void Logitech700eCamera::Logitech700eCameraImpl(QString username, QString password)
//{
//    qDebug() << __FUNCTION__;

//    QString jidStr = username + "@" + hostAddress.toString() + "/Commander/" + uuid.toString().mid(1,36);
//    gloox::JID jid( jidStr.toUtf8().constData() );
//    client = QSharedPointer<gloox::Client>( new gloox::Client( jid, password.toUtf8().constData() ) );

//    client->registerConnectionListener( this );
//    client->registerPresenceHandler( this );
//    client->registerMessageHandler( this );


//    LogitechRecordingSearchResult *recordingSearch = new LogitechRecordingSearchResult( this );
////    extensions.append( QSharedPointer<gloox::StanzaExtension>(recordingSearch) );
//    client->registerStanzaExtension( recordingSearch );

//    fileTransfer = new gloox::SIProfileFT( client.data(), this );
//    adHoc = new gloox::Adhoc(client.data());

//    if ( client->connect( false ) )
//    {
//        int sock = static_cast<gloox::ConnectionTCPClient*>( client->connectionImpl() )->socket();
//        socketNotifier = QSharedPointer<QSocketNotifier>( new QSocketNotifier(sock,QSocketNotifier::Read) );
//        connect(socketNotifier.data(),SIGNAL(activated(int)),this,SLOT(readyRead()));
//        socketNotifier->setEnabled( true );
//        readyRead();
//    } else {
//        // will this cause a double disconnect emit?
//        emit disconnected();
//    }
//}

int Logitech700eCamera::features()
{
    return LiveVideo | Infrared | LocalRecording | Audio;
}

QUrl Logitech700eCamera::liveStream()
{
    return QUrl("rtsp://" + m_addr.toString() + "/HighResolutionVideo");
}

QString Logitech700eCamera::recordings()
{
    qDebug() << __FUNCTION__;
    return "";
}

void Logitech700eCamera::xmppConnected()
{
    // Things to do once connected.
    // Ptz = pan tilt zoom
    // nvr = network video recorder ?
    // wasatch = wasatch front (I havn't been to park city in a while)

    // Reverse eng commands

    // command: logitech-com:logitech-alert:nvr:basic:get
    // command: urn:logitech-com:logitech-alert:nvr:time:get (Do i need to set time if it is wrong?)
    // command: urn:logitech-com:logitech-alert:nvr:online:get (Do I really care about this one?)
    // command: urn:logitech-com:logitech-alert:nvr:devices:get

    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:basic:settings
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:video:settings
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:video:motion-detection:settings
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:video:ptz
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:audio:settings
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:alert:settings
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:wasatch:settings
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:media:recording:started
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:media:recording:motion-detected
    // pubsub: urn:logitech-com:logitech-alert:remote-event:device:media:recording:ended


    // command: urn:logitech-com:logitech-alert:nvr:device:alert:get (I don't know what this one is, something to do with email)
    // AvailableLiveMediaStreamsInfo: logitech-alert:device:media:stream
    // Ptz: urn:logitech-com:logitech-alert:device:video:ptz (Pan Tilt Zoom)
    // command: urn:logitech-com:logitech-alert:nvr:device:video:get
    // CaptureLiveImage: urn:logitech-com:logitech-alert:device:media:image
    // command: urn:logitech-com:logitech-alert:nvr:device:audio:get
    // command urn:logitech-com:logitech-alert:nvr:device:wasatch:get
    // Query: urn:logitech-com:logitech-alert:device:media:recording:search
    // Transfer: urn:logitech-com:logitech-alert:device:media:recording:file

    commandNvrBasicGet();
    subscribeRecordingEnded();


    // All known commands: (found via m_adHoc->getCommands();
    //urn:logitech-com:logitech-alert:device:media:recording:delete LogiNvrControlDeviceMediaRecordingDelete
    //urn:logitech-com:logitech-alert:device:media:recording:protect LogiNvrControlDeviceMediaRecordingProtect
    //urn:logitech-com:logitech-alert:nvr:basic:get LogiNvrControlNvrBasicGetRequest
    //urn:logitech-com:logitech-alert:nvr:basic:set LogiNvrControlNvrBasicSetRequest
    //urn:logitech-com:logitech-alert:nvr:device:alert:get LogiNvrControlDeviceAlertGetRequest
    //urn:logitech-com:logitech-alert:nvr:device:alert:set LogiNvrControlDeviceAlertSetRequest
    //urn:logitech-com:logitech-alert:nvr:device:audio:get LogiNvrControlDeviceAudioGetRequest
    //urn:logitech-com:logitech-alert:nvr:device:audio:set LogiNvrControlDeviceAudioSetRequest
    //urn:logitech-com:logitech-alert:nvr:device:get LogiNvrControlDeviceBasicGetRequest
    //urn:logitech-com:logitech-alert:nvr:device:set LogiNvrControlDeviceBasicSetRequest
    //urn:logitech-com:logitech-alert:nvr:device:video:get LogiNvrControlDeviceVideoGetRequest
    //urn:logitech-com:logitech-alert:nvr:device:video:set LogiNvrControlDeviceVideoSetRequest
    //urn:logitech-com:logitech-alert:nvr:device:wasatch:get LogiNvrControlDeviceWasatchGetRequest
    //urn:logitech-com:logitech-alert:nvr:device:wasatch:set LogiNvrControlDeviceWasatchSetRequest
    //urn:logitech-com:logitech-alert:nvr:devices:get LogiNvrControlNvrDevicesGetRequest
    //urn:logitech-com:logitech-alert:nvr:diagnostics:bandwidth-test:start LogiNvrControlNvrBandwidthTestStart
    //urn:logitech-com:logitech-alert:nvr:online:get LogiNvrControlNvrOnlineGetRequest
    //urn:logitech-com:logitech-alert:nvr:online:set LogiNvrControlNvrOnlineSetRequest
    //urn:logitech-com:logitech-alert:nvr:reset LogiNvrControlNvrReset
    //urn:logitech-com:logitech-alert:nvr:security:key:change LogiNvrControlNvrSecurityChangeKey
    //urn:logitech-com:logitech-alert:nvr:security:password:change LogiNvrControlNvrSecurityChangePassword
    //urn:logitech-com:logitech-alert:nvr:time:get LogiNvrControlNvrTimeGetRequest
    //urn:logitech-com:logitech-alert:nvr:time:set LogiNvrControlNvrTimeSetRequest
}

void Logitech700eCamera::xmppDisconnected()
{
    qDebug() << __FUNCTION__;
}

void Logitech700eCamera::commandNvrBasicGet()
{
    gloox::DataForm *form = new gloox::DataForm(gloox::TypeSubmit,"Get NVR Basic Request");
    form->addField( gloox::DataFormField::TypeHidden, "FORM_TYPE", "urn:logitech-com:logitech-alert:nvr:basic:get" );
    gloox::Adhoc::Command *cmd = new gloox::Adhoc::Command("urn:logitech-com:logitech-alert:nvr:basic:get", gloox::Adhoc::Command::Execute, form);

    m_xmppClient->sendCommand( cmd );
}

void Logitech700eCamera::subscribeRecordingEnded()
{
    m_xmppClient->subscribe("urn:logitech-com:logitech-alert:remote-event:device:media:recording:ended");
}

void Logitech700eCamera::requestRecordingTransfer(QString recordingId)
{
    gloox::Tag *Transfer       = new gloox::Tag("Transfer");
    gloox::Tag *MediaRecording = new gloox::Tag(Transfer, "MediaRecording");
    gloox::Tag *FileTransfer   = new gloox::Tag(Transfer, "FileTransfer");
    gloox::Tag *TransferMethod = new gloox::Tag(FileTransfer, "TransferMethod");

    Transfer->setXmlns("urn:logitech-com:logitech-alert:device:media:recording:file");
    MediaRecording->addAttribute("id",recordingId.toUtf8().constData());
    FileTransfer->setXmlns("urn:logitech-com:logitech-alert:file-transfer");
    TransferMethod->addAttribute("type","http://jabber.org/protocol/bytestreams");

    qDebug() << "requesting" << recordingId;
    m_xmppClient->sendCustomIq( gloox::IQ::Set, Transfer );
}

void Logitech700eCamera::xmppCommandResult(QSharedPointer<gloox::Adhoc::Command> cmd)
{
    QString node = cmd->node().c_str();
    if ( "urn:logitech-com:logitech-alert:nvr:basic:get" == node )
    {
        const gloox::DataForm *form = cmd->form();
        m_instanceId                 = QString( form->field("InstanceId")->value().c_str() );
        m_instanceName               = QString( form->field("InstanceName")->value().c_str() );
        m_instanceType               = QString( form->field("InstanceType")->value().c_str() );
        m_softwareVersion            = QString( form->field("SoftwareVersion")->value().c_str() );
        m_softwareVersionReleaseDate = QString( form->field("SoftwareVersionReleaseDate")->value().c_str() );
        m_softwareInstallDate        = QString( form->field("SoftwareInstallDate")->value().c_str() );
        m_operatingSystemFullName    = QString( form->field("OperatingSystemFullName")->value().c_str() );
        m_operatingSystemVersion     = QString( form->field("OperatingSystemVersion")->value().c_str() );
        m_systemUpTime               = QString( form->field("SystemUpTime")->value().c_str() );
    }
}

void Logitech700eCamera::xmppPublishEvent(QSharedPointer<gloox::PubSub::Event> event)
{
    gloox::Tag *tag = event->tag();
    tag = tag->findChild("items");
    tag = tag->findChild("item");
    tag = tag->findChild("MediaRecordingEnded");

    gloox::Tag *ThumbnailSnapshot = tag->findChild("ThumbnailSnapshot");

    m_lastEventDateTime = QDateTime::fromString( ThumbnailSnapshot->findAttribute("timestamp").c_str(), Qt::ISODate );
    QString    mimeType = ThumbnailSnapshot->findAttribute("mimeType").c_str();
    QByteArray cdata    = QByteArray::fromBase64( ThumbnailSnapshot->cdata().c_str() );
    m_lastEventSnapshot.loadFromData( cdata, mimeType.toAscii().constData() );

    emit recordingEnded();

    // Add recording to transfer queue
    gloox::Tag *MediaRecording = tag->findChild("MediaRecording");
    QString recordingId = MediaRecording->findAttribute("id").c_str();
    requestRecordingTransfer( recordingId );
}

void Logitech700eCamera::xmppCustomStanza(QSharedPointer<gloox::Tag> tag)
{
    QString name = tag->name().c_str();
    if ( "Transfer" == name )
    {
        qDebug() << tag->xml().c_str();

        Recording rec;
        gloox::Tag *MediaRecording = tag->findChild("MediaRecording");
        rec.m_recordingId = MediaRecording->findAttribute("id").c_str();

        gloox::Tag *Device = tag->findChild("Device");
        rec.m_deviceId = Device->findAttribute("id").c_str();

        gloox::Tag *FileTransfer = tag->findChild("FileTransfer");
        gloox::Tag *File = FileTransfer->findChild("File");
        rec.m_fileName = File->findAttribute("name").c_str();
        rec.m_fileSize = QString( File->findAttribute("size").c_str() ).toLongLong();
        rec.m_fileTime = QDateTime::fromString( File->findAttribute("date").c_str(), Qt::ISODate );
        rec.m_fileHash = QString( File->findAttribute("hash").c_str() ).toLower();

        Registry::addRecording( rec );
    }
}

void Logitech700eCamera::xmppTransferComplete(QSharedPointer<QXmppFileTransfer> transfer,bool ok)
{
    qDebug() << __FUNCTION__  << __FILE__;
    if ( ok )
    {
        qDebug() << __FUNCTION__  << __FILE__;
        Recording rec = Registry::findRecordingByHash( transfer->hash() );
        if( rec.m_fileHash == transfer->hash() )
        {
            qDebug() << __FUNCTION__  << __FILE__;
            QString newName = rec.filePath();
            rec.mkpath();
            transfer->moveFile( newName );

            // set the time stamps
            struct utimbuf times;
            times.actime  = rec.m_fileTime.toTime_t();
            times.modtime = rec.m_fileTime.toTime_t();
            utime( newName.toUtf8().constData(), &times );

            emit newRecording(rec.m_recordingId);
        }
    }
}

//void Logitech700eCamera::handleNewRecording(QString id)
//{
//    qDebug() << __FUNCTION__;
//    pendingTransfers.append( id );
//    if ( 1 == pendingTransfers.size() )
//        downloadFile(id);
//}

//void Logitech700eCamera::downloadFile(QString id)
//{
//    QString serverAddr("server@127.0.0.1/NvrCore");
//    gloox::JID serverJid( serverAddr.toUtf8().constData() );
//    LogitechRecordingTransferRequestIQ iq(serverJid, id.toUtf8().constData(), client->getID() );
//    client->send( iq );
//}

//void Logitech700eCamera::basicGet()
//{
//    gloox::DataForm *form = new gloox::DataForm(gloox::TypeSubmit,"Get NVR Basic Request");
//    form->addField( gloox::DataFormField::TypeHidden, "FORM_TYPE", "urn:logitech-com:logitech-alert:nvr:basic:get" );
//    gloox::Adhoc::Command *cmd = new gloox::Adhoc::Command("urn:logitech-com:logitech-alert:nvr:basic:get", gloox::Adhoc::Command::Execute, form);

//    QString serverAddr("server@127.0.0.1/NvrCore");
//    gloox::JID serverJid( serverAddr.toUtf8().constData() );
//    adHoc->execute( serverJid, cmd, this);
//}


//void Logitech700eCamera::downloadComplete(QString sid, bool success)
//{
//    qDebug() << __FUNCTION__;
//    QHash< QString, QSharedPointer<LogitechBytestreamDataHandler> >::iterator dataHandler = transfers.find( sid );
//    if( dataHandler != transfers.end() )
//    {
//        (*dataHandler)->deleteLater();
//        transfers.erase( dataHandler );
//    } else {
//        qDebug() << "Could not find transfer" << sid;
//    }

//    // remove the transfer from the list
//    pendingTransfers.pop_front();
//    if ( 0 < pendingTransfers.size() )
//        downloadFile( pendingTransfers.first() );

//    qDebug() << pendingTransfers.size() << "Pending Transfers";
//}


//void Logitech700eCamera::readyRead()
//{
//    client->recv( 1 );
//}

//void Logitech700eCamera::onConnect()
//{
//    qDebug() << "connected";
//    emit connected(false);

//    // TODO I thing we need a state machiene now
//    basicGet();
//}

//void Logitech700eCamera::onDisconnect(gloox::ConnectionError e)
//{
//    qDebug() << "disconnected" << e;
//    emit disconnected();
//}

//bool Logitech700eCamera::onTLSConnect( const gloox::CertInfo& info )
//{
//    qDebug() << "connected (TLS)";
//    emit connected(true);
//    return true;
//}

//void Logitech700eCamera::handlePresence( const gloox::Presence &presence )
//{
//    // presence info
//    qDebug() << "handlePresence" << presence.subtype();
//}

//void Logitech700eCamera::handleFTRequest (const gloox::JID &from, const gloox::JID &to, const std::string &sid, const std::string &name, long size, const std::string &hash, const std::string &date, const std::string &mimetype, const std::string &desc, int stypes)
//{
//    qDebug() << __FUNCTION__;
//    QString fileName = recoringPath( cameraName(), QDateTime::fromString(date.c_str(), Qt::ISODate), name.c_str() );

//    if( QFileInfo( fileName ).exists() )
//    { // we already have this file downloaded

//        //fileTransfer->declineFT( from, sid, gloox::SIManager::RequestRejected );
//        // Change this!
//        // I think declineFT removes teh file from teh camera
//        // WOW! this doesnt delete the files, But they become unavailable, but continue to take up space.
//        // DO NOT DO THIS!

//        qDebug()  << "File exists:" << fileName << date.c_str();

//        pendingTransfers.pop_front();
//        if ( 0 < pendingTransfers.size() )
//            downloadFile( pendingTransfers.first() );
//    } else {
//        qDebug()  << "Downloading:" << fileName;
//        LogitechBytestreamDataHandler *dataHandler = new LogitechBytestreamDataHandler( cameraName(), from, to, sid, name, size, hash, date, mimetype, desc, stypes);
//        connect(dataHandler,SIGNAL(downloadComplete(QString,bool)), this, SLOT(downloadComplete(QString,bool)));
//        transfers.insert( QString(sid.c_str()), QSharedPointer<LogitechBytestreamDataHandler>(dataHandler) );
//        fileTransfer->acceptFT( from, sid );
//    }
//}

//void Logitech700eCamera::handleFTRequestError (const gloox::IQ &iq, const std::string &sid)
//{
//    qDebug() << __FUNCTION__;
//}

//void Logitech700eCamera::handleFTBytestream (gloox::Bytestream *bs)
//{
//    qDebug() << __FUNCTION__;
//    QHash< QString, QSharedPointer<LogitechBytestreamDataHandler> >::iterator dataHandler = transfers.find( QString( bs->sid().c_str() ) );
//    if( dataHandler != transfers.end() )
//    {
//        (*dataHandler)->beginTransfer( bs );
//    } else {
//        qDebug() << "Could not find transfer" << bs->sid().c_str();
//    }
//}

//const std::string Logitech700eCamera::handleOOBRequestResult (const gloox::JID &from, const gloox::JID &to, const std::string &sid)
//{
//    qDebug() << __FUNCTION__;
//    return "";
//}

//void Logitech700eCamera::handleAdhocSupport (const gloox::JID &remote, bool support)
//{
//    qDebug() << __FUNCTION__;
//}

//void Logitech700eCamera::handleAdhocCommands (const gloox::JID &remote, const gloox::StringMap &commands)
//{
//    qDebug() << __FUNCTION__;
//}

//void Logitech700eCamera::handleAdhocError (const gloox::JID &remote, const gloox::Error *error)
//{
//    qDebug() << __FUNCTION__;
//}

//void Logitech700eCamera::handleAdhocExecutionResult (const gloox::JID &remote, const gloox::Adhoc::Command &command)
//{
//    qDebug() << __FUNCTION__;
//    if( "urn:logitech-com:logitech-alert:nvr:basic:get" == command.node() )
//    {
//        instanceId = QString( command.form()->field("InstanceId")->value().c_str() );
//        instanceName = QString( command.form()->field("InstanceName")->value().c_str() );
//        instanceType = QString( command.form()->field("InstanceType")->value().c_str() );
//        softwareVersion = QString( command.form()->field("SoftwareVersion")->value().c_str() );
//        softwareVersionReleaseDate = QString( command.form()->field("SoftwareVersionReleaseDate")->value().c_str() );
//        softwareInstallDate = QString( command.form()->field("SoftwareInstallDate")->value().c_str() );
//        operatingSystemFullName = QString( command.form()->field("OperatingSystemFullName")->value().c_str() );
//        operatingSystemVersion = QString( command.form()->field("OperatingSystemVersion")->value().c_str() );
//        systemUpTime = QString( command.form()->field("SystemUpTime")->value().c_str() );


//        QString serverAddr("server@127.0.0.1/NvrCore");
//        gloox::JID serverJid( serverAddr.toUtf8().constData() );
//        LogitechRecordingSearchIQ iq(instanceId, serverJid, client->getID() );
//        client->send( iq );
//    }
//}

//void Logitech700eCamera::handleMessage (const gloox::Message &msg, gloox::MessageSession *session)
//{
//    const gloox::PubSub::Event *pse = msg.findExtension<gloox::PubSub::Event>( gloox::ExtPubSubEvent );
//     if( pse )
//     {
//         qDebug() << "PubSub message";
//         // use the Event
//     }
//     else
//     {
//         qDebug() << "Non PubSub message";
//       // no Event
//     }
//}


