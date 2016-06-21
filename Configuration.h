#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>

class QSettings;
namespace YADownloader {

class Configuration : public QObject
{
    Q_OBJECT
public:
    explicit Configuration(QObject *parent = 0);
    virtual ~Configuration();

private:
    QSettings *m_settings;
};


} //YADownloader
#endif // CONFIGURATION_H
