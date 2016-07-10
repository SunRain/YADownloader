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

    ///
    /// \brief request
    /// \return DLRequest of current task, the DLRequest maybe modified by this task;
    ///
    DLRequest request() const;

    ///
    /// \brief taskInfo
    /// \return currrent DLTaskInfo
    /// Current DLTaskInfo may changed in DLTask life-cycle
    DLTaskInfo taskInfo() const;

    ///
    /// \brief
    /// \return
    ///
    bool overwriteExistFile() const;

    ///
    /// \brief setOverwriteExistFile
    /// \param overwrite
    /// If set to True, if target file exist, file will be overwirted,
    /// Otherwise, will add suffix in targe file
    ///
    void setOverwriteExistFile(bool overwrite);

    ///
    /// \brief bytesReceived
    /// \return bytes of downloaded data since start
    ///
    qint64 bytesReceived() const;

    ///
    /// \brief bytesDownloaded
    /// \return bytes of ALL downloaded data
    ///
    qint64 bytesDownloaded() const;

    ///
    /// \brief bytesFileSize
    /// \return bytes of file
    ///
    qint64 bytesFileSize() const;

    ///
    /// \brief bytesStartOffest
    /// \return bytes of allready downloaded file size when start or continue download
    ///
    qint64 bytesStartOffest() const;

    ///
    /// \brief uuid unique ID of task
    /// UUID is calculated in DLTask constructe function and never changed in class life-cycle
    /// \return
    ///
    QString uuid() const;

    TaskStatus status() const;

    // QObject interface
protected:
    bool event(QEvent *event);

protected:
    DLTask(DLTransmissionDatabase *db, QObject *parent = 0);
    DLTask(DLTransmissionDatabase *db, const DLRequest &request, QObject *parent = 0);
    void setRequest(const DLRequest &request);

signals:
    void initFileSize(qint64 fileSize);
    void downloadProgress(const QString &uuid, qint64 bytesReceived, qint64 bytesDownloaded, qint64 bytesFileSize);
    void statusChanged(const QString &uuid, TaskStatus status);
    void taskInfoChanged(const QString &uuid, const DLTaskInfo &info);

public slots:
    void start();
    void abort();
    void suspend();
    void resume();

private:
//    QString calculateUID() const;
    void initTaskInfo();
    void download();
    void saveInfo();
//    inline void emitStatus() {
//        emit statusChanged(m_DLStatus);
//    }
    bool peerCompleted(const DLTaskPeerInfo &info);
    bool allPeerCompleted();
    void managerFinishedFile();
    QString adjustSaveName(const DLRequest &req);

private:
    QNetworkAccessManager *m_networkMgr;
    QNetworkReply *m_reply;
    QNetworkReply *m_headReply;
//    QThread *m_workerThread;
    DLTaskHeaderReader *m_headerReader;
    DLTaskStateDispatch *m_dispatch;
    DLTransmissionDatabase *m_transDB;

    QList<DLTaskPeer *> m_peerList;

    DLRequest m_dlRequest;
    TaskStatus m_DLStatus;
    DLTaskInfo m_dlTaskInfo;

    QHash<QString, quint64> m_dlCompletedCountHash;//completedCount
    QString m_uuid;
    QMutex m_peerLocker;

    bool m_overwriteExistFile;
    int m_initHeaderCounts;
//    int m_peerCount;            //分解的数据块量
//    int m_peerSize;             //每个数据块大小
    qint64 m_bytesFileSize;        //文件总大小
    qint64 m_bytesDownloaded;   //文件已经下载的大小
//    qint64 m_dlBytesWhenStarted; //had beend downloadeded size when started
    qint64 m_bytesReceived; //downloaded size since start
    qint64 m_bytesStartFileOffest; //downloadeded file size when start
};



} //YADownloader
#endif // DLTASK_H
