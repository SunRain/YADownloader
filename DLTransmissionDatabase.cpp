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
    loadFromLocalStorage();
}

DLTransmissionDatabase::~DLTransmissionDatabase()
{
    qDebug()<<Q_FUNC_INFO<<"<<<<<<<<<<<<";
    saveToLocalStorage();
}

void DLTransmissionDatabase::appendTaskInfo(const DLTaskInfo &info)
{
    m_infoHash.insert(info.requestUrl(), info);
}

DLTaskInfoList DLTransmissionDatabase::list() const
{
//    qDebug()<<Q_FUNC_INFO<<">>>>>>>>> "<<m_infoHash.values();
    return m_infoHash.values();
}

void DLTransmissionDatabase::flush()
{
    saveToLocalStorage();
}

void DLTransmissionDatabase::loadFromLocalStorage()
{
    m_infoHash.clear();

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
        file.close();
        return;
    }
    QVariantList list = doc.toVariant().toList();
    foreach (QVariant vaa, list) {
        QVariantMap va = vaa.toMap();
        DLTaskInfo info;
        info.setDownloadUrl(va.value(TASK_INFO_DL_URL).toString());
        info.setFilePath(va.value(TASK_INFO_FILE_PATH).toString());
        info.setReadySize(va.value(TASK_INFO_READY_SIZE).toULongLong());
        info.setRequestUrl(va.value(TASK_INFO_REQ_URL).toString());
        info.setTotalSize(va.value(TASK_INFO_TOTAL_SIZE).toULongLong());
//        info.setHash(va.value(TASK_INFO_UID).toString());
        QVariantList vaList = va.value(TASK_INFO_PEER_LIST).toList();
        DLTaskPeerInfoList plist;
        foreach (QVariant hh, vaList) {
            QVariantMap h = hh.toMap();
            DLTaskPeerInfo p;
            p.setStartIndex(h.value(TASK_PEER_START_IDX).toString().toULongLong());
            p.setEndIndex(h.value(TASK_PEER_END_IDX).toString().toULongLong());
            p.setCompletedCount(h.value(TASK_PEER_COMPLETED_CNT).toString().toULongLong());
            p.setFilePath(h.value(TASK_INFO_FILE_PATH).toString());
            plist.append(p);
        }
        info.setPeerList(plist);
        m_infoHash.insert(info.requestUrl(), info);
    }
    file.close();
}

void DLTransmissionDatabase::saveToLocalStorage()
{
    qDebug()<<Q_FUNC_INFO<<"===========";
    QVariantList all;
    foreach (QString key, m_infoHash.keys()) {
        DLTaskInfo info = m_infoHash.value(key);
        QVariantHash top;
//        top.insert(TASK_INFO_UID, info.hash());
        top.insert(TASK_INFO_DL_URL, info.downloadUrl());
        top.insert(TASK_INFO_REQ_URL, info.requestUrl());
        top.insert(TASK_INFO_FILE_PATH, info.filePath());
        top.insert(TASK_INFO_READY_SIZE, QString::number(info.readySize()));
        top.insert(TASK_INFO_TOTAL_SIZE, QString::number(info.totalSize()));
        QVariantList list;
        foreach (DLTaskPeerInfo p, info.peerList()) {
            QVariantHash hash;
            hash.insert(TASK_PEER_COMPLETED_CNT, QString::number(p.dlCompleted()));
            hash.insert(TASK_PEER_START_IDX, QString::number(p.startIndex()));
            hash.insert(TASK_PEER_END_IDX, QString::number(p.endIndex()));
            hash.insert(TASK_INFO_FILE_PATH, p.filePath());
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
