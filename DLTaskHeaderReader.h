#ifndef DLTASKHEADERREADER_H
#define DLTASKHEADERREADER_H

#include <QObject>
#include <QThread>
#include "yadownloader_global.h"
#include "DLRequest.h"

class QNetworkReply;

namespace YADownloader {

static const int HEADER_READER_TIMEOUT  = 5*1000;

class DLTaskStateDispatch;
class DLTaskHeaderReader : public QThread
{
    Q_OBJECT
public:
    explicit DLTaskHeaderReader(DLRequest *req, DLTaskStateDispatch *dispatch, QObject *parent = 0);
    virtual ~DLTaskHeaderReader();
    void initFileSize();
    void abort();

    // QThread interface
protected:
    void run();
private:
    QNetworkReply *m_reply;
    DLRequest *m_dlRequest;
    DLTaskStateDispatch *m_dispatch;
    bool m_requestAborted;
};

} //YADownloader
#endif // DLTASKHEADERREADER_H
