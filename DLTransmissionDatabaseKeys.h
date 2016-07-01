#ifndef DLTRANSMISSIONDATABASEKEYS_H
#define DLTRANSMISSIONDATABASEKEYS_H

#include <QString>
#include <QObject>
#include <QJSEngine>
#include <QQmlEngine>

namespace YADownloader {

const static QString TASK_INFO_UID("uid");
const static QString TASK_INFO_DL_URL("downloadUrl");
const static QString TASK_INFO_REQ_URL("requestUrl");
const static QString TASK_INFO_TOTAL_SIZE("totalSize");
const static QString TASK_INFO_READY_SIZE("readySize");
const static QString TASK_INFO_PEER_LIST("peerList");
const static QString TASK_INFO_FILE_PATH("filePath");
const static QString TASK_INFO_STATUS("taskStatus");
const static QString TASK_PEER_START_IDX("startIndex");
const static QString TASK_PEER_END_IDX("endIndex");
const static QString TASK_PEER_COMPLETED_CNT("completedCount");

class DLTransmissionDatabaseKeysName : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString keyUid READ keyUid CONSTANT)
    Q_PROPERTY(QString keyDownloadUrl READ keyDownloadUrl CONSTANT)
    Q_PROPERTY(QString keyRequestUrl READ keyRequestUrl CONSTANT)
    Q_PROPERTY(QString keyTotalSize READ keyTotalSize CONSTANT)
    Q_PROPERTY(QString keyReadySize READ keyReadySize CONSTANT)
    Q_PROPERTY(QString keyPeerList READ keyPeerList CONSTANT)
    Q_PROPERTY(QString keyFilePath READ keyFilePath CONSTANT)
    Q_PROPERTY(QString keyStartIndex READ keyStartIndex CONSTANT)
    Q_PROPERTY(QString keyEndIndex READ keyEndIndex CONSTANT)
    Q_PROPERTY(QString keyCompletedCount READ keyCompletedCount CONSTANT)
    Q_PROPERTY(QString KeyTaskStatus READ taskStatus CONSTANT)
public:
    DLTransmissionDatabaseKeysName(QObject *parent = 0)
        : QObject(parent)
    {
    }
    virtual ~DLTransmissionDatabaseKeysName() {}


    static QObject *qmlSingleton(QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new DLTransmissionDatabaseKeysName();
    }
    static QStringList keys() {
        QStringList list;
        list << TASK_INFO_DL_URL
             << TASK_INFO_FILE_PATH
             << TASK_INFO_PEER_LIST
             << TASK_INFO_READY_SIZE
             << TASK_INFO_REQ_URL
             << TASK_INFO_TOTAL_SIZE
             << TASK_INFO_UID
             << TASK_PEER_COMPLETED_CNT
             << TASK_PEER_END_IDX
             << TASK_PEER_START_IDX
             << TASK_INFO_STATUS;
        return list;
    }

    QString keyUid() const
    {
        return TASK_INFO_UID;
    }

    QString keyDownloadUrl() const
    {
        return TASK_INFO_DL_URL;
    }

    QString keyRequestUrl() const
    {
        return TASK_INFO_REQ_URL;
    }

    QString keyTotalSize() const
    {
        return TASK_INFO_TOTAL_SIZE;
    }

    QString keyReadySize() const
    {
        return TASK_INFO_READY_SIZE;
    }

    QString keyPeerList() const
    {
        return TASK_INFO_PEER_LIST;
    }

    QString keyFilePath() const
    {
        return TASK_INFO_FILE_PATH;
    }

    QString keyStartIndex() const
    {
        return TASK_PEER_START_IDX;
    }

    QString keyEndIndex() const
    {
        return TASK_PEER_END_IDX;
    }

    QString keyCompletedCount() const
    {
        return TASK_PEER_COMPLETED_CNT;
    }
    QString taskStatus() const
    {
        return TASK_INFO_STATUS;
    }
};

}
#endif // DLTRANSMISSIONDATABASEKEYS_H
