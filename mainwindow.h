#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUrl>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QListWidgetItem>
#include <phonon/videoplayer.h>

#include <upnp/upnp.h>
#include <gloox/gloox.h>

#include "camera.h"

namespace Ui
{
    class MainWindow;
}

// TODO move the upnp code it its own class
class MainWindow;
int UpnpCallback(Upnp_EventType eventType, void *event, MainWindow *cookie );

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
   explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

public slots:
    void refreshCameraList();
private slots:
    void on_cameraRefresh_clicked();
    void on_cameraList_currentIndexChanged(const QString &arg1);

private:
    void closeEvent(QCloseEvent *event);

    Ui::MainWindow *ui;
    CameraRoster    cameras;

// Video Window
    QWidget              videoWindow;
    QVBoxLayout         *videoLayout;
    Phonon::VideoPlayer *videoPlayer;

// Upnp stuff
    UpnpClient_Handle UpnpHandle;
    friend int UpnpCallback(Upnp_EventType eventType, void *event, MainWindow *cookie );
    void UPnPEvent(Upnp_Discovery *event);
    void UPnPEvent(Upnp_EventType eventType); // gotta catch 'em all!
};

#endif // MAINWINDOW_H
