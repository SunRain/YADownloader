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
    connect(getTransDB(), &DLTransmissionDatabase::listChanged, [&]() {
        m_list.clear();
        foreach (DLTaskInfo info, getTransDB()->list()) {
            if (!m_runningUUID.contains(info.identifier())) {
                info.setIdentifier(QString());
            }
            m_list.append(info);
        }
        emit resumablesChanged(m_list);
    });
    foreach (DLTaskInfo info, getTransDB()->list()) {
        if (!m_runningUUID.contains(info.identifier())) {
            info.setIdentifier(QString());
        }
        m_list.append(info);
    }
    emit resumablesChanged(m_list);
}

DLTaskAccessMgr::~DLTaskAccessMgr()
{
    disconnect(getTransDB(), 0, 0, 0);
}

DLTask *DLTaskAccessMgr::get(const DLRequest &request)
{
    DLTaskImpl *task = new DLTaskImpl(request, getTransDB());
    ///FIXME append but not remove uuids may cause big-big length of QStringList
    if (!m_runningUUID.contains(task->uuid()))
        m_runningUUID.append(task->uuid());
    return task;
}

DLTaskInfoList DLTaskAccessMgr::resumables() const
{
    return m_list;
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
