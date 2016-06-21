#include "DLTask.h"

#include <QDebug>
#include <QThread>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "DLRequest.h"
#include "SignalCenter.h"
#include "DLTaskPeer.h"

namespace YADownloader {

DLTask::DLTask(QObject *parent)
    : QObject(parent)
    , m_networkMgr(new QNetworkAccessManager(this))
    , m_reply(nullptr)
    , m_headReply(nullptr)
    , m_workerThread(new QThread(this))
    , m_dlRequest(DLRequest())
    , m_DLStatus(DL_STOP)
    , m_initHeaderCounts(3)
    , m_isInitiated(false)
    , m_peerCount(0)
    , m_peerSize(0)
    , m_totalSize(-1)
    , m_downloadedSize(0)
{
    connect (m_workerThread, &QThread::finished, [&](){
        qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<<<<<";
        //TODO finish DLTaskPeer
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
}

DLTask::DLTask(const DLRequest &request, QObject *parent)
    : DLTask(parent)
{
    m_dlRequest = request;
}

DLTask::~DLTask()
{
    if (m_workerThread->isRunning ()) {
        m_workerThread->quit ();
    }
    m_workerThread->wait ();
    m_workerThread->deleteLater ();
    m_workerThread = nullptr;

    if (m_headReply) {
        m_headReply->abort ();
        m_headReply->deleteLater ();
        m_headReply = nullptr;
    }
    if (m_networkMgr) {
        m_networkMgr->deleteLater ();
        m_networkMgr = nullptr;
    }

}

void DLTask::setRequest(const DLRequest &request) {
    m_dlRequest = request;
}

const DLRequest &DLTask::request() const {
    return m_dlRequest;
}

void DLTask::start()
{
    m_DLStatus = DL_START;
    if (!m_isInitiated) {
        initFileSize ();
        return;
    }
    qDebug()<<Q_FUNC_INFO<<"File size "<<m_totalSize;

    if (!m_workerThread->isRunning ())
        m_workerThread->start ();
    if (m_peerList.isEmpty ()) {
        initPeers ();
    }


}

void DLTask::initFileSize() {
    qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";

    QNetworkRequest req(m_dlRequest.downloadUrl ());
    if (!m_dlRequest.rawHeaders ().isEmpty ()) {
        foreach (QByteArray key, m_dlRequest.rawHeaders ().keys ()) {
            req.setRawHeader (key, m_dlRequest.rawHeaders ().value (key, QByteArray()));
        }
    }
    if (m_headReply) {
        m_headReply->deleteLater ();
        m_headReply = nullptr;
    }
    m_headReply = m_networkMgr->head (req);
    if (m_headReply) {
        connect (m_headReply, &QNetworkReply::finished, [&]() {
            int count = m_initHeaderCounts--;
            qDebug()<<Q_FUNC_INFO<<" try count "<<count;

            if (count == 0) {
                qDebug()<<Q_FUNC_INFO<<"Try counts out";
                m_isInitiated = true;
                m_headReply->deleteLater ();
                m_headReply = nullptr;
                if (m_DLStatus == DL_START)
                    start ();
            } else {
                QNetworkReply::NetworkError error = m_headReply->error ();
                if (error != QNetworkReply::NoError) {
                    m_headReply->deleteLater ();
                    m_headReply = nullptr;
                    qDebug()<<Q_FUNC_INFO<<"get head error ["<<m_headReply->errorString ()<<"]";
                    if (count > 0) { //re-try to get header
                        initFileSize ();
                    }
                } else {
                    int status = m_headReply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ();
                    qDebug()<<Q_FUNC_INFO<<"header statu code "<<status;
                    if (status == 302) { //redirect
                        qDebug()<<Q_FUNC_INFO<<" try  redirect";
                        ++m_initHeaderCounts; //redirect, we should add 1 to m_initHeaderCounts
                        QUrl url = m_headReply->header (QNetworkRequest::LocationHeader).toUrl ();
                        if (url.isValid ()) {
                            qDebug()<<Q_FUNC_INFO<<"redirect url: "<<url;
                            m_dlRequest.setDownloadUrl (url);
                        }
                        m_headReply->deleteLater ();
                        m_headReply = nullptr;
                        initFileSize ();
                    } else {
                        m_totalSize = m_headReply->header (QNetworkRequest::ContentLengthHeader).toLongLong ();
                        m_headReply->deleteLater ();
                        m_headReply = nullptr;
                        qDebug()<<Q_FUNC_INFO<<" m_totalSize "<<m_totalSize;
                        m_isInitiated = true;
                        if (m_DLStatus == DL_START)
                            start ();
                    }
                }
            }
        });
    }
}

void DLTask::initPeers()
{
    if (m_totalSize <= 0) { //We can't get downloaded file size, so we should use 1 thread to download
        m_dlRequest.setPreferThreadCount (1);
    }

    //TODO for continue download from tmp files

    //download new file
    QList<PeerInfo> infoList;
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
        info.startIndex = start; //dl start point
        info.endIndex = end-1; //dl end point
        info.completedCount = 0; //finished
        info.filePath = m_dlRequest.filePath ();
        infoList.append (info);
    }

    int index = 0;
    foreach (PeerInfo info, infoList) {
        QNetworkRequest req(m_dlRequest.downloadUrl ());
        QString range = QString("bytes=%1-%2").arg (info.rangeStart ()).arg (info.endIndex);
        req.setRawHeader ("Range", range.toUtf8 ());
        req.setRawHeader ("Connection", "keep-alive");
        QNetworkReply *tmp = m_networkMgr->get (req);
        if (!tmp) {
            qCritical()<<Q_FUNC_INFO<<"No QNetworkReply， download cant start";
            return;
        }
        DLTaskPeer *peer = new DLTaskPeer(index++, info, tmp, 0);
        peer->moveToThread (m_workerThread);
        connect (m_workerThread, &QThread::finished, [&]() {
            qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<";
            foreach (DLTaskPeer *p, m_peerList) {
                p->deleteLater ();
            }
        });
        m_peerList.append (peer);
    }
}





























} //YADownloader
