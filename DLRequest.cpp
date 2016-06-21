#include "DLRequest.h"

#include <QDir>

namespace YADownloader {

DLRequest::DLRequest()
    : m_requestUrl(QUrl())
    , m_downloadUrl(m_requestUrl)
    , m_savePath(QString())
    , m_saveName(QString())
    , m_preferThreadCount(1)
{
}

DLRequest::DLRequest(const QUrl &url, const QString &savePath, const QString &saveName)
    : m_requestUrl(url)
    , m_downloadUrl(m_requestUrl)
    , m_savePath(savePath)
    , m_saveName(saveName)
    , m_preferThreadCount(1)
{
//    initDir ();
}

DLRequest::DLRequest(const DLRequest &other)
{
    m_requestUrl = other.requestUrl ();
    m_downloadUrl = other.downloadUrl ();
    m_savePath = other.savePath ();
    m_saveName = other.saveName ();
    m_preferThreadCount = other.preferThreadCount ();

//    initDir ();
}

DLRequest::~DLRequest()
{
}

DLRequest &DLRequest::operator =(const DLRequest &other)
{
    if (*this == other)
        return  *this;
    this->m_requestUrl = other.requestUrl ();
    this->m_downloadUrl = other.downloadUrl ();
    this->m_savePath = other.savePath ();
    this->m_saveName = other.saveName ();
    this->m_preferThreadCount = other.preferThreadCount ();
    return *this;
}

bool DLRequest::operator ==(const DLRequest &other) const
{
    return this->m_requestUrl == other.requestUrl ()
            && this->m_downloadUrl == other.downloadUrl ()
            && this->m_savePath == other.savePath ()
            && this->m_saveName == other.saveName ();
}

void DLRequest::setRawHeader(const QByteArray &name, const QByteArray &value)
{
    if (!name.isEmpty ())
        m_headerList.insert (name, value);
}

QUrl DLRequest::requestUrl() const
{
    return m_requestUrl;
}

void DLRequest::setRequestUrl(const QUrl &url)
{
    m_requestUrl = url;
}

QString DLRequest::savePath() const
{
    return m_savePath;
}

void DLRequest::setSavePath(const QString &savePath)
{
    m_savePath = savePath;
}

QString DLRequest::saveName() const
{
    return m_saveName;
}

void DLRequest::setSaveName(const QString &saveName)
{
    m_saveName = saveName;
}

int DLRequest::preferThreadCount() const
{
    return m_preferThreadCount;
}

void DLRequest::setPreferThreadCount(int preferThreadCount)
{
    m_preferThreadCount = preferThreadCount;
}

QUrl DLRequest::downloadUrl() const
{
    return m_downloadUrl;
}

void DLRequest::setDownloadUrl(const QUrl &downloadUrl)
{
    m_downloadUrl = downloadUrl;
}

} //
