#ifndef DLTASKIMPL_H
#define DLTASKIMPL_H

#include <QObject>

#include "yadownloader_global.h"
#include "DLTask.h"

namespace YADownloader {

class DLTransmissionDatabase;
class YADOWNLOADERSHARED_EXPORT DLTaskImpl : public DLTask
{
    Q_OBJECT
public:
    explicit DLTaskImpl(const DLRequest &request, DLTransmissionDatabase *db, QObject *parent = 0);

};

} //YADownloader

#endif // DLTASKIMPL_H
