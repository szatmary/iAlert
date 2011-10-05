#ifndef HTTPSERVER_H
#define HTTPSERVER_H


#include <QPair>
#include <QString>

class HttpClient
{
public:
    enum Method
    {
        Get,
        Post,
    };

    Method  m_methid;
    QString m_resource;
    qint16  m_httpVersion;

    QList< QPair<QString,QString> > m_headers;
};

class HttpServer
{
public:
    HttpServer();
};

#endif // HTTPSERVER_H
