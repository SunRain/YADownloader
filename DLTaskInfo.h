#ifndef DLTASKINFO_H
#define DLTASKINFO_H

#include <QDebug>
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

    ///
    /// \brief isEmpty
    /// \return true if downloadUrl or requestUrl or filePath or peerList is empty
    ///
    bool isEmpty() const;

    //TODO
    ///
    /// \brief hasSameIdentifier
    /// \param other
    /// \return if has same identifier.
    /// The identifier tuple is composed of the
    ///
    bool hasSameIdentifier(const PeerInfo &other);

    QString hash() const;

    QString downloadUrl() const;

    QString requestUrl() const;

    QString filePath() const;

    qint64 totalSize() const;

    quint64 readySize() const;

    PeerInfoList peerList() const;

    void setHash(const QString &hash);

    void setDownloadUrl(const QString &downloadUrl);

    void setRequestUrl(const QString &requestUrl);

    void setFilePath(const QString &filePath);

    void setTotalSize(qint64 totalSize);

    void setReadySize(quint64 readySize);

    void setPeerList(const PeerInfoList &peerList);

private:
    QSharedPointer<DLTaskInfoPriv> d;
};

QDebug operator <<(QDebug dbg, const DLTaskInfo& info);


} //YADownloader
#endif // DLTASKINFO_H
