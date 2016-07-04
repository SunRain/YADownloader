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
public:
    explicit DLTaskAccessMgr(QObject *parent = 0);
    virtual ~DLTaskAccessMgr();

    DLTask *get(const DLRequest &request);

    ///
    /// \brief resumables
    /// \return list of all resumable DLTaskInfo
    /// DLTaskAccessMgr will save all of current running DLTask uuids in DLTaskAccessMgr life-cycle
    /// If uuid of resumable DLTaskInfo not included in current running DLTask, if would be empty string
    ///
    DLTaskInfoList resumables() const;

protected:
    static DLTransmissionDatabase *getTransDB();

signals:
    void resumablesChanged(const DLTaskInfoList &list);

private:
    DLTaskInfoList m_list;
    QStringList m_runningUUID;

};

} //YADownloader
#endif // DLTASKACCESSMGR_H
