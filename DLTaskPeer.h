#ifndef DLTASKPEER_H
#define DLTASKPEER_H

#include <QObject>
#include <QReadWriteLock>
#include <QMutex>
#include <QSharedPointer>
#include "yadownloader_global.h"

class QFile;
class QNetworkReply;
namespace YADownloader {

class PeerInfo;
typedef QList<PeerInfo> PeerInfoList;

class DLTaskPeer;
typedef QList<DLTaskPeer*> PeerList;


class PeerInfoPriv;
class YADOWNLOADERSHARED_EXPORT PeerInfo {
public:
//    quint64 startIndex;     //起始块
//    quint64 endIndex;       //结束块
//    quint64 completedCount; //完成块数
//    QString filePath; //full path of file

    PeerInfo();
    PeerInfo(const PeerInfo &other);

    ///
    /// \brief operator ==
    /// \param other
    /// \return true if has same startIndex endIndex completedCount filePath
    ///
    bool operator ==(const PeerInfo &other) const;
    bool operator !=(const PeerInfo &other);
    PeerInfo &operator =(const PeerInfo &other);

    ///
    /// \brief hasSameIdentifier
    /// \param other
    /// \return if has same identifier.The identifier tuple is composed of the startIndex endIndex filePath
    ///
    bool hasSameIdentifier(const PeerInfo &other);

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
    QSharedPointer<PeerInfoPriv> d;
};

QDebug operator <<(QDebug dbg, const PeerInfo& info);

class DLTaskStateDispatch;
class YADOWNLOADERSHARED_EXPORT DLTaskPeer : public QObject
{
    Q_OBJECT
public:
    explicit DLTaskPeer(DLTaskStateDispatch *dispatch, const PeerInfo &info, QNetworkReply *reply, QObject *parent = 0);
    virtual ~DLTaskPeer();

    PeerInfo info() const;
    QString hash() const;
    QUrl downloadUrl() const;

    qint64 doneCount() const;

protected:
    void setDoneCount(const quint64 &doneCount);
//    QNetworkReply *reply();
//    int index();

//signals:
//    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

public slots:
    void abort();
private:
    QFile *m_file;
    QNetworkReply *m_reply;
    DLTaskStateDispatch *m_dispatch;

    PeerInfo m_peerInfo;
    QReadWriteLock m_fileLocker;
//    QMutex m_locker;

    QString m_hash;

//    int m_index;
    qint64 m_doneCount;
};

typedef QList<DLTaskPeer*> PeerList;

} //YADownloader

#endif // DLTASKPEER_H
