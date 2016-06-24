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
    , m_peerCount(0)
    , m_peerSize(0)
    , m_totalSize(-1)
    , m_downloadedSize(0)
{
    connect (m_workerThread, &QThread::finished, [&]() {
        qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<";
        foreach (DLTaskPeer *p, m_peerList) {
            p->reply()->abort();
            p->deleteLater ();
        }
    });

    connect (m_networkMgr, &QNetworkAccessManager::finished,
             [&](QNetworkReply *reply) {
        qDebug()<<Q_FUNC_INFO<<">>>>>>>>>>>>>>>>>>>>>>>";
        foreach (DLTaskPeer *r, m_peerList) {
            if (r->reply () == reply) {
                qDebug()<<Q_FUNC_INFO<<"found peer "<<r->index ()<<" remain file size "<<reply->size ();
            }
        }
    });

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

void DLTask::initPeers()
{
    if (m_totalSize <= 0) { //We can't get downloaded file size, so we should use 1 thread to download
        m_dlRequest.setPreferThreadCount (1);
    }

//    DLTransmissionDatabase *db = DLTransmissionDatabase::instance();
    DLTaskInfo task;
    foreach (DLTaskInfo info, m_transDB->list()) {
        if (info.uid() == m_uid  //uid same means same requestUrl && filePath && downloadUrl
//                && info.downloadUrl() == m_dlRequest.downloadUrl() //check if url redirected
                && info.totalSize() == m_totalSize) { //check if remote host file changed
            task = info;
            break;
        }
    }
    PeerInfoList infoList;
    if (task.isEmpty()) { //download new file
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
            PeerInfo info;
            info.setStartIndex(start);
            info.setEndIndex(end-1);
            info.setCompletedCount(0);
            info.setFilePath(m_dlRequest.filePath());
            infoList.append (info);
        }
        task.setDownloadUrl(m_dlRequest.downloadUrl().toString());
        task.setFilePath(m_dlRequest.filePath());
        task.setPeerList(infoList);
        task.setReadySize(0);
        task.setRequestUrl(m_dlRequest.requestUrl().toString());
        task.setTotalSize(m_totalSize);
        task.setUid(m_uid);
        m_transDB->appendTaskInfo(task);
        m_transDB->flush();
    } else { // continue to download
        infoList = task.peerList();
        m_dlRequest.setPreferThreadCount(infoList.size());
    }

    int index = 0;
    foreach (PeerInfo info, infoList) {
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
        DLTaskPeer *peer = new DLTaskPeer(index++, info, tmp, 0);
        peer->moveToThread (m_workerThread);
        m_peerList.append (peer);
    }
    if (!m_workerThread->isRunning())
        m_workerThread->start();
}

void DLTask::calculateUID()
{
    QString str = m_dlRequest.requestUrl().toString() + m_dlRequest.filePath() + m_dlRequest.downloadUrl().toString();
    m_uid = QString(QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
}





























} //YADownloader
