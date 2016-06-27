#include "DLTaskInfo.h"

#include <QSharedData>

#include "DLTaskPeer.h"
#include "DLTransmissionDatabaseKeys.h"

namespace YADownloader {

class DLTaskInfoPriv : public QSharedData
{
public:
    DLTaskInfoPriv()
//        : uid(QString())
        : downloadUrl(QString())
        , requestUrl(QString())
        , filePath(QString())
        , totalSize(0)
        , readySize(0)
    {
    }
    virtual ~DLTaskInfoPriv() {}

//    QString uid;
    QString downloadUrl;
    QString requestUrl;
    QString filePath;
    qint64 totalSize;
    quint64 readySize;
    DLTaskPeerInfoList peerList;
};

DLTaskInfo::DLTaskInfo()
    : d(new DLTaskInfoPriv)
{

}

DLTaskInfo::DLTaskInfo(const DLTaskInfo &other)
    : d(other.d)
{

}

bool DLTaskInfo::operator ==(const DLTaskInfo &other) const
{
    return d.data()->downloadUrl == other.d.data()->downloadUrl
            && d.data()->peerList == other.d.data()->peerList
            && d.data()->readySize == other.d.data()->readySize
            && d.data()->requestUrl == other.d.data()->requestUrl
            && d.data()->totalSize == other.d.data()->totalSize
//            && d.data()->uid == other.d.data()->uid
            && d.data()->filePath == other.d.data()->filePath;
}

bool DLTaskInfo::operator !=(const DLTaskInfo &other)
{
    return !operator ==(other);
}

DLTaskInfo &DLTaskInfo::operator =(const DLTaskInfo &other)
{
    if (this != &other)
        d.operator =(other.d);
    return *this;
}

bool DLTaskInfo::isEmpty() const
{
    return d.data()->downloadUrl.isEmpty() || d.data()->requestUrl.isEmpty()
            || d.data()->filePath.isEmpty() || d.data()->peerList.isEmpty();
}

void DLTaskInfo::clear()
{
    d.data()->downloadUrl.clear();
    d.data()->filePath.clear();
    d.data()->peerList.clear();
    d.data()->readySize = 0;
    d.data()->totalSize = 0;
}

bool DLTaskInfo::hasSameIdentifier(const DLTaskPeerInfo &other)
{
    //TODO
    return false;
}

//QString DLTaskInfo::hash() const
//{
//    return d.data()->uid;
//}

QString DLTaskInfo::downloadUrl() const
{
    return d.data()->downloadUrl;
}

QString DLTaskInfo::requestUrl() const
{
    return d.data()->requestUrl;
}

QString DLTaskInfo::filePath() const
{
    return d.data()->filePath;
}

qint64 DLTaskInfo::totalSize() const
{
    return d.data()->totalSize;
}

quint64 DLTaskInfo::readySize() const
{
    return d.data()->readySize;
}

DLTaskPeerInfoList DLTaskInfo::peerList() const
{
    return d.data()->peerList;
}

//void DLTaskInfo::setHash(const QString &hash)
//{
//    d.data()->uid = hash;
//}

void DLTaskInfo::setDownloadUrl(const QString &downloadUrl)
{
    d.data()->downloadUrl = downloadUrl;
}

void DLTaskInfo::setRequestUrl(const QString &requestUrl)
{
    d.data()->requestUrl = requestUrl;
}

void DLTaskInfo::setFilePath(const QString &filePath)
{
    d.data()->filePath = filePath;
}

void DLTaskInfo::setTotalSize(qint64 totalSize)
{
    d.data()->totalSize = totalSize;
}

void DLTaskInfo::setReadySize(quint64 readySize)
{
    d.data()->readySize = readySize;
}

void DLTaskInfo::setPeerList(const DLTaskPeerInfoList &peerList)
{
//    d.data()->peerList.clear();
//    d.data()->peerList.append(peerList);
    d.data()->peerList = peerList;
}

QDebug operator <<(QDebug dbg, const YADownloader::DLTaskInfo &info)
{
    QVariantHash hash;
//    hash.insert(TASK_INFO_UID, info.hash());
    hash.insert(TASK_INFO_FILE_PATH, info.filePath());
    hash.insert(TASK_INFO_DL_URL, info.downloadUrl());
    hash.insert(TASK_INFO_REQ_URL, info.requestUrl());
    hash.insert(TASK_INFO_READY_SIZE, info.readySize());
    hash.insert(TASK_INFO_TOTAL_SIZE, info.totalSize());
    QVariantList list;
    foreach (DLTaskPeerInfo p, info.peerList()) {
        QVariantHash hash;
        hash.insert(TASK_PEER_START_IDX, p.startIndex());
        hash.insert(TASK_PEER_END_IDX, p.endIndex());
        hash.insert(TASK_PEER_COMPLETED_CNT, p.dlCompleted());
        list.append(hash);
    }
    hash.insert(TASK_INFO_PEER_LIST, list);
    return dbg<<hash;
}

} //YADownloader
