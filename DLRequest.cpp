#include "DLRequest.h"

#include <QDebug>
#include <QDir>
#include <QSharedData>
#include <QUrl>

namespace YADownloader {

class DLRequestPriv : public QSharedData
{
public:
    DLRequestPriv()
        : requestUrl(QString())
        , downloadUrl(QString())
        , savePath(QString())
        , saveName(QString())
        , preferThreadCount(1)
    {
    }
    virtual ~DLRequestPriv() {}

    QString requestUrl;
    QString downloadUrl;
    QString savePath;
    QString saveName;
    QHash<QByteArray, QByteArray> headerList;
    int preferThreadCount;
};


DLRequest::DLRequest()
    : d(new DLRequestPriv())
{
}

DLRequest::DLRequest(const QString &url, const QString &savePath, const QString &saveName)
    : d(new DLRequestPriv())
//    : m_requestUrl(url)
//    , m_downloadUrl(m_requestUrl)
//    , m_savePath(savePath)
//    , m_saveName(saveName)
//    , m_preferThreadCount(1)
{
    d.data()->requestUrl = sortUrlQuery(url);
    d.data()->downloadUrl = sortUrlQuery(url);
    d.data()->savePath = savePath;
    d.data()->saveName = saveName;
}

DLRequest::DLRequest(const DLRequest &other)
    : d(other.d)
{
//    m_requestUrl = other.requestUrl ();
//    m_downloadUrl = other.downloadUrl ();
//    m_savePath = other.savePath ();
//    m_saveName = other.saveName ();
//    m_preferThreadCount = other.preferThreadCount ();

//    initDir ();
}

DLRequest::~DLRequest()
{
}

DLRequest &DLRequest::operator =(const DLRequest &other)
{
    if (this != &other)
        d.operator =(other.d);
    return *this;
}

bool DLRequest::operator ==(const DLRequest &other) const
{
    return d.data()->downloadUrl == other.d.data()->downloadUrl
            && d.data()->headerList == other.d.data()->headerList
            && d.data()->preferThreadCount == other.d.data()->preferThreadCount
            && d.data()->requestUrl == other.d.data()->requestUrl
            && d.data()->saveName == other.d.data()->saveName
            && d.data()->savePath == other.d.data()->savePath;
}

bool DLRequest::operator !=(const DLRequest &other) const {
    return !operator == (other);
}

bool DLRequest::hasSameIdentifier(const DLRequest &other)
{
    return d.data()->downloadUrl == other.d.data()->downloadUrl
            && d.data()->requestUrl == other.d.data()->requestUrl
            && d.data()->saveName == other.d.data()->saveName
            && d.data()->savePath == other.d.data()->savePath;
}

QString DLRequest::filePath() const
{
    if (d.data()->savePath.isEmpty() || d.data()->saveName.isEmpty()
            || d.data()->downloadUrl.isEmpty()) {
        return QString();
    }
    QString savePath = d.data()->savePath;
    QString saveName = d.data()->saveName;

    if (savePath.startsWith("file://")) { //remove file:// scheme
#if defined(Q_OS_WIN) || defined(Q_OS_WIN32)
        savePath = savePath.right(savePath.length() - 8);
#else
        savePath = savePath.right(savePath.length() - 7);
#endif
    }
    if (savePath.endsWith("/")) { //remove last /
        savePath = savePath.left(savePath.length() -1);
    }
    if (saveName.startsWith("/")) { //remove first /
        saveName = saveName.right(saveName.length() -1);
    }
    return QString("%1/%2").arg(savePath).arg(saveName);
}

void DLRequest::setRawHeader(const QByteArray &name, const QByteArray &value)
{
    if (!name.isEmpty ())
        d.data()->headerList.insert(name, value);
}

QHash<QByteArray, QByteArray> DLRequest::rawHeaders() const {
    return d.data()->headerList;
}

QString DLRequest::requestUrl() const
{
    return d.data()->requestUrl;
}

void DLRequest::setRequestUrl(const QString &url)
{
    d.data()->requestUrl = sortUrlQuery(url);
    d.data()->downloadUrl = d.data()->requestUrl;
}

QString DLRequest::savePath() const
{
    return d.data()->savePath;
}

void DLRequest::setSavePath(const QString &savePath)
{
    d.data()->savePath = savePath;
}

QString DLRequest::saveName() const
{
    return d.data()->saveName;
}

void DLRequest::setSaveName(const QString &saveName)
{
    d.data()->saveName = saveName;
}

int DLRequest::preferThreadCount() const
{
    return d.data()->preferThreadCount;
}

void DLRequest::setPreferThreadCount(int preferThreadCount)
{
    d.data()->preferThreadCount = preferThreadCount;
}

QString DLRequest::downloadUrl() const
{
    return d.data()->downloadUrl;
}

void DLRequest::setDownloadUrl(const QString &downloadUrl)
{
    d.data()->downloadUrl = sortUrlQuery(downloadUrl);
}

QString DLRequest::sortUrlQuery(const QString &url)
{
    if (url.isEmpty())
        return QString();
    QStringList list = url.split("?");
    if (list.isEmpty() || list.size() != 2)
        return url;
    QStringList querylist = list.last().split("&");
    qStableSort(querylist.begin(), querylist.end(), qGreater<QString>());
    QUrl u(QString("%1?%2").arg(list.at(0)).arg(querylist.join("&")));
    return QString(u.toEncoded());
}

QDebug operator <<(QDebug dbg, const DLRequest &req)
{
    QStringList list;
    foreach (QByteArray key, req.rawHeaders()) {
        list.append(QString("%1=%2").arg(QString(key)).arg(QString(req.rawHeaders().value(key))));
    }
    return dbg<<QString("DLRequest [requestUrl=%1], [downloadUrl=%2], [filePath=%3], [preferThreadCount=%4]")
                .arg(req.requestUrl()).arg(req.downloadUrl())
                .arg(req.filePath()).arg(req.preferThreadCount())
             <<QString(" [rawHeaders >>> %1]").arg(list.join(","));
}

} //
