#ifndef DLTASKACCESSMGR_H
#define DLTASKACCESSMGR_H

#include <QObject>
#include "yadownloader_global.h"
#include "SingletonPointer.h"
#include "DLRequest.h"
#include "DLTaskInfo.h"

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

    ///
    /// \brief resumables
    /// \return list of all resumable DLTaskInfo
    ///
    DLTaskInfoList resumables() const;

protected:
    static DLTransmissionDatabase *getTransDB();

signals:
    void resumablesChanged(const DLTaskInfoList &list);

};

} //YADownloader
#endif // DLTASKACCESSMGR_H