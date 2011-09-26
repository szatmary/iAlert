#include "rtsp.h"

#include <QTimer>
#include <QDebug>

rtsp::rtsp(QObject *parent) :
    QObject(parent)
{
    moveToThread( &rtspThread );
    rtspThread.start();
}

void rtsp::openStream(QString path)
{
    QMetaObject::invokeMethod( this, "openStreamImpl", Qt::QueuedConnection, Q_ARG( QString, path ) );
}

void rtsp::openStreamImpl(QString path)
{
    pFormatCtx = 0;

    qDebug() << __FILE__ << __LINE__ << avformat_open_input(&pFormatCtx, path.toUtf8().constData(), 0, 0);

    if ( 0 > av_find_stream_info(pFormatCtx) )
    {
        qDebug() << "av_find_stream_info failed";
        emit finished();
        return;
    }

    dump_format(pFormatCtx, 0, path.toUtf8().constData(), 0);

    videoStream = -1;
    for( int i = 0; i < pFormatCtx->nb_streams; ++i )
    {
        if( AVMEDIA_TYPE_VIDEO == pFormatCtx->streams[i]->codec->codec_type )
        {
            videoStream=i;
            break;
        }
    }

    if( 0 > videoStream)
    {
        qDebug() << "could not find video stream";
        emit finished();
        return;
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    AVCodec *pCodec = avcodec_find_decoder( pCodecCtx->codec_id) ;
    if( 0 == pCodec )
    {
        qDebug() << "unsuported video codec";
        emit finished();
        return;
    }

    // Open codec
    if( 0 > avcodec_open(pCodecCtx, pCodec) )
    {
        qDebug() << "vcodec_open failed";
        emit finished();
        return;
    }

    pFrame = avcodec_alloc_frame();
    QTimer::singleShot(0,this,SLOT(nextFrame()));
}


void rtsp::nextFrame()
{
    int frameFinished;
    AVPacket packet;
    int result =  av_read_frame(pFormatCtx, &packet);
    if ( 0 <= result )
    {
        // Is this a packet from the video stream?
        if( videoStream == packet.stream_index )
        {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            // Did we get a video frame?
            if( frameFinished )
            {
                qDebug() << "got frame";
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
        QTimer::singleShot(0,this,SLOT(nextFrame()));
    }
    else
    {
        emit finished();
    }
}
