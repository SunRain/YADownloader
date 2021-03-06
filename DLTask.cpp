#include "DLTask.h"

#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QEventLoopLocker>
#include <QTimer>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QCoreApplication>

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

const static QString PEER_TAG = ".peer";

DLTask::DLTask(DLTransmissionDatabase *db, QObject *parent)
    : DLTask(db, DLRequest(), parent)
{
}

DLTask::DLTask(DLTransmissionDatabase *db, const DLRequest &request, QObject *parent)
    : QObject(parent)
    , m_networkMgr(new QNetworkAccessManager(this))
    , m_reply(nullptr)
//    , m_workerThread(new QThread(this))
    , m_headerReader(nullptr)
    , m_dispatch(new DLTaskStateDispatch(this))
    , m_transDB(db)
    , m_dlRequest(request)
    , m_DLStatus(DL_STOP)
    , m_overwriteExistFile(false)
    , m_constructFromResumable(false)
    , m_initHeaderCounts(3)
    , m_bytesFileSize(-1)
    , m_bytesDownloaded(0)
    , m_bytesReceived(0)
    , m_bytesStartFileOffest(0)
{
    QString value = QUuid::createUuid().toString() + m_dlRequest.requestUrl();
    m_uuid = QString(QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Md5).toHex());

//    connect (m_workerThread, &QThread::finished, [&]() {
//        qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<<<<<<  m_workerThread finished";
//        if(m_DLStatus != DL_STOP || m_DLStatus != DL_FINISH) {
//            abort();
//        }
    //    });

}

DLTask::DLTask(DLTransmissionDatabase *db, const DLTaskInfo &info,
               const QHash<QByteArray, QByteArray> &rawHeaders,
               QObject *parent)
    : DLTask(db, DLRequest(), parent)

{
    m_uuid = info.identifier();
    m_dlTaskInfo = info;

    QFileInfo fi(info.filePath());
    m_dlRequest.setDownloadUrl(info.downloadUrl());
    m_dlRequest.setPreferThreadCount(info.peerList().size());
    m_dlRequest.setRequestUrl(info.requestUrl());
    m_dlRequest.setSaveName(fi.fileName());
    m_dlRequest.setSavePath(fi.absolutePath());
    foreach (const QByteArray &key, rawHeaders.keys()) {
        m_dlRequest.setRawHeader(key, rawHeaders.value(key));
    }
    m_constructFromResumable = true;
}

DLTask::~DLTask()
{
    qDebug()<<Q_FUNC_INFO<<"===========";

    if(m_DLStatus != DL_STOP || m_DLStatus != DL_FINISH) {
        abort();
    }
//    if (m_workerThread->isRunning ()) {
//        m_workerThread->quit ();
//    }

//    while (!m_workerThread->isFinished()) {
//        /// do nothing
////        qApp->processEvents();
//        m_workerThread->yieldCurrentThread();
//        QThread::yieldCurrentThread();
//    }

//    m_workerThread->deleteLater ();
//    m_workerThread = nullptr;

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
//    if (m_dispatch)
//        m_dispatch->deleteLater();
//    m_dispatch = nullptr;
}

void DLTask::setRequest(const DLRequest &request) {
    m_dlRequest = request;
}

DLRequest DLTask::request() const {
    return m_dlRequest;
}

DLTaskInfo DLTask::taskInfo() const
{
    return m_dlTaskInfo;
}

bool DLTask::overwriteExistFile() const
{
    return m_overwriteExistFile;
}

void DLTask::setOverwriteExistFile(bool overwrite)
{
    m_overwriteExistFile = overwrite;
}

qint64 DLTask::bytesReceived() const
{
    return m_bytesReceived;
}

qint64 DLTask::bytesDownloaded() const
{
    return m_bytesDownloaded;
}

qint64 DLTask::bytesFileSize() const
{
    return m_bytesFileSize;
}

qint64 DLTask::bytesStartOffest() const
{
    return m_bytesStartFileOffest;
}

QString DLTask::uuid() const
{
    return m_uuid;
}

bool DLTask::event(QEvent *event)
{
    if (event->type() == DLTASK_EVENT_FILE_SIZE) {
        qDebug()<<Q_FUNC_INFO<<"========== FileSizeEvent";
        FileSizeEvent *e = (FileSizeEvent*)event;
        m_bytesFileSize = e->fileSize();
        qDebug()<<Q_FUNC_INFO<<" file size "<<m_bytesFileSize;
        emit initFileSize(m_bytesFileSize);
        if (m_DLStatus == DL_START) {
//            if (!m_workerThread->isRunning ())
//                m_workerThread->start ();
            m_peerLocker.lock();
            if (m_peerList.isEmpty ()) {
                initTaskInfo();
                download();
//                emitStatus();
//                m_dispatch->dispatchDownloadStatus(m_uuid, DLStatusEvent::DLStatus::DL_START, false);
            }
            m_peerLocker.unlock();
        }
        return true;
    }
    if (event->type() == DLTASK_EVENT_DL_PROGRESS) {
        DLProgressEvent *e = (DLProgressEvent*)event;
        QString hash = e->hash();
        qint64 dlcnt = e->downloadedCount();
        m_bytesReceived += e->bytesReceived();
        m_dlCompletedCountHash.insert(hash, dlcnt);
        quint64 d =0;
        foreach (quint64 i, m_dlCompletedCountHash.values()) {
            d += i;
        }
        m_bytesDownloaded = d;//m_dlBytesFileOffest + d;
//        qDebug()<<Q_FUNC_INFO<<" hash "<<hash<<" dlcnt "<<dlcnt<<" total "<<m_totalSize
//               <<" bytesReceived "<<e->bytesReceived();

//        qDebug()<<Q_FUNC_INFO<<">>> "<<m_dlCompletedHash;

        m_dlTaskInfo.setReadySize(m_bytesDownloaded);
//        qDebug()<<Q_FUNC_INFO<<"downloadProgress downloadedSize "<<m_bytesDownloaded
//               <<" totalSize "<<m_bytesFileSize
//               <<" percent "<<(float)m_bytesDownloaded/(float)m_bytesFileSize;
        ///FIXME dispatchDLTaskInfo in download progress may use lots of resources
//        m_dispatch->dispatchDLTaskInfo(m_uuid, m_dlTaskInfo);
        emit downloadProgress(m_uuid, m_bytesReceived, m_bytesDownloaded, m_bytesFileSize);
        return true;
    }
    if (event->type() == DLTASK_EVENT_DL_STATUS) {
        DLStatusEvent *e = (DLStatusEvent*)event;
        QString hash = e->hash();
        DLStatusEvent::DLStatus status = e->status();
        qDebug()<<Q_FUNC_INFO<<">>>>>>>>>>>>> DLTASK_EVENT_DL_STATUS for hash "<<hash
               <<"  is peere event "<<e->isTaskPeerEvent();
        if (e->isTaskPeerEvent()) {
            if (status == DLStatusEvent::DLStatus::DL_FINISH) {
                if (allPeerCompleted()) {
                    managerFinishedFile();
//                    m_dispatch->dispatchDownloadStatus(m_uuid, DLStatusEvent::DLStatus::DL_FINISH, false);
//                    m_DLStatus = DL_FINISH;
//                    m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_STOP);
                    m_peerLocker.lock();
                    if (!m_peerList.isEmpty()) {
                        foreach (DLTaskPeer *p, m_peerList) {
                            while (!p->isFinished()) {
                                qApp->processEvents();
                            }
                        }
                        qDeleteAll(m_peerList);
                        m_peerList.clear();
                    }
                    m_peerLocker.unlock();
//                    emit statusChanged(m_uuid, m_DLStatus);
                }
            }
        } else {
            if (status == DLStatusEvent::DLStatus::DL_FAILURE) {
                m_DLStatus = DL_FAILURE;
                m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_STOP);
            } else if (status == DLStatusEvent::DLStatus::DL_FINISH) {
                m_DLStatus = DL_FINISH;
                m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_STOP);
                m_peerLocker.lock();
                if (!m_peerList.isEmpty()) {
                    foreach (DLTaskPeer *p, m_peerList) {
                        while (!p->isFinished()) {
                            qApp->processEvents();
                        }
                    }
                    qDeleteAll(m_peerList);
                    m_peerList.clear();
                }
                m_peerLocker.unlock();
            } else if (status == DLStatusEvent::DLStatus::DL_PROGRESSING) {
                //TODO dummy
            } else if (status == DLStatusEvent::DLStatus::DL_START) {
                m_DLStatus = DL_START;
                m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_RUNNING);
            } else if (status == DLStatusEvent::DLStatus::DL_STOP) {
                m_DLStatus = DL_STOP;
                m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_STOP);
                m_peerLocker.lock();
                if (!m_peerList.isEmpty()) {
                    foreach (DLTaskPeer *p, m_peerList) {
                        while (!p->isFinished()) {
                            qApp->processEvents();
                        }
                    }
                    qDeleteAll(m_peerList);
                    m_peerList.clear();
                }
                m_peerLocker.unlock();
            }
            emit statusChanged(m_uuid, m_DLStatus);
        }

        return true;
    }
    if (event->type() == DLTASK_EVENT_DL_TASKINFO) {
        DLTaskInfoEvent *e = (DLTaskInfoEvent *)event;
        QString hash = e->hash();
        DLTaskInfo m_dlTaskInfo = e->taskInfo();
        if (hash == m_uuid) {
            emit taskInfoChanged(hash, m_dlTaskInfo);
        }
        return true;
    }
    return QObject::event(event);
}

DLTask::TaskStatus DLTask::status() const
{
    return m_DLStatus;
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
    qDebug()<<Q_FUNC_INFO<<"===========";
    m_DLStatus = DL_STOP;
    m_peerLocker.lock();
    if (!m_peerList.isEmpty()) {
        foreach (DLTaskPeer *p, m_peerList) {
            p->abort();
            while (!p->isFinished()) {
                qApp->processEvents();
            }
        }
        m_peerLocker.unlock();
        m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_STOP);
        saveInfo();
    }
    m_peerLocker.unlock();
    m_dispatch->dispatchDownloadStatus(m_uuid, DLStatusEvent::DLStatus::DL_STOP, false);
}

void DLTask::suspend()
{
    abort();
}

void DLTask::resume()
{
    if (!m_peerList.isEmpty()) {
        qWarning()<<Q_FUNC_INFO<<"Can't resume as download peer still exists!!";
        return;
    }
    m_DLStatus = DL_START;
    if (m_bytesFileSize <= 0) {
        start();
    } else {
        initTaskInfo();
        download();
    }
}

void DLTask::download()
{
//    if (m_workerThread->isRunning())
//        m_workerThread->quit();
//    m_workerThread->wait();

    DLTaskPeerInfoList peerInfoList = m_dlTaskInfo.peerList();
    foreach (DLTaskPeerInfo info, peerInfoList) {
//        m_downloadedSize += info.dlCompleted();
        m_bytesStartFileOffest += info.dlCompleted();


        /*
         * NOTE we still start a new request as we need to save the request infomation into local storage
         */
//        if (peerCompleted(info))
//            continue;
        QNetworkRequest req(m_dlRequest.downloadUrl ());
        if (!m_dlRequest.rawHeaders ().isEmpty ()) {
            foreach (QByteArray key, m_dlRequest.rawHeaders ().keys ()) {
//                QString value = m_dlRequest.rawHeaders().value(key, QByteArray());
//                qDebug()<<Q_FUNC_INFO<<QString("insert header [%1] = [%2]").arg(QString(key)).arg(value);
                req.setRawHeader(key, m_dlRequest.rawHeaders().value(key, QByteArray()));
            }
        }
        /*
         * NOTE if Range is not supported by remote server or we can't get remote file size,
         * we DISABLE resume downloading
         * Need forther codeing
         */
        if (m_bytesFileSize > 0) {
            QString range = QString("bytes=%1-%2").arg(info.rangeStart()).arg(info.endIndex());
            req.setRawHeader ("Range", range.toUtf8 ());
        }
        req.setRawHeader ("Connection", "keep-alive");

        QNetworkReply *reply = m_networkMgr->get(req);
        if (!reply) {
            qCritical()<<Q_FUNC_INFO<<"No QNetworkReply， download cant start";
            return;
        }
        DLTaskPeer *peer = new DLTaskPeer(m_dispatch, info, reply, 0);
//        peer->moveToThread (m_workerThread);
        m_dlCompletedCountHash.insert(peer->hash(), info.dlCompleted());
        m_peerList.append(peer);
    }
//    if (!m_workerThread->isRunning())
//        m_workerThread->start();
    m_dlTaskInfo.setStatus(DLTaskInfo::STATUS_RUNNING);
    m_transDB->appendTaskInfo(m_dlTaskInfo);
    m_transDB->flush();
    m_dispatch->dispatchDownloadStatus(m_uuid, DLStatusEvent::DLStatus::DL_START, false);
    m_dispatch->dispatchDLTaskInfo(m_uuid, m_dlTaskInfo);
}

//QString DLTask::calculateUID() const
//{
//    QString str = m_dlRequest.requestUrl().toString() + m_dlRequest.filePath()/* + m_dlRequest.downloadUrl().toString()*/;
//    return QString(QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
//}

void DLTask::initTaskInfo()
{
    if (m_bytesFileSize <= 0) { //We can't get downloaded file size, so we should use 1 thread to download
        m_dlRequest.setPreferThreadCount (1);
    }
    /// FIX
    /// As we remove finished task in database now, we need to check if target file is existing
    m_dlRequest.setSaveName(adjustSaveName(m_dlRequest));

    /// FIX
    /// After adjusted save-name, check if resumable task available in database
    m_dlTaskInfo.clear();
    foreach (DLTaskInfo info, m_transDB->list()) {
        if (QUrl(info.requestUrl()) == m_dlRequest.requestUrl()
//                && info.downloadUrl() == m_dlRequest.downloadUrl() //check if url redirected
                && info.filePath() == m_dlRequest.filePath()
                && info.totalSize() == m_bytesFileSize) { //check if remote host file changed
            m_dlTaskInfo = info;
            break;
        }
    }

    /* An exist dltask in database, but we start a new dltask
     * Thus we need to adjust file save name and clear m_dlTaskInfo
     */
    if (!m_constructFromResumable && !m_dlTaskInfo.isEmpty()) {
        m_dlRequest.setSaveName(constructFileNameSuffix(m_dlTaskInfo.filePath()));
        m_dlTaskInfo.clear();
    }

    /// If we find task info in database
    if (!m_dlTaskInfo.isEmpty()) {

        /// Exist download finished
        if (QFile::exists(m_dlTaskInfo.filePath())) {
            m_dlTaskInfo.clear();
            QFile file(m_dlRequest.filePath());
            if (overwriteExistFile()) {
                if (!file.remove()) {
                    qWarning()<<Q_FUNC_INFO<<"Try to remove exist file error, "<<file.errorString();
                }
            } else {
                m_dlRequest.setSaveName(adjustSaveName(m_dlRequest));

//                qDebug()<<Q_FUNC_INFO<<"............. new request "<<m_dlRequest;

                /// We lookup database to find if there's exist task again
                foreach (DLTaskInfo info, m_transDB->list()) {
                    if (QUrl(info.requestUrl()) == m_dlRequest.requestUrl()
                            && info.filePath() == m_dlRequest.filePath()
                            && info.totalSize() == m_bytesFileSize) {
                        m_dlTaskInfo = info;
//                        qDebug()<<Q_FUNC_INFO<<"++++++++++++= find exit task "<<m_dlTaskInfo;
                        break;
                    }
                }
                /// We find an exist task in database,
                /// Then we need to check if traget temp file was removed
                /// IF target tmp file was removed, we need to reset its peer info to start a new dl
                if (!m_dlTaskInfo.isEmpty() && !QFile::exists(m_dlTaskInfo.filePath()+PEER_TAG)) {
//                    qDebug()<<Q_FUNC_INFO<<"Reset peer for "<<m_dlTaskInfo;
                    DLTaskPeerInfoList list;
                    foreach (DLTaskPeerInfo i, m_dlTaskInfo.peerList()) {
                        i.setCompletedCount(0);
                        list.append(i);
                    }
                    m_dlTaskInfo.setPeerList(list);
                }
            }
        } else { // Exist download not finished
            /* Download not finished, but tmp file was removed
             * We need to reset its peer info to start a new dl
             */
            if (!QFile::exists(m_dlTaskInfo.filePath()+PEER_TAG)) {
                DLTaskPeerInfoList list;
                foreach (DLTaskPeerInfo i, m_dlTaskInfo.peerList()) {
                    i.setCompletedCount(0);
                    list.append(i);
                }
                m_dlTaskInfo.setPeerList(list);
            }
        }
    }
    if (m_dlTaskInfo.isEmpty()) { //download new file

        /// FIX
        /// As we remove finished task in database now, we need to check if target file is existing
        m_dlRequest.setSaveName(adjustSaveName(m_dlRequest));

        int threadCount = m_dlRequest.preferThreadCount ();
        quint64 step = m_bytesFileSize/threadCount;
        quint64 start = -step;
        quint64 end = 0;
        DLTaskPeerInfoList peerInfoList;
        for (int index=1; index <= threadCount; ++index) {
            end += step;
            start += step;
            if (index == threadCount) {
                end = m_bytesFileSize;
            }
            DLTaskPeerInfo pInfo;
            pInfo.setStartIndex(start);
            pInfo.setEndIndex(end-1);
            pInfo.setCompletedCount(0);
            pInfo.setFilePath(m_dlRequest.filePath() + PEER_TAG);
            peerInfoList.append(pInfo);
        }
        m_dlTaskInfo.setDownloadUrl(m_dlRequest.downloadUrl());
        m_dlTaskInfo.setFilePath(m_dlRequest.filePath());
        m_dlTaskInfo.setPeerList(peerInfoList);
        m_dlTaskInfo.setReadySize(0);
        m_dlTaskInfo.setRequestUrl(m_dlRequest.requestUrl());
        m_dlTaskInfo.setTotalSize(m_bytesFileSize);
    }
    if (!m_dlTaskInfo.isEmpty()) {
        m_dlTaskInfo.setStatus(DLTaskInfo::STATUS_STOP);
        m_dlTaskInfo.setIdentifier(m_uuid);
        m_transDB->appendTaskInfo(m_dlTaskInfo);
        m_transDB->flush();
    }

    m_bytesDownloaded = m_dlTaskInfo.readySize();
    m_bytesReceived = 0;
    m_bytesStartFileOffest = 0;

    m_dispatch->dispatchDLTaskInfo(m_uuid, m_dlTaskInfo);
}

void DLTask::saveInfo()
{
    DLTaskPeerInfoList list;
    foreach (DLTaskPeer *peer, m_peerList) {
        qint64 done = m_dlCompletedCountHash.value(peer->hash());
        DLTaskPeerInfo info = peer->info();
        info.setCompletedCount(done);
        list.append(info);
    }
    m_dlTaskInfo.setPeerList(list);
    m_dlTaskInfo.setReadySize(m_bytesDownloaded);
    m_transDB->appendTaskInfo(m_dlTaskInfo);
    m_transDB->flush();
    m_dispatch->dispatchDLTaskInfo(m_uuid, m_dlTaskInfo);
}

bool DLTask::peerCompleted(const DLTaskPeerInfo &info)
{
    if (info.dlCompleted() <= 0)
        return false;
    if (info.dlCompleted() == (info.endIndex()-info.startIndex()+1))
        return true;
    return false;
}

bool DLTask::allPeerCompleted()
{
    if (m_bytesFileSize > 0) {
        quint64 total = 0;
        foreach (quint64 i, m_dlCompletedCountHash.values()) {
            total += i;
        }
        if (total == m_bytesFileSize)
            return true;
        else
            return false;
    }
    bool ret = true;
    foreach (DLTaskPeer *p, m_peerList) {
        DLTaskPeerInfo info = p->info();
        qint64 completed = m_dlCompletedCountHash.value(p->hash());
        qint64 total = info.endIndex() - info.startIndex() +1;
        if (completed != total && total > 0) {
            ret = false;
            break;
        }
    }
    return ret;
}

void DLTask::managerFinishedFile()
{
    qDebug()<<Q_FUNC_INFO<<">>>>>>>>>>>>>>>>>>>>>>>>> dl finish";

    bool error = false;
    foreach (DLTaskPeer *p, m_peerList) {
        if (!p->isFinished()) {
            error = true;
            break;
        }
    }
    if (error) {
        qCritical()<<Q_FUNC_INFO<<"Manager finish error as some download peer not finish!!!";
//        m_DLStatus = DL_STOP;
//        m_dlTaskInfo.setStatus(DLTaskInfo::STATUS_STOP);
//        m_dispatch->dispatchDownloadStatus(m_uuid, DLStatusEvent::DLStatus::DL_STOP, false);
//        m_dispatch->dispatchDLTaskInfo(m_uuid, m_dlTaskInfo);
//        m_dlTaskInfo.setStatus(DLTaskInfo::TaskStatus::STATUS_STOP);
//        saveInfo();
        abort();
        return;
    }
//    qDeleteAll(m_peerList);
//    m_peerList.clear();

//    if (m_workerThread->isRunning()) {
//        m_workerThread->quit();
////        m_workerThread->wait();
//    }
    m_dlCompletedCountHash.clear();

    m_DLStatus = DL_FINISH;
    m_dlTaskInfo.setStatus(DLTaskInfo::STATUS_STOP);
    m_dispatch->dispatchDownloadStatus(m_uuid, DLStatusEvent::DLStatus::DL_FINISH, false);
    m_dispatch->dispatchDLTaskInfo(m_uuid, m_dlTaskInfo);
    m_transDB->removeTaskInfo(m_dlTaskInfo);
    m_transDB->flush();

    QString fname = m_dlRequest.filePath();
    if (!QFile::exists(fname + PEER_TAG) && !QFile::exists(fname)) {
        qWarning()<<Q_FUNC_INFO<<QString("Download finished, but we can't find tmp file [%1] now")
                    .arg(fname + PEER_TAG);
        return;
    }
    if (QFile::exists(fname))
        return;

    QFile f(fname + PEER_TAG);
    if (f.size() != m_bytesFileSize && m_bytesFileSize > 0) {
        qWarning()<<Q_FUNC_INFO<<QString("Download finished, but downloaded file size [%1], not equal as prefer size [%2]")
                    .arg(f.size()).arg(m_bytesDownloaded);
        return;
    }

    if (!f.rename(fname))
        qWarning()<<Q_FUNC_INFO<<"Rename download file error "<<f.errorString();
}

QString DLTask::adjustSaveName(const DLRequest &req)
{
    QString file = req.filePath();
    if (!QFile::exists(file))
        return req.saveName();

    QFileInfo info(file);
    QString dir = info.absolutePath();
    QString name = info.baseName();
    QString suffix = info.completeSuffix();
    QString nName = req.filePath();
    int tail = 0;
    /// loop to find new suffix for file
    ///
    /// We check real target file name (means name without PEER_TAG),
    /// this means, if there's a running && not finished (but current stopped) download task,
    /// current new request will point to this task
    while(true) {
        if (!QFile::exists(nName))
            break;
        nName = QString("%1/%2-%3.%4").arg(dir).arg(name).arg(tail).arg(suffix);
        tail++;
    }
    QFileInfo fi(nName);

    qDebug()<<Q_FUNC_INFO<<" --------- "<<fi.fileName();
    return fi.fileName();
}

QString DLTask::constructFileNameSuffix(const QString &filePath)
{
    if (filePath.isEmpty()) {
        qCritical()<<Q_FUNC_INFO<<"Basename is empty!!!!";
        return QString();
    }
    QFileInfo info(filePath);
    QString dir = info.absolutePath();
    QString name = info.baseName();
    QString suffix = info.completeSuffix();
    QString nName;
    int tail = 0;
    while (true) {
        nName = QString("%1/%2-%3.%4").arg(dir).arg(name).arg(tail).arg(suffix);
        if (!QFile::exists(nName))
            break;
        tail++;
    }
    QFileInfo fi(nName);

    qDebug()<<Q_FUNC_INFO<<" --------- "<<fi.fileName();
    return fi.fileName();
}





























} //YADownloader
