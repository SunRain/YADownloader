#include "DLTransmissionDatabase.h"

#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "DLTransmissionDatabaseKeys.h"

namespace YADownloader {

DLTransmissionDatabase::DLTransmissionDatabase(QObject *parent)
    : QObject(parent)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir(dataPath);
    if (!dir.exists())
        dir.mkpath(dataPath);
    m_cfgFile = QString("%1/data.json").arg(dataPath);

}

DLTransmissionDatabase::~DLTransmissionDatabase()
{
    qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<";
    saveToLocalStorage();
}

void DLTransmissionDatabase::appendTaskInfo(const DLTaskInfo &info)
{
    m_infoHash.insert(info.uid(), info);
}

DLTaskInfoList DLTransmissionDatabase::list() const
{
    return m_infoHash.values();
}

void DLTransmissionDatabase::flush()
{
    saveToLocalStorage();
}

void DLTransmissionDatabase::loadFromLocalStorage()
{
    QFile file(m_cfgFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  {
        qDebug()<<Q_FUNC_INFO<<"Can't open exist data files";
        file.close();
        return;
    }
    QByteArray qba = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(qba, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug()<<Q_FUNC_INFO<<"Parse json error => "<<error.errorString ();
        return;
    }
    //TODO



    file.close();
}

void DLTransmissionDatabase::saveToLocalStorage()
{
    qDebug()<<Q_FUNC_INFO<<"===========";
    QVariantList all;
    foreach (QString key, m_infoHash.keys()) {
        DLTaskInfo info = m_infoHash.value(key);
        QVariantHash top;
        top.insert(TASK_INFO_UID, info.uid());
        top.insert(TASK_INFO_DL_URL, info.downloadUrl());
        top.insert(TASK_INFO_REQ_URL, info.requestUrl());
        top.insert(TASK_INFO_FILE_PATH, info.filePath());
        top.insert(TASK_INFO_READY_SIZE, QString::number(info.readySize()));
        top.insert(TASK_INFO_TOTAL_SIZE, QString::number(info.totalSize()));
        QVariantList list;
        foreach (PeerInfo p, info.peerList()) {
            QVariantHash hash;
            hash.insert(TASK_PEER_COMPLETED_CNT, QString::number(p.completedCount()));
            hash.insert(TASK_PEER_START_IDX, QString::number(p.startIndex()));
            hash.insert(TASK_PEER_END_IDX, QString::number(p.endIndex()));
            list.append(hash);
        }
        top.insert(TASK_INFO_PEER_LIST, list);
        all.append(top);
    }
    QJsonDocument doc = QJsonDocument::fromVariant(all);
    QByteArray qba = doc.toJson();

//    qDebug()<<Q_FUNC_INFO<<" value "<<qba;

    QFile file(m_cfgFile);
    if (file.exists()) {
        if (!file.remove())
            qWarning()<<Q_FUNC_INFO<<"Can't remove file!!";
    }
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning()<<Q_FUNC_INFO<<"Can't open to write file";
        return;
    }
    QTextStream out(&file);
    out << qba;
    out.flush();
    file.close();
}

} //YADownloader
