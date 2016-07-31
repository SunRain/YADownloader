#include "DLTaskHeaderReader.h"

#include <QEventLoop>
#include <QTimer>
#include <QDebug>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "DLRequest.h"
#include "DLTaskStateDispatch.h"

namespace YADownloader {

DLTaskHeaderReader::DLTaskHeaderReader(DLRequest *req, DLTaskStateDispatch *dispatch, QObject *parent)
    : QThread(parent)
    , m_reply(nullptr)
    , m_dlRequest(req)
    , m_dispatch(dispatch)
    , m_requestAborted(false)
{

}

DLTaskHeaderReader::~DLTaskHeaderReader() {
    if (m_dlRequest)
        m_dlRequest = nullptr;
    if (m_dispatch)
        m_dispatch = nullptr;
}

void DLTaskHeaderReader::initFileSize()
{
    this->abort();
    this->start();
}

void DLTaskHeaderReader::abort()
{
    if (m_reply) {
        if (!m_reply->isFinished()) {
            m_requestAborted = true;
            m_reply->abort();
        }
        disconnect(m_reply, 0, 0, 0);
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (this->isRunning()) {
        this->quit();
        this->wait();
    }
}

void DLTaskHeaderReader::run() {
    QNetworkAccessManager *networkMgr = new QNetworkAccessManager();
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(HEADER_READER_TIMEOUT);

#define FREE_REPLY  if (m_reply) { \
                        if (!m_reply->isFinished()) { \
                            m_requestAborted = true; \
                            m_reply->abort(); \
                        } \
                        disconnect(m_reply, 0, 0, 0); \
                        m_reply->deleteLater(); \
                        m_reply = nullptr; \
                    }

#define FREE_BLOCK  if (timer.isActive()) timer.stop(); \
                    if (loop.isRunning()) loop.quit();

    connect(&timer, &QTimer::timeout, [&]() {
        FREE_REPLY;
        FREE_BLOCK;
    });

    bool breakFlag = false;
    int count = 3;
    QUrl rqurl(m_dlRequest->requestUrl());
    do {
        count--;
        FREE_REPLY;
        m_requestAborted = false;
        if (count < 0) {
            m_dispatch->dispatchFileSize(-1);
            break;
        }
        QNetworkRequest req = QNetworkRequest(rqurl);
        if (!m_dlRequest->rawHeaders().isEmpty()) {
            foreach (QByteArray key, m_dlRequest->rawHeaders().keys()) {
                req.setRawHeader(key, m_dlRequest->rawHeaders().value(key, QByteArray()));
            }
        }
        m_reply = networkMgr->head(req);
        if (m_reply) {
            connect (m_reply, &QNetworkReply::finished, [&]() {
                if (m_requestAborted) {
                    m_requestAborted = false;
                    FREE_REPLY;
                    if (count < 0) {
                        m_dispatch->dispatchFileSize(-1);
                        FREE_BLOCK;
                        breakFlag = true;
                        return;
                    }
                    FREE_BLOCK;
                } else {
                    QNetworkReply::NetworkError error = m_reply->error ();
                    if (error != QNetworkReply::NoError) {
                        qDebug()<<Q_FUNC_INFO<<"get head error ["<<m_reply->errorString()<<"]";
                        FREE_REPLY;
                        if (count < 0) { //re-try count out
                            m_dispatch->dispatchFileSize(-1);
                            FREE_BLOCK;
                            breakFlag = true;
                            return;
                        }
                        FREE_BLOCK;
                    } else {
                        QVariant vrt = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                        qDebug()<<Q_FUNC_INFO<<"==== http status code "<<vrt;
                        if (vrt.isNull() || !vrt.isValid()) {
                            FREE_REPLY;
                            if (count < 0) {
                                m_dispatch->dispatchFileSize(-1);
                                FREE_BLOCK;
                                breakFlag = true;
                                return;
                            }
                            FREE_BLOCK;
                        } else {
                            bool ok = false;
                            int ret = vrt.toInt(&ok);
                            if (!ok) {
                                FREE_REPLY;
                                if (count < 0) {
                                    m_dispatch->dispatchFileSize(-1);
                                    FREE_BLOCK;
                                    breakFlag = true;
                                    return;
                                }
                                FREE_BLOCK;
                            } else if (ret == 200) {
                                int size = m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
                                qDebug()<<Q_FUNC_INFO<<" file size is "<<size;
                                QUrl url = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
                                if (url.isValid ()) {
                                    qDebug()<<Q_FUNC_INFO<<"real download url: "<<url;
                                    m_dlRequest->setDownloadUrl(url.toString());
                                }
                                FREE_REPLY;
                                m_dispatch->dispatchFileSize(size);
                                FREE_BLOCK;
                                breakFlag = true;
                            } else if (ret == 302) { //redirect
                                qDebug()<<Q_FUNC_INFO<<" try  redirect";
                                count++; //redirect, we should add 1 to count
                                rqurl = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
                                FREE_REPLY;
                                FREE_BLOCK;
                                breakFlag = false;
                            } else if (count < 0) {
                                FREE_REPLY;
                                m_dispatch->dispatchFileSize(-1);
                                FREE_BLOCK;
                                breakFlag = true;
                            }
                        }
                    }
                }
                FREE_REPLY;
                FREE_BLOCK;
            });
            timer.start();
            loop.exec();
        }
    } while (!breakFlag);

    if (networkMgr) {
        networkMgr->deleteLater();
        networkMgr = nullptr;
    }
}


} //YADownloader
