#include "DLTaskInfo.h"

#include <QSharedData>

#include "DLTaskPeer.h"
#include "DLTransmissionDatabaseKeys.h"

namespace YADownloader {

class DLTaskInfoPriv : public QSharedData
{
public:
    DLTaskInfoPriv()
        : uid(QString())
        , downloadUrl(QString())
        , requestUrl(QString())
        , filePath(QString())
        , totalSize(0)
        , readySize(0)
    {
    }
    virtual ~DLTaskInfoPriv() {}

    QString uid;
    QString downloadUrl;
    QString requestUrl;
    QString filePath;
    quint64 totalSize;
    quint64 readySize;
    PeerInfoList peerList;
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
            && d.data()->uid == other.d.data()->uid
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

bool DLTaskInfo::hasSameIdentifier(const PeerInfo &other)
{
    //TODO
    return false;
}

QString DLTaskInfo::uid() const
{
    return d.data()->uid;
}

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

quint64 DLTaskInfo::totalSize() const
{
    return d.data()->totalSize;
}

quint64 DLTaskInfo::readySize() const
{
    return d.data()->readySize;
}

PeerInfoList DLTaskInfo::peerList() const
{
    return d.data()->peerList;
}

void DLTaskInfo::setUid(const QString &uid)
{
    d.data()->uid = uid;
}

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

void DLTaskInfo::setTotalSize(quint64 totalSize)
{
    d.data()->totalSize = totalSize;
}

void DLTaskInfo::setReadySize(quint64 readySize)
{
    d.data()->readySize = readySize;
}

void DLTaskInfo::setPeerList(const PeerInfoList &peerList)
{
    d.data()->peerList = peerList;
}

QDebug operator <<(QDebug dbg, const YADownloader::DLTaskInfo &info)
{
    QVariantMap map;
    map.insert(TASK_INFO_UID, info.uid());
    map.insert(TASK_INFO_FILE_PATH, info.filePath());
    map.insert(TASK_INFO_DL_URL, info.downloadUrl());
    map.insert(TASK_INFO_REQ_URL, info.requestUrl());
    map.insert(TASK_INFO_READY_SIZE, info.readySize());
    map.insert(TASK_INFO_TOTAL_SIZE, info.totalSize());
    QVariantList list;
    foreach (PeerInfo p, info.peerList()) {
        QVariantMap map;
        map.insert(TASK_PEER_START_IDX, p.startIndex());
        map.insert(TASK_PEER_END_IDX, p.endIndex());
        map.insert(TASK_PEER_COMPLETED_CNT, p.completedCount());
        list.append(map);
    }
    map.insert(TASK_INFO_PEER_LIST, map);
    return dbg<<map;
}

} //YADownloader
