#include "registry.h"

#include <QDir>
#include <QDebug>
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
