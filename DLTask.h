#ifndef DLTASK_H
#define DLTASK_H

#include <QObject>
#include <QList>
#include "yadownloader_global.h"
#include "DLRequest.h"

class QNetworkAccessManager;
class QNetworkReply;
class QThread;
namespace YADownloader {

class DLTaskPeer;
class DLTaskHeaderReader;
class DLTaskStateDispatch;
class YADOWNLOADERSHARED_EXPORT DLTask : public QObject
{
    Q_OBJECT
public:

    virtual ~DLTask();

    const DLRequest &request() const;

    ///
    /// \brief uid unique ID of task
    /// \return
    ///
    QString uid() const;

    // QObject interface
public:
    bool event(QEvent *event);

public slots:
    void start();
    //    void stop();
    //    void suspend();
    //    void resume();

protected:
    enum TaskStatus {
        DL_START = 0x0,
        DL_STOP,
        DL_SUSPEND
    };
    explicit DLTask(QObject *parent = 0);
    explicit DLTask(const DLRequest &request, QObject *parent = 0);
    void setRequest(const DLRequest &request);

signals:
    void initFileSize(qint64 fileSize);

private:
//    void initFileSize();
    void initPeers();

private:
    QNetworkAccessManager *m_networkMgr;
    QNetworkReply *m_reply;
    QNetworkReply *m_headReply;
    QThread *m_workerThread;
    DLTaskHeaderReader *m_headerReader;
    DLTaskStateDispatch *m_dispatch;

    QList<DLTaskPeer *> m_peerList;

    DLRequest m_dlRequest;
    TaskStatus m_DLStatus;

    QString m_uid;

    int m_initHeaderCounts;
    int m_peerCount;            //分解的数据块量
    int m_peerSize;             //每个数据块大小
    qint64 m_totalSize;        //文件总大小
    quint64 m_downloadedSize;   //文件已经下载的大小
};



} //YADownloader
#endif // DLTASK_H
