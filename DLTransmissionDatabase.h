#ifndef DLTRANSMISSIONDATABASE_H
#define DLTRANSMISSIONDATABASE_H

#include <QObject>
#include <QHash>

#include "SingletonPointer.h"
#include "DLTaskInfo.h"
#include "DLTaskPeer.h"
#include "DLTaskInfo.h"

namespace YADownloader {

class DLTransmissionDatabase : public QObject
{
    Q_OBJECT
public:
    explicit DLTransmissionDatabase(QObject *parent = 0);
    virtual ~DLTransmissionDatabase();

    void removeTaskInfo(const DLTaskInfo &info);
    void appendTaskInfo(const DLTaskInfo &info);

    DLTaskInfoList list() const;

    void flush();
private:
    void loadFromLocalStorage();
    void saveToLocalStorage();

private:
    ///
    /// \brief m_infoHash key is DLTaskInfo.requestUrl()+DLTaskInfo.filePath()
    ///
    QHash<QString, DLTaskInfo> m_infoHash;
    QString m_cfgFile;

};

} //YADownloader
#endif // DLTRANSMISSIONDATABASE_H
