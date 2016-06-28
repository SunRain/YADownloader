#ifndef DLTASKSTATEDISPATCH_H
#define DLTASKSTATEDISPATCH_H

#include <QObject>
#include <QMutex>
#include <qcoreevent.h>

namespace YADownloader {

const static int DLTASK_EVENT_INNER = QEvent::Type(QEvent::User);
const static int DLTASK_EVENT_FILE_SIZE = QEvent::Type(QEvent::User+1);
const static int DLTASK_EVENT_DL_PROGRESS = QEvent::Type(QEvent::User+2);
const static int DLTASK_EVENT_DL_STATUS = QEvent::Type(QEvent::User+3);

class FileSizeEvent : public QEvent
{
public:
    explicit FileSizeEvent(qint64 fileSize = -1);
    virtual ~FileSizeEvent() {}
    qint64 fileSize() const;
private:
    qint64 m_fileSize;
};

class DLProgressEvent : public QEvent
{
public:
    explicit DLProgressEvent(const QString &hash, qint64 bytesReceived, qint64 downloadedCnt ,qint64 bytesTotal);
    virtual ~DLProgressEvent() {}
    QString hash() const;
    qint64 bytesReceived() const;
    qint64 bytesTotal() const;
    qint64 downloadedCount() const;
private:
    QString m_hash;
    qint64 m_br;
    qint64 m_bt;
    qint64 m_dlCnt;
};

class DLStatusEvent : public QEvent
{
public:
    enum DLStatus {
        DL_PROGRESSING = 0X0,
        DL_START,
        DL_STOP,
        DL_FINISH,
        DL_FAILURE
    };
    explicit DLStatusEvent(const QString &hash, DLStatus status, bool isTaskPeerEvent);
    virtual ~DLStatusEvent() {}
    bool isTaskPeeEvent() const;
    QString hash() const;
    DLStatus status() const;
private:
    bool m_isTaskPeer;
    QString m_hash;
    DLStatus m_status;
};

class DLTaskStateDispatch : public QObject
{
    Q_OBJECT
public:
    explicit DLTaskStateDispatch(QObject *parent = 0);
    virtual ~DLTaskStateDispatch();

    void dispatchFileSize(qint64 fileSize = -1);

    ///
    /// \brief dispatchDownloadProgress
    /// \param hash Hash for taskpeer
    /// \param bytesReceived Received bytes sine download started
    /// \param downloadedCnt ALL Downloaded bytes for a taskpeer
    /// \param bytesTotal Total bytes for a taskpeer
    ///
    void dispatchDownloadProgress(const QString &hash, qint64 bytesReceived, qint64 downloadedCnt, qint64 bytesTotal);

    void dispatchDownloadStatus(const QString &hash, DLStatusEvent::DLStatus status, bool isTaskPeerEvent);
private:
    QMutex m_locker;
};

} //YADownloader
#endif // DLTASKSTATEDISPATCH_H
