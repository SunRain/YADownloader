#ifndef DLTASKACCESSMGR_H
#define DLTASKACCESSMGR_H

#include <QObject>
#include "yadownloader_global.h"
#include "SingletonPointer.h"
#include "DLRequest.h"

namespace YADownloader {

class DLTask;
class DLTransmissionDatabase;
class YADOWNLOADERSHARED_EXPORT DLTaskAccessMgr : public QObject
{
    Q_OBJECT
//    DECLARE_SINGLETON_POINTER(DownloadMgr)
public:
    explicit DLTaskAccessMgr(QObject *parent = 0);
    virtual ~DLTaskAccessMgr();

    DLTask *get(const DLRequest &request);

private:
    DLTransmissionDatabase *m_transDB;

};

} //YADownloader
#endif // DLTASKACCESSMGR_H
