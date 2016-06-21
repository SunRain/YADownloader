#ifndef DOWNLOADMGR_H
#define DOWNLOADMGR_H

#include <QObject>
#include "yadownloader_global.h"
#include "SingletonPointer.h"

namespace YADownloader {

class TaskObject;
class YADOWNLOADERSHARED_EXPORT DownloadMgr : public QObject
{
    Q_OBJECT
    DECLARE_SINGLETON_POINTER(DownloadMgr)
public:
    virtual ~DownloadMgr();

    void NewDownloadTask(TaskObject *object);

};

} //YADownloader
#endif // DOWNLOADMGR_H
