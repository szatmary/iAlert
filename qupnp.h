#ifndef QUPNP_H
#define QUPNP_H

#include <QObject>
#include <QMetaType>
#include <QDateTime>
#include <QHostAddress>

#include <upnp/upnp.h>


// libupnp uses native threads, This is here to convert those callbacks to signals, that can be queued by a receiver.
// TODO move the upnp code it its own class

class QUpnp;
int QUpnpCallback(Upnp_EventType eventType, void *event, QUpnp *cookie );

struct QUpnpDiscovery
{
    int          m_errCode;
    int          m_expires;
    QString      m_deviceId;
    QString      m_deviceType;
    QString      m_serviceType;
    QString      m_serviceVer;
    QString      m_location;
    QString      m_os;
    QString      m_date; // TODO parse this and use a QDateTime Object
    QString      m_ext;
    QHostAddress m_destAddr;
};


#define QUPNP_DEFAULTSEARCHTIME 30 // in seconds

class QUpnp : public QObject
{
Q_OBJECT
public:
    explicit QUpnp(QObject *parent = 0);
    void discover(int searchTime = QUPNP_DEFAULTSEARCHTIME, QString target = QString("ssdp:all"));
signals:
    void discovery(QUpnpDiscovery);
    void discoveryTimeout();

private:
    UpnpClient_Handle UpnpHandle;

    friend int QUpnpCallback(Upnp_EventType eventType, void *event, QUpnp *cookie );
    void event(Upnp_Discovery *event);
    void event(Upnp_EventType eventType); // gotta catch 'em all!
};

#endif // UPNP_H
