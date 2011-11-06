#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent)
: QWidget(parent)
{
    m_layout.addWidget( &m_player );
    m_layout.addWidget( &m_seek );
    m_layout.addWidget( &m_volume );
    m_layout.setStretch(0,100);
    setLayout( &m_layout );
}

void VideoWidget::play(QString url)
{
    m_player.play( Phonon::MediaSource( url ) );
    m_seek.setMediaObject( m_player.mediaObject() );
    m_volume.setAudioOutput( m_player.audioOutput() );
}

void VideoWidget::stop()
{
    m_player.stop();
}
