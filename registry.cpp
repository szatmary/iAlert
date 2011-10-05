#include "registry.h"

#include <QDir>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QDesktopServices>


Registry::Registry(QObject *parent)
: QObject(parent)
{
    QString registryPath = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
    QDir dir(registryPath); if ( ! dir.exists() ); { dir.mkpath( registryPath ); }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName( registryPath + "/registry.db" );
    db.open(); // TODO test for errors}

    db.exec("CREATE TABLE IF NOT EXISTS settings ([name],[value])");
    db.exec("CREATE UNIQUE INDEX IF NOT EXISTS settings_idx1 ON settings ([name])");

    db.exec("CREATE TABLE IF NOT EXISTS recording ([recordingId], [deviceId], [fileName], [fileSize], [fileTime], [fileHash])");
    db.exec("CREATE UNIQUE INDEX IF NOT EXISTS recording_idx1 ON recording ([recordingId])");
    db.exec("CREATE UNIQUE INDEX IF NOT EXISTS recording_idx2 ON recording ([fileHash])");
    db.exec("CREATE INDEX IF NOT EXISTS recording_idx3 ON recording ([fileTime])");
}


QVariant Registry::getValue(QString name, QVariant defaultValue)
{
    QSqlQuery q("SELECT [value] FROM settings WHERE [name] = ?");
    q.bindValue(0,name);
    if ( q.exec() && q.next() )
        defaultValue = q.value(0);

    return defaultValue;
}

bool Registry::setValue(QString name, QVariant value)
{
    QSqlQuery q("REPLACE INTO settings ([name],[VALUE])VALUES(?,?)");
    q.bindValue(0,name);
    q.bindValue(1,value);
    return q.exec();
}

bool Registry::addRecording(Recording &rec)
{
    QSqlQuery q("REPLACE INTO recording ( [recordingId], [deviceId], [fileName], [fileSize], [fileTime], [fileHash] )VALUES( ?,?,?,?,?,? )");
    q.bindValue(0,rec.m_recordingId);
    q.bindValue(1,rec.m_deviceId);
    q.bindValue(2,rec.m_fileName);
    q.bindValue(3,rec.m_fileSize);
    q.bindValue(4,rec.m_fileTime.toTime_t());
    q.bindValue(5,rec.m_fileHash);
    return q.exec();
}

Recording Registry::findRecordingByHash(QString fileHash)
{
    Recording rec;
    QSqlQuery q("SELECT [recordingId], [deviceId], [fileName], [fileSize], [fileTime], [fileHash] FROM recording WHERE [fileHash] = ?");
    q.bindValue(0,fileHash);
    if( q.exec() && q.next() )
    {
        rec.m_recordingId = q.value(0).toString();
        rec.m_deviceId    = q.value(1).toString();
        rec.m_fileName    = q.value(2).toString();
        rec.m_fileSize    = q.value(3).toLongLong();
        rec.m_fileTime    = QDateTime::fromTime_t( q.value(4).toUInt() );
        rec.m_fileHash    = q.value(5).toString();
    }

    return rec;
}

Recording Registry::findRecordingById(QString recordingId)
{
    Recording rec;
    QSqlQuery q("SELECT [recordingId], [deviceId], [fileName], [fileSize], [fileTime], [fileHash] FROM recording WHERE [recordingId] = ?");
    q.bindValue(0,recordingId);
    if( q.exec() && q.next() )
    {
        rec.m_recordingId = q.value(0).toString();
        rec.m_deviceId    = q.value(1).toString();
        rec.m_fileName    = q.value(2).toString();
        rec.m_fileSize    = q.value(3).toLongLong();
        rec.m_fileTime    = QDateTime::fromTime_t( q.value(4).toUInt() );
        rec.m_fileHash    = q.value(5).toString();
    }

    return rec;
}

QList<Recording> Registry::findRecordingsByDate(QDate date)
{
    QList<Recording> list;
    int StartTime = QDateTime( date ).toTime_t();
    int EndTime = QDateTime( date.addDays(1) ).toTime_t();
    QSqlQuery q("SELECT [recordingId], [deviceId], [fileName], [fileSize], [fileTime], [fileHash] FROM recording WHERE [fileTime] >= ? AND [fileTime] < ? ORDER BY [fileTime]");
    q.bindValue(0,StartTime);
    q.bindValue(1,EndTime);
    q.exec();
    while( q.next() )
    {
        Recording rec;
        rec.m_recordingId = q.value(0).toString();
        rec.m_deviceId    = q.value(1).toString();
        rec.m_fileName    = q.value(2).toString();
        rec.m_fileSize    = q.value(3).toLongLong();
        rec.m_fileTime    = QDateTime::fromTime_t( q.value(4).toUInt() );
        rec.m_fileHash    = q.value(5).toString();
        list.append( rec );
    }

    return list;
}

