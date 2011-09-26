#ifndef RTSP_H
#define RTSP_H

#include <QThread>


extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}


class rtsp : public QObject
{
Q_OBJECT
public:
    explicit rtsp(QObject *parent = 0);
    void openStream(QString path);
signals:
    void finished();
public slots:
private slots:
    void openStreamImpl(QString path);
    void nextFrame();
private:
    QThread rtspThread;

    AVFormatContext *pFormatCtx;
    int videoStream;
    AVCodecContext *pCodecCtx;
    AVFrame *pFrame;
};

#endif // RTSP_H
