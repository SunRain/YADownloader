#include "DLTaskAccessMgr.h"

#include <QDebug>
#include <QMutex>
#include <QScopedPointer>
#include <QFileInfo>

#include "DLTaskImpl.h"
#include "DLTransmissionDatabase.h"

namespace YADownloader {

DLTaskAccessMgr::DLTaskAccessMgr(QObject *parent)
    : QObject(parent)
{
    connect(getTransDB(), &DLTransmissionDatabase::listChanged, [&]() {
        m_list.clear();
        foreach (DLTaskInfo info, getTransDB()->list()) {
//            if (!m_runningUUID.contains(info.identifier())) {
//                info.setIdentifier(QString());
//            }
            m_list.append(info);
        }
        emit resumablesChanged(m_list);
    });
    foreach (DLTaskInfo info, getTransDB()->list()) {
//        if (!m_runningUUID.contains(info.identifier())) {
//            info.setIdentifier(QString());
//        }
        m_list.append(info);
    }
    emit resumablesChanged(m_list);
}

DLTaskAccessMgr::~DLTaskAccessMgr()
{
    disconnect(getTransDB(), 0, 0, 0);
}

DLTask *DLTaskAccessMgr::get(const DLRequest &request, bool overrideByAccessMgrHeader)
{
    DLRequest req = request;
    if (overrideByAccessMgrHeader) {
        foreach (const QByteArray &key, m_headerList.keys()) {
            req.setRawHeader(key, m_headerList.value(key));
        }
    }
    DLTaskImpl *task = new DLTaskImpl(req, getTransDB());
    ///FIXME append but not remove uuids may cause big-big length of QStringList
    if (!m_runningUUID.contains(task->uuid()))
        m_runningUUID.append(task->uuid());
    return task;
}

DLTask *DLTaskAccessMgr::get(const QString &identifier)
{
    if (identifier.isEmpty())
        return nullptr;
    DLTaskInfo info;
    foreach (const DLTaskInfo &i, m_list) {
        if (i.identifier() == identifier) {
            info = i;
            break;
        }
    }
    if (info.isEmpty())
        return nullptr;
    DLTaskImpl *task = new DLTaskImpl(info, getTransDB(), m_headerList);
    ///FIXME append but not remove uuids may cause big-big length of QStringList
//    if (!m_runningUUID.contains(task->uuid()))
//        m_runningUUID.append(task->uuid());
    return task;


}


DLTaskInfoList DLTaskAccessMgr::resumables() const
{
    return m_list;
}

void DLTaskAccessMgr::setRawHeader(const QByteArray &name, const QByteArray &value)
{
    if (name.isEmpty())
        return;
    m_headerList.insert(name, value);
}

QHash<QByteArray, QByteArray> DLTaskAccessMgr::rawHeaders() const
{
    return m_headerList;
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
