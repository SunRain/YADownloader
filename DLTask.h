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
class YADOWNLOADERSHARED_EXPORT DLTask : public QObject
{
    Q_OBJECT
public:
    explicit DLTask(QObject *parent = 0);
    explicit DLTask(const DLRequest &request, QObject *parent = 0);
    virtual ~DLTask();

    void setRequest(const DLRequest &request);
    const DLRequest &request() const;

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

private:
    void initFileSize();
    void initPeers();

private:
    QNetworkAccessManager *m_networkMgr;
    QNetworkReply *m_reply;
    QNetworkReply *m_headReply;
    QThread *m_workerThread;

    QList<DLTaskPeer *> m_peerList;

    DLRequest m_dlRequest;
    TaskStatus m_DLStatus;

    int m_initHeaderCounts;
    bool m_isInitiated;

    int m_peerCount;            //分解的数据块量
    int m_peerSize;             //每个数据块大小
    qint64 m_totalSize;        //文件总大小
    quint64 m_downloadedSize;   //文件已经下载的大小

//    PeerList m_peerList;
};

} //YADownloader
#endif // DLTASK_H
