#include "DLTask.h"

#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QEventLoopLocker>
#include <QTimer>
#include <QCryptographicHash>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "DLRequest.h"
#include "SignalCenter.h"
#include "DLTaskPeer.h"
#include "DLTaskStateDispatch.h"
#include "DLTaskHeaderReader.h"
#include "DLTransmissionDatabase.h"

namespace YADownloader {

DLTask::DLTask(DLTransmissionDatabase *db, QObject *parent)
    : DLTask(db, DLRequest(), parent)
//    , m_networkMgr(new QNetworkAccessManager(this))
//    , m_reply(nullptr)
//    , m_workerThread(new QThread(this))
//    , m_headerReader(nullptr)
//    , m_dispatch(new DLTaskStateDispatch(this))
//    , m_transDB(db)
//    , m_dlRequest(DLRequest())
//    , m_DLStatus(DL_STOP)
//    , m_uid(QString())
//    , m_initHeaderCounts(3)
//    , m_peerCount(0)
//    , m_peerSize(0)
//    , m_totalSize(-1)
//    , m_downloadedSize(0)
{
//    connect (m_workerThread, &QThread::finished, [&]() {
//        qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<";
//        foreach (DLTaskPeer *p, m_peerList) {
//            p->reply()->abort();
//            p->deleteLater ();
//        }
//    });

//    connect (m_networkMgr, &QNetworkAccessManager::finished,
//             [&](QNetworkReply *reply) {
//        qDebug()<<Q_FUNC_INFO<<">>>>>>>>>>>>>>>>>>>>>>>";
//        foreach (DLTaskPeer *r, m_peerList) {
//            if (r->reply () == reply) {
//                qDebug()<<Q_FUNC_INFO<<"found peer "<<r->index ()<<" remain file size "<<reply->size ();
//            }
//        }
//    });

//    calculateUID();
}

DLTask::DLTask(DLTransmissionDatabase *db, const DLRequest &request, QObject *parent)
    : QObject(parent)
    , m_networkMgr(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_workerThread(new QThread(this))
    , m_headerReader(nullptr)
    , m_dispatch(new DLTaskStateDispatch(this))
    , m_transDB(db)
    , m_dlRequest(request)
    , m_DLStatus(DL_STOP)
    , m_uid(QString())
    , m_initHeaderCounts(3)
//    , m_peerCount(0)
//    , m_peerSize(0)
    , m_totalSize(-1)
    , m_downloadedSize(0)
//    , m_dlBytesWhenStarted(0)
    , m_dlBytesReceived(0)
{
    connect (m_workerThread, &QThread::finished, [&]() {
        qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<  m_workerThread finished";
        foreach (DLTaskPeer *p, m_peerList) {
            p->abort();
//            p->deleteLater();
        }
    });

//    connect (m_networkMgr, &QNetworkAccessManager::finished,
//             [&](QNetworkReply *reply) {
//        qDebug()<<Q_FUNC_INFO<<">>>>>>>>>>>>>>>>>>>>>>>";
//        foreach (DLTaskPeer *r, m_peerList) {
//            if (r->reply () == reply) {
//                qDebug()<<Q_FUNC_INFO<<"found peer "<<r->index ()<<" remain file size "<<reply->size ();
//            }
//        }
//    });

    calculateUID();
}

DLTask::~DLTask()
{
    if (m_workerThread->isRunning ()) {
        m_workerThread->quit ();
    }
    m_workerThread->wait ();
    m_workerThread->deleteLater ();
    m_workerThread = nullptr;

    foreach (DLTaskPeer *p, m_peerList) {
        p->abort();
    }

    saveInfo();

    qDeleteAll(m_peerList);
    m_peerList.clear();

    if (m_networkMgr) {
        m_networkMgr->deleteLater ();
        m_networkMgr = nullptr;
    }
    if (m_headerReader) {
        if (m_headerReader->isRunning())
            m_headerReader->abort();
        m_headerReader->deleteLater();
        m_headerReader = nullptr;
    }

}

void DLTask::setRequest(const DLRequest &request) {
    m_dlRequest = request;
    calculateUID();
}

DLRequest DLTask::request() const {
    return m_dlRequest;
}

QString DLTask::uid() const
{
    return m_uid;
}

bool DLTask::event(QEvent *event)
{
    if (event->type() == DLTASK_EVENT_FILE_SIZE) {
        qDebug()<<Q_FUNC_INFO<<"========== FileSizeEvent";
        FileSizeEvent *e = (FileSizeEvent*)event;
        m_totalSize = e->fileSize();
        qDebug()<<Q_FUNC_INFO<<" file size "<<m_totalSize;
        emit initFileSize(m_totalSize);
        if (m_DLStatus == DL_START) {
            if (!m_workerThread->isRunning ())
                m_workerThread->start ();
            if (m_peerList.isEmpty ()) {
                initPeers ();
            }
        }
        return true;
    } else if (event->type() == DLTASK_EVENT_DL_PROGRESS) {
        DLProgressEvent *e = (DLProgressEvent*)event;
        QString hash = e->hash();
        qint64 dlcnt = e->downloadedCount();
        m_downloadedSize += dlcnt;
        m_dlBytesReceived += e->bytesReceived();
        m_dlCompletedHash.insert(hash, dlcnt);
//        m_downloadedSize = 0;
        quint64 d =0;
        foreach (quint64 i, m_dlCompletedHash.values()) {
            d += i;
        }
        m_downloadedSize == d;
        qDebug()<<Q_FUNC_INFO<<" hash "<<hash<<" dlcnt "<<dlcnt<<" total "<<m_totalSize
               <<" bytesReceived "<<e->bytesReceived();

        qDebug()<<Q_FUNC_INFO<<"downloadProgress "<<m_downloadedSize<<" "<<m_totalSize
               <<" percent "<<(float)m_downloadedSize/(float)m_totalSize;
        emit downloadProgress(m_downloadedSize, m_totalSize);
        return true;
    }
    return QObject::event(event);
}

void DLTask::start()
{
    m_DLStatus = DL_START;
    if (!m_headerReader)
        m_headerReader = new DLTaskHeaderReader(&m_dlRequest, m_dispatch, this);
    if (m_headerReader->isRunning())
        m_headerReader->abort();
    m_headerReader->initFileSize();
}

void DLTask::abort()
{
    m_DLStatus = DL_STOP;
    foreach (DLTaskPeer *p, m_peerList) {
        p->abort();
    }
    saveInfo();
    qDeleteAll(m_peerList);
    m_peerList.clear();
}

void DLTask::initPeers()
{
    if (m_totalSize <= 0) { //We can't get downloaded file size, so we should use 1 thread to download
        m_dlRequest.setPreferThreadCount (1);
    }

    m_dlTaskInfo.setDownloadUrl(m_dlRequest.downloadUrl().toString());
    m_dlTaskInfo.setFilePath(m_dlRequest.filePath());
    m_dlTaskInfo.setReadySize(0);
    m_dlTaskInfo.setRequestUrl(m_dlRequest.requestUrl().toString());
    m_dlTaskInfo.setTotalSize(m_totalSize);
    m_dlTaskInfo.setHash(m_uid);

    foreach (DLTaskInfo info, m_transDB->list()) {
//        qDebug()<<Q_FUNC_INFO<<"+++++ info.hash "<<info.hash()<<" m_dlTaskInfo.hash() "<<m_dlTaskInfo.hash();
//        qDebug()<<Q_FUNC_INFO<<"+++++ info.totalSize "<<info.totalSize()<<" m_totalSize "<<m_totalSize;
        if (info.hash() == m_dlTaskInfo.hash()  //uid same means same requestUrl && filePath /*&& downloadUrl*/
//                && info.downloadUrl() == m_dlRequest.downloadUrl() //check if url redirected
                && info.totalSize() == m_totalSize) { //check if remote host file changed
            m_dlTaskInfo = info;
            break;
        }
    }
    PeerInfoList peerInfoList;
    if (m_dlTaskInfo.isEmpty()) { //download new file
//        qDebug()<<Q_FUNC_INFO<<"....................... m_dlTaskInfo.isEmpty";
        int threadCount = m_dlRequest.preferThreadCount ();
        quint64 step = m_totalSize/threadCount;
        quint64 start = -step;
        quint64 end = 0;
        for (int index=1; index <= threadCount; ++index) {
            end += step;
            start += step;
            if (index == threadCount) {
                end = m_totalSize;
            }
            PeerInfo pInfo;
            pInfo.setStartIndex(start);
            pInfo.setEndIndex(end-1);
            pInfo.setCompletedCount(0);
            pInfo.setFilePath(m_dlRequest.filePath());
            peerInfoList.append(pInfo);
        }
//        m_dlTaskInfo.setDownloadUrl(m_dlRequest.downloadUrl().toString());
//        m_dlTaskInfo.setFilePath(m_dlRequest.filePath());
        m_dlTaskInfo.setPeerList(peerInfoList);
//        m_dlTaskInfo.setReadySize(0);
//        m_dlTaskInfo.setRequestUrl(m_dlRequest.requestUrl().toString());
//        m_dlTaskInfo.setTotalSize(m_totalSize);
//        m_dlTaskInfo.setHash(m_uid);
        m_transDB->appendTaskInfo(m_dlTaskInfo);
        m_transDB->flush();
    } else { // continue to download
        peerInfoList = m_dlTaskInfo.peerList();
        m_dlRequest.setPreferThreadCount(peerInfoList.size());
    }

//    int index = 0;
    ///
    /// reset m_downloadedSize to 0, thus we can calculate downloaded size from configuration file
    m_downloadedSize = 0;
    foreach (PeerInfo info, peerInfoList) {
        m_downloadedSize += info.dlCompleted();

        QNetworkRequest req(m_dlRequest.downloadUrl ());
        if (!m_dlRequest.rawHeaders ().isEmpty ()) {
            foreach (QByteArray key, m_dlRequest.rawHeaders ().keys ()) {
//                QString value = m_dlRequest.rawHeaders().value(key, QByteArray());
//                qDebug()<<Q_FUNC_INFO<<QString("insert header [%1] = [%2]").arg(QString(key)).arg(value);
                req.setRawHeader(key, m_dlRequest.rawHeaders().value(key, QByteArray()));
            }
        }
        QString range = QString("bytes=%1-%2").arg(info.rangeStart()).arg(info.endIndex());
        req.setRawHeader ("Range", range.toUtf8 ());
        req.setRawHeader ("Connection", "keep-alive");

        QNetworkReply *tmp = m_networkMgr->get (req);
        if (!tmp) {
            qCritical()<<Q_FUNC_INFO<<"No QNetworkReply， download cant start";
            return;
        }
        DLTaskPeer *peer = new DLTaskPeer(m_dispatch, info, tmp, 0);
        peer->moveToThread (m_workerThread);
        m_dlCompletedHash.insert(peer->hash(), info.dlCompleted());
        m_peerList.append(peer);
    }
    if (!m_workerThread->isRunning())
        m_workerThread->start();
}

void DLTask::calculateUID()
{
    QString str = m_dlRequest.requestUrl().toString() + m_dlRequest.filePath()/* + m_dlRequest.downloadUrl().toString()*/;
    m_uid = QString(QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
}

void DLTask::saveInfo()
{
    PeerInfoList list;
    foreach (DLTaskPeer *peer, m_peerList) {
        qint64 done = m_dlCompletedHash.value(peer->hash());
        PeerInfo info = peer->info();
        info.setCompletedCount(done);
        list.append(info);
    }
    qDebug()<<Q_FUNC_INFO<<list;

    m_dlTaskInfo.setPeerList(list);
    m_transDB->appendTaskInfo(m_dlTaskInfo);
    m_transDB->flush();
}





























} //YADownloader
