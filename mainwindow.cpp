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
    ui->eventTable->setSelectionBehavior( QAbstractItemView::SelectRows );
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

void MainWindow::newRecording(QString id)
{
    Camera *camera = (Camera *)sender();

    qDebug() << "new recording" << id;
    //    int index = ui->cameraList->findText( camera->id() );
//    if ( index >= 0 ) { ui->cameraList->setCurrentIndex(); }
    ui->lastEventTimeLabel->setText( camera->lastRecordingDateTime().toString( Qt::SystemLocaleShortDate)  );
    ui->lastEventImage->setPixmap( camera->lastRecordingSnapshot() );
    int row = ui->eventTable->rowCount();

    Recording rec = Registry::findRecordingById( id );

    ui->eventTable->insertRow( row );
    ui->eventTable->setItem( row, 0, new QTableWidgetItem( rec.m_fileTime.time().toString() ) );
    ui->eventTable->setItem( row, 1, new QTableWidgetItem( id ) );
}

void MainWindow::refreshCameraList()
{
    ui->cameraList->clear();
    ui->eventTable->clearContents();
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
            connect(camera,SIGNAL(newRecording(QString)),this,SLOT(newRecording(QString)));
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
    ui->videoWidget->stop();
    refreshCameraList();
}

void MainWindow::on_cameraList_currentIndexChanged(const QString &arg1)
{
    CameraRoster::iterator i = cameras.find( arg1 );
    if( cameras.end() != i )
    {
        ui->eventTable->clearContents();
    }
}

//void MainWindow::on_eventList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
//{
//    Q_UNUSED(previous);
//    if( "Live" == current->text() )
//    {
//        QString cameraName = ui->cameraList->currentText();
//        QSharedPointer<Camera> camera = cameras[cameraName];
//        ui->videoWidget->play( camera->liveStream().toString() );
//    }
//}

void MainWindow::on_calendar_selectionChanged()
{
    // TODO load the recordings from teh database.
}

void MainWindow::on_eventTable_itemSelectionChanged()
{
    int row = ui->eventTable->currentRow();
    QString id = ui->eventTable->item(row,1)->text();
    Recording rec = Registry::findRecordingById( id );
    ui->videoWidget->play( rec.filePath() );
}
