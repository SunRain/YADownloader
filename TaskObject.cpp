#include "TaskObject.h"

#include <QUrl>

namespace YADownloader {

TaskObject::TaskObject(QObject *parent)
    : QObject(parent)
{

}

void TaskObject::setUrl(QUrl url) {
    if (m_url == url)
        return;

    m_url = url;
    emit urlChanged(url);
}

void TaskObject::setSavePath(QString savePath)
{
    if (m_savePath == savePath)
        return;

    m_savePath = savePath;
    emit savePathChanged(savePath);
}

} //YADownloader
