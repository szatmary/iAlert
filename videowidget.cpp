#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent)
: QWidget(parent)
, layout( new QVBoxLayout() )
, player( new Phonon::VideoPlayer() )
{
    layout->addWidget( player );
    setLayout( layout );
}


void VideoWidget::play(QString url)
{
    player->play( Phonon::MediaSource( url ) );
}
