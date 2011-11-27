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
    CameraRoster::iterator i = cameras.find( ui->cameraList->currentText() );

    ui->lastEventTimeLabel->setText( camera->lastRecordingDateTime().toString( Qt::SystemLocaleShortDate)  );
    ui->lastEventImage->setPixmap( camera->lastRecordingSnapshot() );
    int row = ui->eventTable->rowCount();

    Recording rec = Registry::findRecordingById( id );
    if ( (*i)->id() == rec.m_deviceId && ui->calendar->selectedDate() == rec.m_fileTime.date() )
    { // add to view if we are looking at this day and this camera
        ui->eventTable->insertRow( row );
        ui->eventTable->setItem( row, 0, new QTableWidgetItem( rec.m_fileTime.time().toString() ) );
        ui->eventTable->setItem( row, 1, new QTableWidgetItem( id ) );
    }
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
            Logitech700eCamera *camera = new Logitech700eCamera( disco.m_destAddr );
            connect(camera,SIGNAL(newRecording(QString)),this,SLOT(newRecording(QString)));
            cameras.insert( disco.m_deviceId, QSharedPointer<Camera>(camera) );
            ui->cameraList->insertItem(0, disco.m_deviceId );
            if ( 1 == ui->cameraList->count() )
            {
                ui->cameraList->setCurrentIndex( 0 );
            }
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

    CameraRoster::iterator i = cameras.find( ui->cameraList->currentText() );
    if( cameras.end() != i )
    {
        while( ui->eventTable->rowCount() )
        {
            ui->eventTable->removeRow( 0 );
        }

        ui->eventTable->insertRow(0);
        ui->eventTable->setItem( 0, 0, new QTableWidgetItem( "LIVE" ) );
        ui->eventTable->setItem( 0, 1, new QTableWidgetItem( (*i)->liveStream().toString() ) );
        ui->eventTable->setCurrentCell(0,0);
        on_calendar_selectionChanged();
    }
}

void MainWindow::on_calendar_selectionChanged()
{
    // This is a bit of a race condition here.
    // we add a recording to the database beofore it is done downloading.
    CameraRoster::iterator i = cameras.find( ui->cameraList->currentText() );
    if( cameras.end() != i )
    {
        // Really? we have to delete the rows one at a time?
        while( 1 < ui->eventTable->rowCount() )
            ui->eventTable->removeRow( 1 );

        QList<Recording> list = Registry::findRecordingsByDate( ui->calendar->selectedDate() );
        foreach(Recording rec, list)
        {
            if( (*i)->id() == rec.m_deviceId )
            {
                int row = ui->eventTable->rowCount();
                ui->eventTable->insertRow( row );
                ui->eventTable->setItem( row, 0, new QTableWidgetItem( rec.m_fileTime.time().toString() ) );
                ui->eventTable->setItem( row, 1, new QTableWidgetItem( rec.m_recordingId ) );
            }
        }
    }
}

void MainWindow::on_eventTable_itemSelectionChanged()
{
    int row = ui->eventTable->currentRow();
    if( 0 <= row)
    {
        QString time = ui->eventTable->item(row,0)->text();
        QString id   = ui->eventTable->item(row,1)->text();
        if( "LIVE" == time )
        {
            qDebug() << __LINE__ << "Playing" << id;
            ui->videoWidget->play( id );
        } else {
            Recording rec = Registry::findRecordingById( id );
            qDebug() << __LINE__  << "Playing" << rec.filePath();
            ui->videoWidget->play( rec.filePath() );
        }
    }
}
