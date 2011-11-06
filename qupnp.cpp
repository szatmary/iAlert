#include "qupnp.h"
#include <QDebug>
int QUpnpCallback(Upnp_EventType eventType, void* event, QUpnp *cookie )
{
    switch(eventType)
    {
    case UPNP_DISCOVERY_SEARCH_RESULT:
        cookie->event( (Upnp_Discovery*)event );
        break;
    default:
        cookie->event( eventType );
        break;
    }

    return 1;
}

QUpnp::QUpnp(QObject *parent)
: QObject(parent)
{
    UpnpInit ( 0, 0 );
    UpnpRegisterClient ( (Upnp_FunPtr)QUpnpCallback, 0, &UpnpHandle );
    qRegisterMetaType<QUpnpDiscovery>("QUpnpDiscovery"); // Is the the correct spot to do this?
}

void QUpnp::discover(int searchTime, QString target)
{
    UpnpSearchAsync( UpnpHandle, searchTime, target.toAscii().constData(), this );
}

void QUpnp::event(Upnp_EventType eventType)
{
    switch(eventType)
    {
    case UPNP_DISCOVERY_SEARCH_TIMEOUT: emit discoveryTimeout(); break;
    }
}

void QUpnp::event(Upnp_Discovery *event)
{
    QUpnpDiscovery disco;
    disco.m_errCode = event->ErrCode;
    disco.m_expires = event->Expires;
    disco.m_deviceId = event->DeviceId;
    disco.m_deviceType = event->DeviceType;
    disco.m_serviceType = event->ServiceType;
    disco.m_serviceVer = event->ServiceVer;
    disco.m_location = event->Location;
    disco.m_os = event->Os;
    disco.m_date = event->Date;
    disco.m_ext = event->Ext;
    disco.m_destAddr = QHostAddress( (const sockaddr * )&event->DestAddr );

//    qDebug() << disco.m_errCode << disco.m_expires << disco.m_deviceId << disco.m_deviceType << disco.m_serviceType << disco.m_serviceVer << disco.m_location << disco.m_os << disco.m_date << disco.m_ext << disco.m_destAddr;
    // can I emit form a non QThread? (This will execute ins a thread created by libupnp)
    emit discovery(disco);
}

