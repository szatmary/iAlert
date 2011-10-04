#include "camera.h"

#include <QDebug>

Camera::Camera(QString id, QObject *parent)
: QObject(parent)
, m_id(id){}

Camera::~Camera() {}
