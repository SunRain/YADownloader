#ifndef DLTASKPEER_H
#define DLTASKPEER_H

#include <QObject>
#include <QReadWriteLock>
#include <QMutex>
#include <QSharedDataPointer>
#include "yadownloader_global.h"

class QFile;
class QNetworkReply;
namespace YADownloader {

class DLTaskPeerInfo;
typedef QList<DLTaskPeerInfo> DLTaskPeerInfoList;

class DLTaskPeer;
typedef QList<DLTaskPeer*> DLTaskPeerList;


//class DLTaskPeerInfoPriv;
class YADOWNLOADERSHARED_EXPORT DLTaskPeerInfo {
public:
    DLTaskPeerInfo();
    DLTaskPeerInfo(const DLTaskPeerInfo &other);

    ///
    /// \brief operator ==
    /// \param other
    /// \return true if has same startIndex endIndex completedCount filePath
    ///
    bool operator ==(const DLTaskPeerInfo &other) const;
    bool operator !=(const DLTaskPeerInfo &other) const;
    DLTaskPeerInfo &operator =(const DLTaskPeerInfo &other);

    ///
    /// \brief hasSameIdentifier
    /// \param other
    /// \return if has same identifier.The identifier tuple is composed of the startIndex endIndex filePath
    ///
    bool hasSameIdentifier(const DLTaskPeerInfo &other);

    quint64 startIndex() const;
    void setStartIndex(const quint64 &value);

    quint64 endIndex() const;
    void setEndIndex(const quint64 &value);

    QString filePath() const;
    void setFilePath(const QString &value);

    void setCompletedCount(const quint64 &value);

    quint64 dlCompleted() const;
    quint64 rangeStart() const;

protected:
    quint64 completedCount() const;

private:
    class DLTaskPeerInfoPriv : public QSharedData
    {
    public:
        DLTaskPeerInfoPriv()
            : startIndex(0)
            , endIndex(0)
            , completedCount(0)
            , filePath(QString())
        {
        }
        virtual ~DLTaskPeerInfoPriv() {}
        quint64 startIndex;     //起始块
        quint64 endIndex;       //结束块
        quint64 completedCount; //完成块数
        QString filePath; //full path of file
    };
    QSharedDataPointer<DLTaskPeerInfoPriv> d;
};

QDebug operator <<(QDebug dbg, const DLTaskPeerInfo& info);

class DLTaskStateDispatch;
class YADOWNLOADERSHARED_EXPORT DLTaskPeer : public QObject
{
    Q_OBJECT
public:
    explicit DLTaskPeer(DLTaskStateDispatch *dispatch, const DLTaskPeerInfo &info, QNetworkReply *reply, QObject *parent = 0);
    virtual ~DLTaskPeer();

    ///
    /// \brief info
    /// \return  DLTaskPeerInfo for origin requested DLTaskPeerInfo
    ///
    DLTaskPeerInfo info() const;
    QString hash() const;
    QUrl downloadUrl() const;

    ///
    /// \brief doneCount
    /// \return downloaded bytes since start
    ///
    qint64 doneCount() const;

    ///
    /// \brief isFinished
    /// \return true when the download peer has finished or was aborted.
    ///
    bool isFinished() const;

protected:
    void setDoneCount(const quint64 &doneCount);

public slots:
    void abort();

private:
    QFile *m_file;
    QNetworkReply *m_reply;
    DLTaskStateDispatch *m_dispatch;

    DLTaskPeerInfo m_peerInfo;
    QReadWriteLock m_fileLocker;
    QMutex m_locker;
    QString m_hash;
    qint64 m_doneCount;
    qint64 m_peerSize;
};

typedef QList<DLTaskPeer*> DLTaskPeerList;

} //YADownloader

#endif // DLTASKPEER_H
