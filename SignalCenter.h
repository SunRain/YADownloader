#ifndef SIGNALCENTER_H
#define SIGNALCENTER_H

#include <QObject>
#include "yadownloader_global.h"
#include "SingletonPointer.h"

namespace YADownloader {

class TaskObject;
class YADOWNLOADERSHARED_EXPORT SignalCenter : public QObject
{
    Q_OBJECT

    DECLARE_SINGLETON_POINTER(SignalCenter)
public:
//    void sendTaskAdd(TaskObject *object);
signals:
    void TaskObjectAdded(TaskObject **object);
    void initNetworkFileHeaderError();
};

} //YADownloader

#endif // SIGNALCENTER_H
