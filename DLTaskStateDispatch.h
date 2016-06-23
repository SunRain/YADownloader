#ifndef DLTASKSTATEDISPATCH_H
#define DLTASKSTATEDISPATCH_H

#include <QObject>
#include <QMutex>
#include <qcoreevent.h>

namespace YADownloader {

const static int DLTASK_EVENT_INNER = QEvent::Type(QEvent::User);
const static int DLTASK_EVENT_FILE_SIZE = QEvent::Type(QEvent::User+1);

class FileSizeEvent : public QEvent
{
public:
    explicit FileSizeEvent(qint64 fileSize = -1);
    virtual ~FileSizeEvent();
    qint64 fileSize() const;
private:
    qint64 m_fileSize;
};

class DLTaskStateDispatch : public QObject
{
    Q_OBJECT
public:
    explicit DLTaskStateDispatch(QObject *parent = 0);
    virtual ~DLTaskStateDispatch();

    void dispatchFileSize(qint64 fileSize = -1);
private:
    QMutex m_locker;
};

} //YADownloader
#endif // DLTASKSTATEDISPATCH_H
