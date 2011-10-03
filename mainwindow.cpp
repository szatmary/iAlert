#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHostAddress>

#include "logitechcameras.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    videoPlayer = new Phonon::VideoPlayer();
    videoLayout = new QVBoxLayout();
    videoLayout->addWidget( videoPlayer );
    videoWindow.setLayout( videoLayout );

    connect(&upnp,SIGNAL(discovery(QUpnpDiscovery)),this,SLOT(upnpDiscovery(QUpnpDiscovery)));
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

void MainWindow::refreshCameraList()
{
    ui->cameraList->clear();
    cameras.clear();
    upnp.discover();
    ui->cameraRefresh->setEnabled( false );
}

void MainWindow::upnpDiscovery(QUpnpDiscovery disco)
{
    if ( QString("urn:upnp-logitech-com:service:SecurityDeviceControl:1") == disco.m_serviceType )
    {
        if( ! cameras.contains( disco.m_deviceId )  )
        {
            Logitech700eCamera *camera = new Logitech700eCamera( disco.m_destAddr );
            cameras.insert( disco.m_deviceId, QSharedPointer<Camera>(camera) );
            ui->cameraList->insertItem(0, disco.m_deviceId );
            if ( 1 == ui->cameraList->count() )
                ui->cameraList->setCurrentIndex( 0 );
        }
    }
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
        videoPlayer->play( Phonon::MediaSource( i.value()->liveStream() ) );
        videoWindow.show();
    }
}
