#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registry.h"

#include <QUuid>
#include <QDebug>
#include <QHostAddress>

#include "logitechcameras.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("iAlert");

    QString uuid = Registry::getValue("uuid").toString();
    if (uuid.isEmpty() )
    {
        uuid = QUuid::createUuid().toString();
        Registry::setValue("uuid",uuid);
    }

    connect(&upnp,SIGNAL(discovery(QUpnpDiscovery)),this,SLOT(upnpDiscovery(QUpnpDiscovery)));
    connect(&upnp,SIGNAL(discoveryTimeout()),this,SLOT(upnpDiscoveryTimeout()));
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

void MainWindow::cameraRecordingEnded()
{
    Camera *camera = (Camera *)sender();
//    int index = ui->cameraList->findText( camera->id() );
//    if ( index >= 0 ) { ui->cameraList->setCurrentIndex(); }
    ui->lastEventTimeLabel->setText( camera->lastRecordingDateTime().toString( Qt::SystemLocaleShortDate)  );
    ui->lastEventImage->setPixmap( camera->lastRecordingSnapshot() );
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
            Logitech700eCamera *camera = new Logitech700eCamera( disco.m_deviceId, disco.m_destAddr );
            connect(camera,SIGNAL(recordingEnded()),this,SLOT(cameraRecordingEnded()));
            cameras.insert( disco.m_deviceId, QSharedPointer<Camera>(camera) );
            ui->cameraList->insertItem(0, disco.m_deviceId );
            if ( 1 == ui->cameraList->count() )
                ui->cameraList->setCurrentIndex( 0 );
        }
    }
}

void MainWindow::upnpDiscoveryTimeout()
{
    ui->cameraRefresh->setEnabled( true );
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
        ui->eventList->clear();
        ui->eventList->addItem("Live");
        ui->eventList->setCurrentRow( 0 );
    }
}

void MainWindow::on_eventList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if( "Live" == current->text() )
    {
        QString cameraName = ui->cameraList->currentText();
        QSharedPointer<Camera> camera = cameras[cameraName];
        ui->videoWidget->play( camera->liveStream().toString() );
    }
}
