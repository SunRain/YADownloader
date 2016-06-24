#include "DLTaskImpl.h"

namespace YADownloader {

DLTaskImpl::DLTaskImpl(const DLRequest &request, DLTransmissionDatabase *db, QObject *parent)
    : DLTask(db, request, parent)
{
}

} //YADownloader
