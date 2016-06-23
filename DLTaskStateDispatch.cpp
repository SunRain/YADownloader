#include "DLTaskStateDispatch.h"

#include <QCoreApplication>

namespace YADownloader {

FileSizeEvent::FileSizeEvent(qint64 fileSize)
    : QEvent((QEvent::Type)DLTASK_EVENT_FILE_SIZE)
    , m_fileSize(fileSize)
{

}

FileSizeEvent::~FileSizeEvent()
{

}

qint64 FileSizeEvent::fileSize() const
{
    return m_fileSize;
}




DLTaskStateDispatch::DLTaskStateDispatch(QObject *parent)
    : QObject(parent)
    , m_locker(QMutex::Recursive)
{

}

DLTaskStateDispatch::~DLTaskStateDispatch()
{

}

void DLTaskStateDispatch::dispatchFileSize(qint64 fileSize)
{
    qApp->postEvent(parent(), new FileSizeEvent(fileSize));
}

} //YADownloader
