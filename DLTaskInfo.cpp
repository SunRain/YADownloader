#include "DLTaskInfo.h"

namespace YADownloader {

DLTaskInfo::DLTaskInfo()
{

}

QString DLTaskInfo::uid() const
{
    return m_uid;
}

QString DLTaskInfo::downloadUrl() const
{
    return m_downloadUrl;
}

QString DLTaskInfo::requestUrl() const
{
    return m_requestUrl;
}

QString DLTaskInfo::totalSize() const
{
    return m_totalSize;
}

QString DLTaskInfo::readySize() const
{
    return m_readySize;
}

PeerInfo DLTaskInfo::peerList() const
{
    return m_peerList;
}

int DLTaskInfo::peerSize() const
{
    return m_peerSize;
}

void DLTaskInfo::setUid(QString uid)
{
    if (m_uid == uid)
        return;

    m_uid = uid;
    emit uidChanged(uid);
}

void DLTaskInfo::setDownloadUrl(QString downloadUrl)
{
    if (m_downloadUrl == downloadUrl)
        return;

    m_downloadUrl = downloadUrl;
    emit downloadUrlChanged(downloadUrl);
}

void DLTaskInfo::setRequestUrl(QString requestUrl)
{
    if (m_requestUrl == requestUrl)
        return;

    m_requestUrl = requestUrl;
    emit requestUrlChanged(requestUrl);
}

void DLTaskInfo::setTotalSize(QString totalSize)
{
    if (m_totalSize == totalSize)
        return;

    m_totalSize = totalSize;
    emit totalSizeChanged(totalSize);
}

void DLTaskInfo::setReadySize(QString readySize)
{
    if (m_readySize == readySize)
        return;

    m_readySize = readySize;
    emit readySizeChanged(readySize);
}

void DLTaskInfo::setPeerList(QVariantList peerList)
{
    if (m_peerList == peerList)
        return;

    m_peerList = peerList;
    emit peerListChanged(peerList);
}

void DLTaskInfo::setPeerSize(int peerSize)
{
    if (m_peerSize == peerSize)
        return;

    m_peerSize = peerSize;
    emit peerSizeChanged(peerSize);
}

} //YADownloader
