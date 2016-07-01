#include "DLTaskAccessMgr.h"

#include <QDebug>
#include <QMutex>
#include <QScopedPointer>

#include "DLTaskImpl.h"
#include "DLTransmissionDatabase.h"

namespace YADownloader {

DLTaskAccessMgr::DLTaskAccessMgr(QObject *parent)
    : QObject(parent)
{

}

DLTaskAccessMgr::~DLTaskAccessMgr()
{
}

DLTask *DLTaskAccessMgr::get(const DLRequest &request)
{
    DLTaskImpl *task = new DLTaskImpl(request, getTransDB());
    return task;
}

DLTaskInfoList DLTaskAccessMgr::resumables() const
{
    return getTransDB()->list();
}

DLTransmissionDatabase *DLTaskAccessMgr::getTransDB()
{
    static QMutex mutex;
    static QScopedPointer<DLTransmissionDatabase> sp;
    if (Q_UNLIKELY(sp.isNull())) {
        mutex.lock();
        if (Q_UNLIKELY(sp.isNull()))
            sp.reset(new DLTransmissionDatabase);
        mutex.unlock();
    }
    return sp.data();
}



} //YADownloader
