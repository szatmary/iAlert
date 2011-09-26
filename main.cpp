#include <QApplication>
#include "mainwindow.h"
#include "rtsp.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    av_register_all();
    MainWindow w;
    w.show();

    return a.exec();
}
