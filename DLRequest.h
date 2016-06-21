#ifndef DLREQUEST_H
#define DLREQUEST_H

#include <QUrl>
#include <QHash>

#include "yadownloader_global.h"

namespace YADownloader {

class YADOWNLOADERSHARED_EXPORT DLRequest
{
public:
    explicit DLRequest();
    explicit DLRequest(const QUrl &requestUrl, const QString &savePath, const QString &saveName);
    explicit DLRequest(const DLRequest &other);
    virtual ~DLRequest();

    DLRequest &operator =(const DLRequest &other);
    bool operator ==(const DLRequest &other) const;
    inline bool operator !=(const DLRequest &other) const {
        return !operator == (other);
    }

    inline QString filePath() {
        return QString("%1/%2").arg (savePath ()).arg (saveName ());
    }

    void setRawHeader(const QByteArray &name, const QByteArray &value);
    inline QHash<QByteArray, QByteArray> rawHeaders() const {
        return m_headerList;
    }

    QUrl requestUrl() const;
    void setRequestUrl(const QUrl &requestUrl);

    QString savePath() const;
    void setSavePath(const QString &savePath);

    QString saveName() const;
    void setSaveName(const QString &saveName);

    int preferThreadCount() const;
    void setPreferThreadCount(int preferThreadCount);

    QUrl downloadUrl() const;
    void setDownloadUrl(const QUrl &downloadUrl);

private:
    QUrl m_requestUrl;
    QUrl m_downloadUrl;
    QString m_savePath;
    QString m_saveName;
    QHash<QByteArray, QByteArray> m_headerList;
    int m_preferThreadCount;
};

} //YADownloader

//Q_DECLARE_METATYPE(YADownloader::DLRequest)

#endif // DLREQUEST_H
