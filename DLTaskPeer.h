#ifndef DLTASKPEER_H
#define DLTASKPEER_H

#include <QObject>
#include <QReadWriteLock>
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

    quint64 completedCount() const;
    void setCompletedCount(const quint64 &value);

    quint64 dlCompleted() const;
    quint64 rangeStart() const;

private:
    QSharedPointer<PeerInfoPriv> d;
};

QDebug operator <<(QDebug dbg, const PeerInfo& info);

class YADOWNLOADERSHARED_EXPORT DLTaskPeer : public QObject
{
    Q_OBJECT
public:
    explicit DLTaskPeer(int index, const PeerInfo &info, QNetworkReply *reply, QObject *parent = 0);
    virtual ~DLTaskPeer();

    PeerInfo info() const;
    QByteArray hash() const;
    QUrl downloadUrl() const;

    quint64 doneCount() const;
    void setDoneCount(const quint64 &doneCount);
//    QNetworkReply *reply();
    int index();

public slots:
    void abort();
private:
    QFile *m_file;
    QNetworkReply *m_reply;

    PeerInfo m_peerInfo;
    QReadWriteLock m_lock;

    int m_index;
    quint64 m_doneCount;
};

typedef QList<DLTaskPeer*> PeerList;

} //YADownloader

#endif // DLTASKPEER_H
