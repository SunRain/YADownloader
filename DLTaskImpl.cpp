#include "DLTaskImpl.h"

namespace YADownloader {

DLTaskImpl::DLTaskImpl(const DLRequest &request, DLTransmissionDatabase *db, QObject *parent)
    : DLTask(db, request, parent)
{
}

DLTaskImpl::DLTaskImpl(const DLTaskInfo &info, DLTransmissionDatabase *db,
                       const QHash<QByteArray, QByteArray> &rawHeaders, QObject *parent)
    : DLTask(db, info, rawHeaders, parent)
{

}

} //YADownloader
