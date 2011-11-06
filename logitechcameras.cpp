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

Logitech700eCamera::Logitech700eCamera(QString id, QHostAddress addr, QString username, QString password)
: Camera(id)
, m_addr(addr)
{
    m_xmppClient = QSharedPointer<QXmpp>( new QXmpp(addr,5222,username,password) );

    m_xmppClient->registerCustomStanza("/iq/Transfer[@xmlns='urn:logitech-com:logitech-alert:device:media:recording:file']");
    m_xmppClient->registerCustomStanza("/iq/Query[@xmlns='urn:logitech-com:logitech-alert:device:media:recording:search']");

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
    qDebug() << __FUNCTION__;

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

void Logitech700eCamera::queueRecordingTransfer(QString recordingId)
{
    m_downloadQueue.append( recordingId );
    if( 1 ==  m_downloadQueue.size() )
        doRecordingTransfer( recordingId );
}

void Logitech700eCamera::doRecordingTransfer(QString recordingId)
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

void Logitech700eCamera::getRecordingList()
{
    gloox::Tag *Query = new gloox::Tag("Query");
    Query->setXmlns("urn:logitech-com:logitech-alert:device:media:recording:search");

    gloox::Tag *set = new gloox::Tag(Query,"set");
    set->setXmlns("http://jabber.org/protocol/rsm");

    gloox::Tag *index = new gloox::Tag(set,"index","0");
    gloox::Tag *max = new gloox::Tag(set,"max","1000000"); // TODO set max to 100 and do pagenation

    gloox::Tag *Fields = new gloox::Tag(Query,"Fields");
    gloox::Tag *Field1 = new gloox::Tag(Fields,"Field", "name", "Id");
    gloox::Tag *Field2 = new gloox::Tag(Fields,"Field", "name", "StartTime");
    gloox::Tag *Field3 = new gloox::Tag(Fields,"Field", "name", "Duration");

    gloox::Tag *Criteria = new gloox::Tag(Query,"Criteria");
    gloox::Tag *Field4 = new gloox::Tag(Criteria,"Field", "name", "DeviceId");
    Field4->addAttribute("type","list-multi");
    gloox::Tag *Value = new gloox::Tag(Field4,"Value", m_instanceId.toUtf8().constData() );

    gloox::Tag *Field5 = new gloox::Tag(Criteria,"Field", "name", "StartTime");
    Field5->addAttribute("type","dateTime");

    gloox::Tag *Sort = new gloox::Tag(Field5, "Sort");
    Sort->addAttribute("allowed","1");
    Sort->addAttribute("order","1");
    Sort->addAttribute("direction","ASC"); // ASC/DESC

//        Tag *MinInclusive = new Tag(Field5,"MinInclusive", MinInclusiveStr);
    gloox::Tag *MinInclusive = new gloox::Tag(Field5,"MinInclusive");
    gloox::Tag *MaxInclusive = new gloox::Tag(Field5,"MaxInclusive");
    m_xmppClient->sendCustomIq( gloox::IQ::Get, Query );
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

        // Now that we have an instanceId, we can get teh recording list.
        getRecordingList();
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

    QPixmap snapshot;
    snapshot.loadFromData( cdata, mimeType.toAscii().constData() );
    m_lastEventSnapshot = snapshot;

    emit recordingEnded();

    // Add recording to transfer queue
    gloox::Tag *MediaRecording = tag->findChild("MediaRecording");
    QString recordingId = MediaRecording->findAttribute("id").c_str();
    queueRecordingTransfer( recordingId );
}

void Logitech700eCamera::xmppCustomStanza(QSharedPointer<gloox::Tag> tag)
{
    QString name = tag->name().c_str();
    if ( "Transfer" == name )
    {
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

        // TODO we should put the recording info in ram, not disk. And nly record to teh registry when teh recording is finisged downloading
        Registry::addRecording( rec );
    }
    else
    if ("Query" == name)
    {
//        qDebug() << tag->xml().c_str();
        gloox::Tag *results = tag->findChild("Results");
        if( results )
        {
            const gloox::TagList children = results->children();
            for( gloox::TagList::const_iterator i = children.begin() ; i != children.end() ; ++i )
            {
                QString id = (*i)->findAttribute("Id").c_str();
                Recording rec = Registry::findRecordingById( id );
                if( ! rec.exists() ) { queueRecordingTransfer( id ); }
            }
        }
    }
}

void Logitech700eCamera::xmppTransferComplete(QSharedPointer<QXmppFileTransfer> transfer,bool ok)
{
    if ( ok )
    {
        Recording rec = Registry::findRecordingByHash( transfer->hash() );
        if( rec.m_fileHash == transfer->hash() )
        {
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

    // Remove the head, We should probally check the return to make sure it isteh file that completed
    m_downloadQueue.takeFirst();
    if ( m_downloadQueue.size() )
        doRecordingTransfer( m_downloadQueue.first() );
}
