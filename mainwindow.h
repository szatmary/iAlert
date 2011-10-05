#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUrl>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QListWidgetItem>

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
    void upnpDiscoveryTimeout();
    void newRecording(QString);
private slots:
    void on_cameraRefresh_clicked();
    void on_cameraList_currentIndexChanged(const QString &arg1);

//    void on_eventList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_calendar_selectionChanged();

    void on_eventTable_itemSelectionChanged();

private:
    void closeEvent(QCloseEvent *event);
    QUpnp upnp;

    Ui::MainWindow *ui;
    CameraRoster    cameras;
};

#endif // MAINWINDOW_H
