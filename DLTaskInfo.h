#ifndef DLTASKINFO_H
#define DLTASKINFO_H

#include <QObject>
#include <QSharedPointer>

#include "yadownloader_global.h"
#include "DLTaskPeer.h"

namespace YADownloader {

class DLTaskInfo;
typedef QList<DLTaskInfo> DLTaskInfoList;

class DLTaskInfoPriv;
class YADOWNLOADERSHARED_EXPORT DLTaskInfo
{
public:
    DLTaskInfo();
    DLTaskInfo(const DLTaskInfo &other);

    bool operator ==(const DLTaskInfo &other) const;
    bool operator !=(const DLTaskInfo &other);
    DLTaskInfo &operator =(const DLTaskInfo &other);

    //TODO
    ///
    /// \brief hasSameIdentifier
    /// \param other
    /// \return if has same identifier.
    /// The identifier tuple is composed of the
    ///
    bool hasSameIdentifier(const PeerInfo &other);

    QString uid() const;

    QString downloadUrl() const;

    QString requestUrl() const;

    QString filePath() const;

    quint64 totalSize() const;

    quint64 readySize() const;

    PeerInfoList peerList() const;

    void setUid(const QString &uid);

    void setDownloadUrl(const QString &downloadUrl);

    void setRequestUrl(const QString &requestUrl);

    void setFilePath(const QString &filePath);

    void setTotalSize(quint64 totalSize);

    void setReadySize(quint64 readySize);

    void setPeerList(const PeerInfoList &peerList);

private:
    QSharedPointer<DLTaskInfoPriv> d;
};

} //YADownloader
#endif // DLTASKINFO_H
