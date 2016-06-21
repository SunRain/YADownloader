#include "DownloadMgr.h"

#include <QDebug>

#include "DLTaskImpl.h"

namespace YADownloader {

DownloadMgr::DownloadMgr(QObject *parent)
    : QObject(parent)
{

}

DownloadMgr::~DownloadMgr()
{

}

DLTask *DownloadMgr::get(const DLRequest &request)
{
    DLTaskImpl *task = new DLTaskImpl(request);
    return task;
}



} //YADownloader
