#ifndef YADOWNLOADER_H
#define YADOWNLOADER_H

#include <QObject>

#include "yadownloader_global.h"

namespace YADownloader {

class YADownloader : public QObject
{
    Q_OBJECT
public:
    explicit YADownloader(QObject *parent = 0);
    virtual ~YADownloader();

    void NewTask(const QUrl &url, const QString &savePath);

};

} //YADownloader

#endif // YADOWNLOADER_H
