#include "DLTaskAccessMgr.h"

#include <QDebug>

#include "DLTaskImpl.h"
#include "DLTransmissionDatabase.h"

namespace YADownloader {

DLTaskAccessMgr::DLTaskAccessMgr(QObject *parent)
    : QObject(parent)
    , m_transDB(new DLTransmissionDatabase(this))
{

}

DLTaskAccessMgr::~DLTaskAccessMgr()
{
    qDebug()<<Q_FUNC_INFO<<"=========";
    if (m_transDB)
        m_transDB->deleteLater();
    m_transDB = nullptr;
}

DLTask *DLTaskAccessMgr::get(const DLRequest &request)
{
    DLTaskImpl *task = new DLTaskImpl(request, m_transDB);
    return task;
}



} //YADownloader
