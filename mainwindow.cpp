#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHostAddress>

#include "rtsp.h"
#include "jabber.h"
#include "logitechcameras.h"


// In Seconds
#define SEARCH_TIME 30

int UpnpCallback(Upnp_EventType eventType, void* event, MainWindow *cookie )
{
    switch(eventType)
    {
    case UPNP_DISCOVERY_SEARCH_RESULT:
        cookie->UPnPEvent( (Upnp_Discovery*)event );
        break;
    default:
        cookie->UPnPEvent( eventType );
        break;
    }

    return 1;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    videoPlayer = new Phonon::VideoPlayer();
    videoLayout = new QVBoxLayout();
    videoLayout->addWidget( videoPlayer );
    videoWindow.setLayout( videoLayout );

    UpnpInit ( 0, 0 );
    UpnpRegisterClient ( (Upnp_FunPtr)UpnpCallback, 0, &UpnpHandle );
    refreshCameraList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qApp->quit();
}

void MainWindow::UPnPEvent(Upnp_EventType eventType)
{
    switch(eventType)
    {
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
        ui->cameraRefresh->setEnabled( true );
    }
}

void MainWindow::UPnPEvent(Upnp_Discovery *event)
{
    if ( QString("urn:upnp-logitech-com:service:SecurityDeviceControl:1") == QString(event->ServiceType) )
    {
        if( ! cameras.contains( QString(event->DeviceId) )  )
        {
            Logitech700eCamera *camera = new Logitech700eCamera( QHostAddress( (const sockaddr * )&event->DestAddr ) );
            cameras.insert( event->DeviceId, QSharedPointer<Camera>(camera) );
            ui->cameraList->insertItem(0, QString( event->DeviceId ) );
        }
    }
}

void MainWindow::refreshCameraList()
{
    ui->cameraList->clear();
    cameras.clear();
    UpnpSearchAsync( UpnpHandle, SEARCH_TIME, "ssdp:all", this );
    ui->cameraRefresh->setEnabled( false );
}

void MainWindow::on_cameraRefresh_clicked()
{
    refreshCameraList();
}

void MainWindow::on_cameraList_currentIndexChanged(const QString &arg1)
{
    CameraRoster::iterator i = cameras.find( arg1 );
    if( cameras.end() != i )
    {
        qDebug() << "Playing stream" << i.value()->liveStream();
        videoPlayer->play( Phonon::MediaSource( i.value()->liveStream() ) );
        videoWindow.show();
    }
}
