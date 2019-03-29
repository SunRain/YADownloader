#include "DLTaskPeer.h"

#include <QDebug>
#include <QFile>
#include <QReadWriteLock>
#include <QCryptographicHash>
#include <QSharedData>

//#include <QNetworkReply>
//#include <QNetworkRequest>
#include "QCNetworkAccessManager.h"
#include "QCNetworkRequest.h"
#include "QCNetworkAsyncReply.h"

#include "DLTransmissionDatabaseKeys.h"
#include "DLTaskStateDispatch.h"

using namespace QCurl;

namespace YADownloader {

//class DLTaskPeerInfoPriv : public QSharedData
//{
//public:
//    DLTaskPeerInfoPriv()
//        : startIndex(0)
//        , endIndex(0)
//        , completedCount(0)
//        , filePath(QString())
//    {
//    }
//    virtual ~DLTaskPeerInfoPriv() {}
//    quint64 startIndex;     //起始块
//    quint64 endIndex;       //结束块
//    quint64 completedCount; //完成块数
//    QString filePath; //full path of file
//};

DLTaskPeerInfo::DLTaskPeerInfo()
    : d(new DLTaskPeerInfoPriv())
{

}

DLTaskPeerInfo::DLTaskPeerInfo(const DLTaskPeerInfo &other)
    : d(other.d)
{

}

bool DLTaskPeerInfo::operator ==(const DLTaskPeerInfo &other) const {
    return d.data()->completedCount == other.d.data()->completedCount
            && d.data()->endIndex == other.d.data()->endIndex
            && d.data()->filePath == other.d.data()->filePath
            && d.data()->startIndex == other.d.data()->startIndex;
}

bool DLTaskPeerInfo::operator !=(const DLTaskPeerInfo &other) const
{
    return !operator == (other);
}

DLTaskPeerInfo &DLTaskPeerInfo::operator =(const DLTaskPeerInfo &other)
{
    if (this != &other)
        d.operator =(other.d);
    return *this;
}

bool DLTaskPeerInfo::hasSameIdentifier(const DLTaskPeerInfo &other) const
{
    return d.data()->endIndex == other.d.data()->endIndex
            && d.data()->filePath == other.d.data()->filePath
            && d.data()->startIndex == other.d.data()->startIndex;
}

quint64 DLTaskPeerInfo::startIndex() const
{
    return d.data()->startIndex;
}

void DLTaskPeerInfo::setStartIndex(const quint64 &value)
{
    d.data()->startIndex = value;
}

quint64 DLTaskPeerInfo::endIndex() const
{
    return d.data()->endIndex;
}

void DLTaskPeerInfo::setEndIndex(const quint64 &value)
{
    d.data()->endIndex = value;
}

QString DLTaskPeerInfo::filePath() const
{
    return d.data()->filePath;
}

void DLTaskPeerInfo::setFilePath(const QString &value)
{
    d.data()->filePath = value;
}

quint64 DLTaskPeerInfo::completedCount() const
{
    return d.data()->completedCount;
}

void DLTaskPeerInfo::setCompletedCount(const quint64 &value)
{
    d.data()->completedCount = value;
}

quint64 DLTaskPeerInfo::dlCompleted() const {
    return d.data()->completedCount > d.data()->endIndex - d.data()->startIndex+1
            ? d.data()->endIndex - d.data()->startIndex+1
            : d.data()->completedCount;
}

quint64 DLTaskPeerInfo::rangeStart() const {
    return d.data()->startIndex + dlCompleted();
}

/**************************************************************************************************
 *                                                                                                *
 **************************************************************************************************/

QDebug operator <<(QDebug dbg, const YADownloader::DLTaskPeerInfo &info)
{
    return dbg<<QString("Peer [start=%1],[end=%2],[complete=%3], [rangeStart=%4], [blockSize=%5], [path=%6]")
                .arg(info.startIndex())
                .arg(info.endIndex())
                .arg(QString::number(info.dlCompleted()))
                .arg(QString::number(info.rangeStart()))
                .arg(QString::number(info.endIndex()-info.startIndex()+1))
                .arg(info.filePath());
}

/**************************************************************************************************
 *                                                                                                *
 **************************************************************************************************/

DLTaskPeer::DLTaskPeer(DLTaskStateDispatch *dispatch, const DLTaskPeerInfo &info, QCNetworkAsyncReply *reply, QObject *parent)
    : QObject(parent)
    , m_file(new QFile(this))
    , m_reply(reply)
    , m_dispatch(dispatch)
    , m_peerInfo(info)
    , m_doneCount(0)
    , m_peerSize(0)
    , m_replyFinish(false)
    , m_headerHack(true)
    , m_headerSize(-1)
{
    qDebug()<<Q_FUNC_INFO<<" info is "<<m_peerInfo;

    m_file->setFileName (m_peerInfo.filePath());
    if (!m_file->open(QIODevice::ReadWrite)) {
        qCritical()<<Q_FUNC_INFO<<"open error for "<<m_file->fileName();
        m_replyFinish = true;
        m_reply->abort();

        m_dispatch->dispatchDownloadStatus(m_hash, DLStatusEvent::DLStatus::DL_FAILURE, true);
        return;
    }

    QString data = QString::number(m_peerInfo.startIndex())
            + QString::number(m_peerInfo.endIndex())
            + m_reply->request().url().toString();
    m_hash = QString(QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha1).toHex());
    m_peerSize = info.endIndex() - info.startIndex() +1;

    qDebug()<<Q_FUNC_INFO<<"peer hash ["<<m_hash<<"] for peer "<<m_peerInfo<<" start pos "<<info.rangeStart();

    connect (m_reply, &QCNetworkAsyncReply::readyRead, [&](){
//        int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        //        qDebug()<<Q_FUNC_INFO<<"readyRead hash ["<<m_hash<<"] statusCode "<<statusCode;
        if (m_headerHack) {
            m_headerHack = !m_headerHack;
            m_headerSize = m_reply->rawHeaderData().size();
            return;
        }
        const int hs = m_reply->rawHeaderData().size();
        if (hs != m_headerSize) {
            qDebug()<<Q_FUNC_INFO<<" find header size "<<hs<<" previous size "<<m_headerSize;
            m_headerSize = hs;
            return;
        }

        ///TODO check statusCode to see if download succeed or url redirect
//        const NetworkError error = m_reply->error();
//        if (error == NetworkNoError) {
            if (m_reply->bytesAvailable () > 4096) {
                qDebug()<<Q_FUNC_INFO<<"--------------- QCNetworkAsyncReply::readyRead";

                //            qDebug()<<Q_FUNC_INFO<<"hash ["<<m_hash<<"] readyRead bytes "<<(m_reply)->bytesAvailable ();
                QByteArray qba = m_reply->readAll ();
                QByteArray hba = m_reply->rawHeaderData();
                const int idx = qba.indexOf(hba);
//                qDebug()<<Q_FUNC_INFO<<" head data size "<<hba.size()<<" data "<<QString(hba);
//                qDebug()<<Q_FUNC_INFO<<" data of content size "<<qba.size()<<" data "<<QString(qba);
                if (idx >= 0) {
//                    qDebug()<<Q_FUNC_INFO<<"try remove header from idx ["<<idx<<"], size ["<<hba.size()<<"]";
                    qba = qba.remove(idx, hba.size());
                }

                quint64 pos = m_peerInfo.rangeStart ()+doneCount ();
                //            qDebug()<<Q_FUNC_INFO<<"  hash ["<<m_hash<<"] file seek to "<<pos<<" write "<<qba.size();
                m_fileLocker.lockForWrite ();
                //            qDebug()<<Q_FUNC_INFO<<"hash ["<<m_hash<<"] lockForWrite ";
                m_file->seek (pos);
                m_file->write (qba, qba.size());
                m_file->flush();
                m_fileLocker.unlock ();
                setDoneCount(doneCount() + qba.size());
                m_dispatch->dispatchDownloadProgress(m_hash, qba.size(), doneCount()+m_peerInfo.dlCompleted(), m_peerSize);
                //            qDebug()<<Q_FUNC_INFO<<"++++ hash "<<m_hash<<" DoneCount "<<doneCount()
                //                               <<" total "<<m_peerInfo.endIndex()-m_peerInfo.startIndex()+1;
            }
//        } else {
//            qWarning()<<Q_FUNC_INFO<<QString("Download peer [%1] Http header error code [%2], string: %3")
//                        .arg(m_hash).arg(error).arg(m_reply->errorString());
//            m_fileLocker.lockForWrite ();
//            m_file->flush();
//            m_fileLocker.unlock ();
//            m_file->close();
//        }
    });

    connect (m_reply, &QCNetworkAsyncReply::finished, [&]() {
        m_replyFinish = true;
//        int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        //        qDebug()<<Q_FUNC_INFO<<"finished hash ["<<m_hash<<"] statusCode "<<statusCode;
        const NetworkError error = m_reply->error();
        const int hs = m_reply->rawHeaderData().size();
//        qDebug()<<Q_FUNC_INFO<<"--------------- QCNetworkAsyncReply::finished, header size "<<hs;
        if (error != NetworkNoError) {
            qWarning()<<Q_FUNC_INFO<<QString("[QCNetworkAsyncReply::finished] Download peer [%1] Http header error code [%2], string: %3")
                        .arg(m_hash).arg(error).arg(m_reply->errorString());
            m_fileLocker.lockForWrite ();
            m_file->flush();
            m_fileLocker.unlock ();
            m_file->close();
            /// if current peer downloaded finished, we'll send finish signal
            /// otherwise we confront some http error
            if (m_peerInfo.dlCompleted() == (m_peerInfo.endIndex() - m_peerInfo.startIndex() +1)) {
                m_dispatch->dispatchDownloadStatus(m_hash, DLStatusEvent::DLStatus::DL_FINISH, true);
            } else { /// download error
                m_dispatch->dispatchDownloadStatus(m_hash, DLStatusEvent::DLStatus::DL_FAILURE, true);
            }
        } else {
            QByteArray qba = m_reply->readAll();
            QByteArray hba = m_reply->rawHeaderData();
            const int idx = qba.indexOf(hba);

//            qDebug()<<Q_FUNC_INFO<<" head data size "<<hba.size()<<" data "<<QString(hba);
//            qDebug()<<Q_FUNC_INFO<<" data of content size "<<qba.size()<<" data "<<QString(qba);
            if (idx >= 0) {
//                qDebug()<<Q_FUNC_INFO<<"try remove header from idx ["<<idx<<"], size ["<<hba.size()<<"]";
                qba = qba.remove(idx, hba.size());
            }
            quint64 pos = m_peerInfo.rangeStart ()+doneCount ();
            //        qDebug()<<Q_FUNC_INFO<<"hash ["<<m_hash<<"] file seek to "<<pos<<" write "<<qba.size();
            m_fileLocker.lockForWrite ();
            m_file->seek (pos);
            m_file->write (qba);
            m_file->flush ();
            //        qDebug()<<Q_FUNC_INFO<<"  now file size "<<m_file->size();
            m_fileLocker.unlock ();
            setDoneCount(doneCount () + qba.size ());
            m_file->close ();
            m_dispatch->dispatchDownloadProgress(m_hash, qba.size(), doneCount()+m_peerInfo.dlCompleted(), m_peerSize);
            m_dispatch->dispatchDownloadStatus(m_hash, DLStatusEvent::DLStatus::DL_FINISH, true);
        }
//        m_reply->deleteLater();
//        m_reply = Q_NULLPTR;
    });
    m_reply->perform();
}

DLTaskPeer::~DLTaskPeer()
{
    qDebug()<<Q_FUNC_INFO<<" ========= "<<m_hash;
    if (m_reply) {
        if (m_reply->isRunning()) {
            m_reply->abort();
        }
        QObject::disconnect(m_reply, 0, 0, 0);
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_file) {
        m_file->flush();
        m_file->close();
        m_file->deleteLater();
    }
    m_file = nullptr;
    m_dispatch = nullptr;
}

DLTaskPeerInfo DLTaskPeer::info() const {
    return m_peerInfo;
}

QString DLTaskPeer::hash() const
{
    return m_hash;
}

QUrl DLTaskPeer::downloadUrl() const
{
    if (!m_reply)
        return QUrl();
    return m_reply->request ().url ();
}

qint64 DLTaskPeer::doneCount() const {
    return m_doneCount;
}

bool DLTaskPeer::isFinished() const
{
    return !m_reply->isRunning() && m_replyFinish;
}

void DLTaskPeer::setDoneCount(const quint64 &doneCount) {
    m_locker.lock();
    m_doneCount = doneCount;
    m_locker.unlock();
}

void DLTaskPeer::abort()
{
    m_reply->abort();
}

} //YADownloader
