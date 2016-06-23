#ifndef DLTASKINFO_H
#define DLTASKINFO_H

#include <QObject>

#include "yadownloader_global.h"
#include "DLTaskPeer.h"

namespace YADownloader {

class YADOWNLOADERSHARED_EXPORT DLTaskInfo
{
public:
    DLTaskInfo();

    QString uid() const;

    QString downloadUrl() const;

    QString requestUrl() const;

    QString totalSize() const;

    QString readySize() const;

    PeerInfo peerList() const;

    int peerSize() const;

    void setUid(QString uid);

    void setDownloadUrl(QString downloadUrl);

    void setRequestUrl(QString requestUrl);

    void setTotalSize(QString totalSize);

    void setReadySize(QString readySize);

    void setPeerList(QVariantList peerList);

    void setPeerSize(int peerSize);

private:
    QString m_uid;
    QString m_downloadUrl;
    QString m_requestUrl;
    QString m_totalSize;
    QString m_readySize;
    QVariantList m_peerList;
    int m_peerSize;
};

} //YADownloader
#endif // DLTASKINFO_H
