#ifndef DOWNLOADMGR_H
#define DOWNLOADMGR_H

#include <QObject>
#include "yadownloader_global.h"
#include "SingletonPointer.h"
#include "DLRequest.h"

namespace YADownloader {

class DLTask;
class YADOWNLOADERSHARED_EXPORT DownloadMgr : public QObject
{
    Q_OBJECT
//    DECLARE_SINGLETON_POINTER(DownloadMgr)
public:
    explicit DownloadMgr(QObject *parent = 0);
    virtual ~DownloadMgr();

    DLTask *get(const DLRequest &request);

};

} //YADownloader
#endif // DOWNLOADMGR_H
