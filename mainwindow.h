#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUrl>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QListWidgetItem>
#include <phonon/videoplayer.h>

#include <gloox/gloox.h>

#include "camera.h"
#include "qupnp.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
   explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

public slots:
    void refreshCameraList();
    void upnpDiscovery(QUpnpDiscovery);
private slots:
    void on_cameraRefresh_clicked();
    void on_cameraList_currentIndexChanged(const QString &arg1);

private:
    void closeEvent(QCloseEvent *event);
    QUpnp upnp;

    Ui::MainWindow *ui;
    CameraRoster    cameras;

// Video Window
    QWidget              videoWindow;
    QVBoxLayout         *videoLayout;
    Phonon::VideoPlayer *videoPlayer;
};

#endif // MAINWINDOW_H
