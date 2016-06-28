#ifndef DLTASK_H
#define DLTASK_H

#include <QObject>
#include <QList>
#include "yadownloader_global.h"
#include "DLRequest.h"
#include "DLTaskInfo.h"

class QNetworkAccessManager;
class QNetworkReply;
class QThread;
namespace YADownloader {

class DLTaskPeer;
class DLTaskHeaderReader;
class DLTaskStateDispatch;
class DLTransmissionDatabase;
class YADOWNLOADERSHARED_EXPORT DLTask : public QObject
{
    Q_OBJECT

public:
    enum TaskStatus {
        DL_START = 0x0,
        DL_STOP,
        DL_FINISH,
        DL_FAILURE
    };

    virtual ~DLTask();

    DLRequest request() const;

    ///
    /// \brief uid unique ID of task
    /// \return
    ///
    QString uid() const;

    // QObject interface
public:
    bool event(QEvent *event);

    TaskStatus status() const;

public slots:
    void start();
    void abort();
    void suspend();
    void resume();

protected:
    DLTask(DLTransmissionDatabase *db, QObject *parent = 0);
    DLTask(DLTransmissionDatabase *db, const DLRequest &request, QObject *parent = 0);
    void setRequest(const DLRequest &request);

signals:
    void initFileSize(qint64 fileSize);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void statusChanged(TaskStatus status);

private:
    QString calculateUID() const;
    void initTaskInfo();
    void download();
    void saveInfo();
    inline void emitStatus() {
        emit statusChanged(m_DLStatus);
    }

private:
    QNetworkAccessManager *m_networkMgr;
    QNetworkReply *m_reply;
    QNetworkReply *m_headReply;
    QThread *m_workerThread;
    DLTaskHeaderReader *m_headerReader;
    DLTaskStateDispatch *m_dispatch;
    DLTransmissionDatabase *m_transDB;

    QList<DLTaskPeer *> m_peerList;

    DLRequest m_dlRequest;
    TaskStatus m_DLStatus;
    DLTaskInfo m_dlTaskInfo;

    QHash<QString, quint64> m_dlCompletedHash;//completedCount
//    QString m_uid;

    int m_initHeaderCounts;
//    int m_peerCount;            //分解的数据块量
//    int m_peerSize;             //每个数据块大小
    qint64 m_totalSize;        //文件总大小
    qint64 m_downloadedSize;   //文件已经下载的大小
//    qint64 m_dlBytesWhenStarted; //had beend downloadeded size when started
    qint64 m_dlBytesReceived; //downloaded size since start
    qint64 m_dlBytesFileOffest; //downloadeded file size when start
};



} //YADownloader
#endif // DLTASK_H
