#ifndef DLREQUEST_H
#define DLREQUEST_H

#include <QUrl>
#include <QHash>
#include <QSharedPointer>

#include "yadownloader_global.h"

namespace YADownloader {

class DLRequestPriv;
class YADOWNLOADERSHARED_EXPORT DLRequest
{
public:
    DLRequest();
    DLRequest(const QUrl &requestUrl, const QString &savePath, const QString &saveName);
    DLRequest(const DLRequest &other);
    virtual ~DLRequest();

    DLRequest &operator =(const DLRequest &other);
    bool operator ==(const DLRequest &other) const;
    bool operator !=(const DLRequest &other) const;

    ///
    /// \brief hasSameIdentifier
    /// \param other
    /// \return if has same identifier.
    /// The identifier tuple is composed of downloadUrl requestUrl saveName savePath
    ///
    bool hasSameIdentifier(const DLRequest &other);

    ///
    /// \brief filePath real saved full file path
    /// \return
    ///
    QString filePath() const;

    void setRawHeader(const QByteArray &name, const QByteArray &value);
    QHash<QByteArray, QByteArray> rawHeaders() const;

    ///
    /// \brief requestUrl url for download request
    /// \return
    ///
    QUrl requestUrl() const;
    void setRequestUrl(const QUrl &requestUrl);

    QString savePath() const;
    void setSavePath(const QString &savePath);

    QString saveName() const;
    void setSaveName(const QString &saveName);

    int preferThreadCount() const;
    void setPreferThreadCount(int preferThreadCount);

    ///
    /// \brief downloadUrl url for real download url if url redirect
    /// \return
    ///
    QUrl downloadUrl() const;
    void setDownloadUrl(const QUrl &downloadUrl);

private:
    QUrl sortUrlQuery(const QUrl &url);
private:
    QSharedPointer<DLRequestPriv> d;
};

} //YADownloader

//Q_DECLARE_METATYPE(YADownloader::DLRequest)

#endif // DLREQUEST_H
