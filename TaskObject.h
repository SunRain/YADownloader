#ifndef TASKOBJECT_H
#define TASKOBJECT_H

#include <QObject>
#include <QUrl>

#include "yadownloader_global.h"

namespace YADownloader {

class YADOWNLOADERSHARED_EXPORT TaskObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString savePath READ savePath WRITE setSavePath NOTIFY savePathChanged)

    Q_DISABLE_COPY(TaskObject)
public:
    explicit TaskObject(QObject *parent = 0);



    inline QUrl url() const {
        return m_url;
    }

    inline QString savePath() const {
        return m_savePath;
    }

protected:

signals:
    void urlChanged(QUrl url);
    void savePathChanged(QString savePath);

public slots:
    void setUrl(QUrl url);
    void setSavePath(QString savePath);

private:
    QUrl m_url;
    QString m_savePath;
};

} //YADownloader
#endif // TASKOBJECT_H
