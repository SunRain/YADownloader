#include "SignalCenter.h"

#include <QDebug>

namespace YADownloader {

SignalCenter::SignalCenter(QObject *parent)
    : QObject(parent)
{

}

//void SignalCenter::sendTaskAdd(TaskObject *object)
//{
//    qDebug()<<Q_FUNC_INFO<<"===";
//    emit TaskObjectAdded (object);
//}

} //
