#ifndef REGISTRY_H
#define REGISTRY_H

#include "camera.h"

#include <QObject>
#include <QVariant>
#include <QSqlDatabase>

// The registry should only be accessed vis its static members. DO not create an instance!
class Registry : public QObject
{
    Q_OBJECT
public:
    explicit Registry(QObject *parent = 0);

    static QVariant getValue(QString name, QVariant defaultValue = QVariant());
    static bool     setValue(QString name, QVariant value);

    static bool addRecording(Recording &rec);
    static Recording findRecordingByHash(QString);
    static Recording findRecordingById(QString);


private:
    QSqlDatabase db;
};

#endif // REGISTRY_H
