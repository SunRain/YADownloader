#ifndef DLREQUEST_H
#define DLREQUEST_H

#include <QHash>
#include <QSharedDataPointer>

#include "yadownloader_global.h"

namespace YADownloader {

class DLRequestPriv;
class YADOWNLOADERSHARED_EXPORT DLRequest
{
public:
    enum CookieFileModeFlag {
        NotOpen = 0x0,
        ReadOnly = 0x1,
        WriteOnly = 0x2,
        ReadWrite = ReadOnly | WriteOnly
    };

    DLRequest();
    DLRequest(const QString &requestUrl, const QString &savePath, const QString &saveName);
    DLRequest(const DLRequest &other);
    virtual ~DLRequest();

    DLRequest &operator =(const DLRequest &other);
    bool operator ==(const DLRequest &other) const;
    bool operator !=(const DLRequest &other) const;

    QString cookieFilePath() const;

    CookieFileModeFlag cookieFileMode() const;

    void setCookieFilePath(const QString &cookieFilePath, CookieFileModeFlag flag = CookieFileModeFlag::ReadOnly);

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
    QString requestUrl() const;
    void setRequestUrl(const QString &requestUrl);

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
    QString downloadUrl() const;
    void setDownloadUrl(const QString &downloadUrl);

private:
    QString sortUrlQuery(const QString &url);

private:
    QSharedDataPointer<YADownloader::DLRequestPriv> d;
};

QDebug operator <<(QDebug dbg, const DLRequest& req);

} //YADownloader

//Q_DECLARE_METATYPE(YADownloader::DLRequest)

#endif // DLREQUEST_H
