#ifndef DLTRANSMISSIONDATABASE_H
#define DLTRANSMISSIONDATABASE_H

#include <QObject>

namespace YADownloader {

class DLTransmissionDatabase : public QObject
{
    Q_OBJECT
public:
    explicit DLTransmissionDatabase(QObject *parent = 0);

signals:

public slots:
};

} //YADownloader
#endif // DLTRANSMISSIONDATABASE_H
