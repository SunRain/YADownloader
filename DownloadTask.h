#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QObject>

namespace YADownloader {

class DownloadTask : public QObject
{
    Q_OBJECT
public:
    explicit DownloadTask(QObject *parent = 0);

signals:

public slots:
};

} //YADownloader

#endif // DOWNLOADTASK_H
