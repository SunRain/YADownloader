#include "DLTaskStateDispatch.h"

#include <QCoreApplication>

namespace YADownloader {

FileSizeEvent::FileSizeEvent(qint64 fileSize)
    : QEvent((QEvent::Type)DLTASK_EVENT_FILE_SIZE)
    , m_fileSize(fileSize)
{

}



qint64 FileSizeEvent::fileSize() const
{
    return m_fileSize;
}

DLProgressEvent::DLProgressEvent(const QString &hash, qint64 bytesReceived, qint64 downloadedCnt, qint64 bytesTotal)
    : QEvent((QEvent::Type)DLTASK_EVENT_DL_PROGRESS)
    , m_hash(hash)
    , m_br(bytesReceived)
    , m_bt(bytesTotal)
    , m_dlCnt(downloadedCnt)
{
}

QString DLProgressEvent::hash() const
{
    return m_hash;
}

qint64 DLProgressEvent::bytesReceived() const
{
    return m_br;
}

qint64 DLProgressEvent::bytesTotal() const
{
    return m_bt;
}

qint64 DLProgressEvent::downloadedCount() const
{
    return m_dlCnt;
}

DLStatusEvent::DLStatusEvent(const QString &hash, DLStatus status, bool isTaskPeerEvent)
    : QEvent((QEvent::Type)DLTASK_EVENT_DL_STATUS)
    , m_isTaskPeer(isTaskPeerEvent)
    , m_hash(hash)
    , m_status(status)
{

}

bool DLStatusEvent::isTaskPeeEvent() const
{
    return m_isTaskPeer;
}

QString DLStatusEvent::hash() const
{
    return m_hash;
}

DLStatusEvent::DLStatus DLStatusEvent::status() const
{
    return m_status;
}

DLTaskStateDispatch::DLTaskStateDispatch(QObject *parent)
    : QObject(parent)
    , m_locker(QMutex::Recursive)
{

}

DLTaskStateDispatch::~DLTaskStateDispatch()
{

}

void DLTaskStateDispatch::dispatchFileSize(qint64 fileSize)
{
    qApp->postEvent(parent(), new FileSizeEvent(fileSize));
}

void DLTaskStateDispatch::dispatchDownloadProgress(const QString &hash, qint64 bytesReceived,
                                                   qint64 downloadedCnt, qint64 bytesTotal)
{
    m_locker.lock();
    qApp->postEvent(parent(), new DLProgressEvent(hash, bytesReceived, downloadedCnt, bytesTotal));
    m_locker.unlock();
}

void DLTaskStateDispatch::dispatchDownloadStatus(const QString &hash, DLStatusEvent::DLStatus status, bool isTaskPeerEvent)
{
    m_locker.lock();
    qApp->postEvent(parent(), new DLStatusEvent(hash, status, isTaskPeerEvent));
    m_locker.unlock();
}




} //YADownloader
