#ifndef DLTASKINFO_H
#define DLTASKINFO_H

#include <QDebug>
#include <QObject>
#include <QSharedDataPointer>

#include "yadownloader_global.h"
#include "DLTaskPeer.h"

namespace YADownloader {

class DLTaskInfo;
typedef QList<YADownloader::DLTaskInfo> DLTaskInfoList;

//class DLTaskInfoPriv;
class YADOWNLOADERSHARED_EXPORT DLTaskInfo
{
public:
    DLTaskInfo();
    DLTaskInfo(const DLTaskInfo &other);

    bool operator ==(const DLTaskInfo &other) const;
    bool operator !=(const DLTaskInfo &other) const;
    DLTaskInfo &operator =(const DLTaskInfo &other);

    ///
    /// \brief isEmpty
    /// \return true if downloadUrl or requestUrl or filePath or peerList is empty
    ///
    bool isEmpty() const;

    ///
    /// \brief clear
    /// This will clear DLTaskInfo except requestUrl value;
    ///
    void clear();

    //TODO
    ///
    /// \brief hasSameIdentifier
    /// \param other
    /// \return if has same identifier.
    /// The identifier tuple is composed of the
    ///
    bool hasSameIdentifier(const DLTaskPeerInfo &other);

//    QString hash() const;

    QString downloadUrl() const;

    QString requestUrl() const;

    QString filePath() const;

    qint64 totalSize() const;

    qint64 readySize() const;

    DLTaskPeerInfoList peerList() const;

//    void setHash(const QString &hash);

    void setDownloadUrl(const QString &downloadUrl);

    void setRequestUrl(const QString &requestUrl);

    void setFilePath(const QString &filePath);

    void setTotalSize(qint64 totalSize);

    void setReadySize(qint64 readySize);

    void setPeerList(const DLTaskPeerInfoList &peerList);

private:
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
        qint64 readySize;
        DLTaskPeerInfoList peerList;
    };
    QSharedDataPointer<DLTaskInfoPriv> d;
};

QDebug operator <<(QDebug dbg, const DLTaskInfo& info);


} //YADownloader
#endif // DLTASKINFO_H
