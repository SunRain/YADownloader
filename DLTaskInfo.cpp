#include "DLTaskInfo.h"

#include <QSharedData>

#include "DLTaskPeer.h"
#include "DLTransmissionDatabaseKeys.h"

namespace YADownloader {

DLTaskInfo::DLTaskInfo()
    : d(new DLTaskInfoPriv())
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
            && d.data()->identifier == other.d.data()->identifier
            && d.data()->filePath == other.d.data()->filePath;
}

bool DLTaskInfo::operator !=(const DLTaskInfo &other) const
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
    d.data()->downloadUrl = QString();
    d.data()->filePath = QString();
    d.data()->identifier = QString();
    d.data()->peerList.clear();
    d.data()->readySize = 0;
    d.data()->totalSize = 0;
    d.data()->status = DLTaskInfo::TaskStatus::STATUS_STOP;
}

bool DLTaskInfo::hasSameIdentifier(const DLTaskInfo &other)
{
    bool flag = true;
    foreach (DLTaskPeerInfo info, d.data()->peerList) {
        DLTaskPeerInfoList list = other.peerList();
        flag = comparePeer(info, list);
        if (!flag)
            break;
    }
    return d.data()->downloadUrl == other.d.data()->downloadUrl
            && d.data()->filePath == other.d.data()->filePath
            && d.data()->requestUrl == other.d.data()->requestUrl
            && d.data()->totalSize == other.d.data()->totalSize
            && flag;
}

QString DLTaskInfo::identifier() const
{
    return d.data()->identifier;
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

qint64 DLTaskInfo::totalSize() const
{
    return d.data()->totalSize;
}

qint64 DLTaskInfo::readySize() const
{
    return d.data()->readySize;
}

DLTaskPeerInfoList DLTaskInfo::peerList() const
{
    return d.data()->peerList;
}

DLTaskInfo::TaskStatus DLTaskInfo::status() const
{
    return d.data()->status;
}

QVariantMap DLTaskInfo::toMap() const
{
    QVariantMap map;
    map.insert(TASK_INFO_FILE_PATH, d.data()->filePath);
    map.insert(TASK_INFO_DL_URL, d.data()->downloadUrl);
    map.insert(TASK_INFO_REQ_URL, d.data()->requestUrl);
    map.insert(TASK_INFO_READY_SIZE, d.data()->readySize);
    map.insert(TASK_INFO_TOTAL_SIZE, d.data()->totalSize);
    map.insert(TASK_INFO_STATUS, QString::number(d.data()->status));
    map.insert(TASK_INFO_IDENTIFIER, d.data()->identifier);
    QVariantList list;
    foreach (DLTaskPeerInfo p, d.data()->peerList) {
        QVariantMap map;
        map.insert(TASK_PEER_START_IDX, p.startIndex());
        map.insert(TASK_PEER_END_IDX, p.endIndex());
        map.insert(TASK_PEER_COMPLETED_CNT, p.dlCompleted());
        map.insert(TASK_INFO_FILE_PATH, p.filePath());
        list.append(map);
    }
    return map;
}

DLTaskInfo DLTaskInfo::fromMap(const QVariantMap &map)
{
    DLTaskInfo info;
    info.setDownloadUrl(map.value(TASK_INFO_DL_URL).toString());
    info.setFilePath(map.value(TASK_INFO_FILE_PATH).toString());
    info.setRequestUrl(map.value(TASK_INFO_REQ_URL).toString());
    info.setReadySize(map.value(TASK_INFO_READY_SIZE).toULongLong());
    info.setTotalSize(map.value(TASK_INFO_TOTAL_SIZE).toULongLong());
    info.setStatus((DLTaskInfo::TaskStatus)map.value(TASK_INFO_STATUS).toString().toInt());
    info.setIdentifier(map.value(TASK_INFO_IDENTIFIER).toString());
    QVariantList vaList = map.value(TASK_INFO_PEER_LIST).toList();
    DLTaskPeerInfoList plist;
    foreach (QVariant hh, vaList) {
        QVariantMap h = hh.toMap();
        DLTaskPeerInfo p;
        p.setStartIndex(h.value(TASK_PEER_START_IDX).toString().toULongLong());
        p.setEndIndex(h.value(TASK_PEER_END_IDX).toString().toULongLong());
        p.setCompletedCount(h.value(TASK_PEER_COMPLETED_CNT).toString().toULongLong());
        p.setFilePath(h.value(TASK_INFO_FILE_PATH).toString());
        plist.append(p);
    }
    info.setPeerList(plist);
    return info;
}

void DLTaskInfo::setIdentifier(const QString &identifier)
{
    d.data()->identifier = identifier;
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

void DLTaskInfo::setTotalSize(qint64 totalSize)
{
    d.data()->totalSize = totalSize;
}

void DLTaskInfo::setReadySize(qint64 readySize)
{
    d.data()->readySize = readySize;
}

void DLTaskInfo::setPeerList(const DLTaskPeerInfoList &peerList)
{
    d.data()->peerList = peerList;
}

void DLTaskInfo::setStatus(DLTaskInfo::TaskStatus status)
{
    d.data()->status = status;
}

bool DLTaskInfo::comparePeer(const DLTaskPeerInfo &p, const DLTaskPeerInfoList &list) const
{
    foreach (DLTaskPeerInfo info, list) {
        if (p.hasSameIdentifier(info))
            return true;
    }
    return false;
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
    hash.insert(TASK_INFO_IDENTIFIER, info.identifier());
    QVariantList list;
    foreach (DLTaskPeerInfo p, info.peerList()) {
        QVariantHash hash;
        hash.insert(TASK_PEER_START_IDX, p.startIndex());
        hash.insert(TASK_PEER_END_IDX, p.endIndex());
        hash.insert(TASK_PEER_COMPLETED_CNT, p.dlCompleted());
        list.append(hash);
    }
    hash.insert(TASK_INFO_STATUS, QString::number(info.status()));

    return dbg<<hash;
}

} //YADownloader
