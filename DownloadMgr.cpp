#include "DownloadMgr.h"

#include <QDebug>

#include "SignalCenter.h"
#include "TaskObject.h"

namespace YADownloader {

DownloadMgr::DownloadMgr(QObject *parent)
    : QObject(parent)
{

}

DownloadMgr::~DownloadMgr()
{

}

void DownloadMgr::NewDownloadTask(TaskObject *object)
{
    if (!object) {
        qWarning()<<Q_FUNC_INFO<<"Opps, empty task object was addedd !!";
        return;
    }

//    SignalCenter::instance ()->TaskObjectAdded (object);
}

} //YADownloader
