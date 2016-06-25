#include "DLTaskPeer.h"

#include <QDebug>
#include <QFile>
#include <QReadWriteLock>
#include <QCryptographicHash>
#include <QSharedData>

#include <QNetworkReply>
#include <QNetworkRequest>

#include "DLTransmissionDatabaseKeys.h"

namespace YADownloader {

const static QString PEER_TAG = ".peer";

class PeerInfoPriv : public QSharedData
{
public:
    PeerInfoPriv()
        : startIndex(0)
        , endIndex(0)
        , completedCount(0)
        , filePath(QString())
    {
    }
    virtual ~PeerInfoPriv() {}
    quint64 startIndex;     //起始块
    quint64 endIndex;       //结束块
    quint64 completedCount; //完成块数
    QString filePath; //full path of file
};

PeerInfo::PeerInfo()
    : d(new PeerInfoPriv())
{

}

PeerInfo::PeerInfo(const PeerInfo &other)
    : d(other.d)
{

}

bool PeerInfo::operator ==(const PeerInfo &other) const {
    return d.data()->completedCount == other.d.data()->completedCount
            && d.data()->endIndex == other.d.data()->endIndex
            && d.data()->filePath == other.d.data()->filePath
            && d.data()->startIndex == other.d.data()->startIndex;
}

bool PeerInfo::operator !=(const PeerInfo &other) {
    return !operator == (other);
}

PeerInfo &PeerInfo::operator =(const PeerInfo &other) {
    if (this != &other)
        d.operator =(other.d);
    return *this;
}

bool PeerInfo::hasSameIdentifier(const PeerInfo &other)
{
    return d.data()->endIndex == other.d.data()->endIndex
            && d.data()->filePath == other.d.data()->filePath
            && d.data()->startIndex == other.d.data()->startIndex;
}

quint64 PeerInfo::startIndex() const
{
    return d.data()->startIndex;
}

void PeerInfo::setStartIndex(const quint64 &value)
{
    d.data()->startIndex = value;
}

quint64 PeerInfo::endIndex() const
{
    return d.data()->endIndex;
}

void PeerInfo::setEndIndex(const quint64 &value)
{
    d.data()->endIndex = value;
}

QString PeerInfo::filePath() const
{
    return d.data()->filePath;
}

void PeerInfo::setFilePath(const QString &value)
{
    d.data()->filePath = value;
}

quint64 PeerInfo::completedCount() const
{
    return d.data()->completedCount;
}

void PeerInfo::setCompletedCount(const quint64 &value)
{
    d.data()->completedCount = value;
}

quint64 PeerInfo::dlCompleted() const {
    return d.data()->completedCount > d.data()->endIndex - d.data()->startIndex+1
            ? d.data()->endIndex - d.data()->startIndex+1
            : d.data()->completedCount;
}

quint64 PeerInfo::rangeStart() const {
    return d.data()->startIndex + dlCompleted();
}

QDebug operator <<(QDebug dbg, const YADownloader::PeerInfo &info)
{
//    return dbg<<QString("Peer [start=%1],[end=%2],[complete=%3], [path=%4]")
//                .arg(info.startIndex()).arg(info.endIndex())
//                .arg(QString::number(info.completedCount())).arg(info.filePath());
    QVariantMap map;
    map.insert(TASK_PEER_START_IDX, info.startIndex());
    map.insert(TASK_PEER_END_IDX, info.endIndex());
    map.insert(TASK_PEER_COMPLETED_CNT, info.completedCount());
    return dbg<<map;
}

DLTaskPeer::DLTaskPeer(int index, const PeerInfo &info, QNetworkReply *reply, QObject *parent)
    : QObject(parent)
    , m_file(new QFile(this))
    , m_peerInfo(info)
    , m_reply(reply)
    , m_index(index)
    , m_doneCount(0)
{
    qDebug()<<Q_FUNC_INFO<<"peer index ["<<m_index<<"] for peer "<<m_peerInfo;

    m_file->setFileName (m_peerInfo.filePath() + PEER_TAG);
    if (!m_file->open (QIODevice::ReadWrite)) {
        qDebug()<<Q_FUNC_INFO<<"open error for "<<m_file->fileName ();
    }

    connect (m_reply, &QNetworkReply::readyRead, [&](){
        if (m_reply->bytesAvailable () > 4096) {
            qDebug()<<Q_FUNC_INFO<<"index ["<<m_index<<"] readyRead bytes "<<(m_reply)->bytesAvailable ();
            QByteArray qba = m_reply->readAll ();
            quint64 pos = m_peerInfo.rangeStart ()+doneCount ();
            qDebug()<<Q_FUNC_INFO<<"index ["<<m_index<<"] file seek to "<<pos;
            m_lock.lockForWrite ();
            qDebug()<<Q_FUNC_INFO<<"index ["<<m_index<<"] lockForWrite ";
            m_file->seek (pos);
            m_file->write (qba);
            m_lock.unlock ();
            qDebug()<<Q_FUNC_INFO<<"index ["<<m_index<<"] setDoneCount "<<doneCount() + qba.size();
            setDoneCount (doneCount () + qba.size ());
        }
    });

    connect (m_reply, &QNetworkReply::finished, [&]() {
        qDebug()<<Q_FUNC_INFO<<"finished index ["<<m_index<<"] readyRead bytes "<<(m_reply)->bytesAvailable ();
        QByteArray qba = m_reply->readAll ();
        quint64 pos = m_peerInfo.rangeStart ()+doneCount ();
        qDebug()<<Q_FUNC_INFO<<"file seek to "<<pos;
        m_lock.lockForWrite ();
        m_file->seek (pos);
        m_file->write (qba);
        m_lock.unlock ();
        setDoneCount (doneCount () + qba.size ());
        m_file->flush ();
        m_file->close ();
    });
}

DLTaskPeer::~DLTaskPeer()
{

}

PeerInfo DLTaskPeer::info() const {
    return m_peerInfo;
}

QByteArray DLTaskPeer::hash() const
{
    QString data = QString::number(m_peerInfo.startIndex())
            + QString::number(m_peerInfo.endIndex())
            + m_reply->request().url().toString();
    return QCryptographicHash::hash (data.toUtf8 (), QCryptographicHash::Sha1);
}

QUrl DLTaskPeer::downloadUrl() const
{
    if (!m_reply)
        return QUrl();
    return m_reply->request ().url ();
}

quint64 DLTaskPeer::doneCount() const {
    return m_doneCount;
}

void DLTaskPeer::setDoneCount(const quint64 &doneCount) {
    m_doneCount = doneCount;
}

//QNetworkReply *DLTaskPeer::reply() {
//    return m_reply;
//}

int DLTaskPeer::index() {
    return m_index;
}

void DLTaskPeer::abort()
{
    m_reply->abort();
}



} //YADownloader
