#include "DLTaskHeaderReader.h"

#include <QEventLoop>
#include <QTimer>
#include <QDebug>

//#include <QNetworkRequest>
//#include <QNetworkReply>
//#include <QNetworkAccessManager>

#include "CurlEasyHandleInitializtionClass.h"
#include "QCNetworkAccessManager.h"
#include "QCNetworkSyncReply.h"
#include "QCNetworkRequest.h"

#include "DLRequest.h"
#include "DLTaskStateDispatch.h"

using namespace QCurl;

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
//        if (!m_reply->isFinished()) {
            m_requestAborted = true;
//            m_reply->abort();
//        }
        m_reply->abort();
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
//    QNetworkAccessManager *networkMgr = new QNetworkAccessManager();
    QCNetworkAccessManager *networkMgr = new QCNetworkAccessManager();
    if (!m_dlRequest->cookieFilePath().isEmpty()) {
        DLRequest::CookieFileModeFlag flag = m_dlRequest->cookieFileMode();
        int mode = QCNetworkAccessManager::NotOpen;
        if ((flag & DLRequest::ReadOnly) == DLRequest::ReadOnly) {
            mode |= QCNetworkAccessManager::CookieFileModeFlag::ReadOnly;
        }
        if ((flag & DLRequest::WriteOnly) == DLRequest::WriteOnly) {
            mode |= QCNetworkAccessManager::CookieFileModeFlag::WriteOnly;
        }
        networkMgr->setCookieFilePath(m_dlRequest->cookieFilePath(),
                                      static_cast<QCNetworkAccessManager::CookieFileModeFlag>(mode));
    }

    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(HEADER_READER_TIMEOUT);

#define FREE_REPLY  if (m_reply) { \
                        m_requestAborted = true; \
                        m_reply->abort(); \
                        disconnect(m_reply, 0, 0, 0); \
                        m_reply->deleteLater(); \
                        m_reply = nullptr; \
                    }
#define FREE_BLOCK  if (timer.isActive()) timer.stop();


    connect(&timer, &QTimer::timeout, [&]() {
        FREE_REPLY;
        FREE_BLOCK;
    });

    QUrl reqUrl(m_dlRequest->requestUrl());
    qint64 fileSize = -1;
    m_requestAborted = false;
    QCNetworkRequest req = QCNetworkRequest(reqUrl);
    req.setFollowLocation(true);
    if (!m_dlRequest->rawHeaders().isEmpty()) {
        qDebug()<<"........................................";
        foreach (QByteArray key, m_dlRequest->rawHeaders().keys()) {
            qDebug()<<Q_FUNC_INFO<<"add header ["<<key<<"] value ["<<m_dlRequest->rawHeaders().value(key, QByteArray())<<"]";
            req.setRawHeader(key, m_dlRequest->rawHeaders().value(key, QByteArray()));
        }
    }
    m_reply = networkMgr->create(req);

    if (!m_reply) {
        m_dispatch->dispatchFileSize(-1);
        FREE_BLOCK;
    } else {
        m_reply->setReceivedContentType(CurlEasyHandleInitializtionClass::HeaderOnly);
        m_reply->setCustomHeaderFunction([&](char *buffer, size_t size)->size_t {
            if (m_requestAborted) {
                m_dispatch->dispatchFileSize(-1);
                FREE_REPLY;
                FREE_BLOCK;
            } else {
                QString header(QByteArray(buffer, static_cast<int>(size)));
                qDebug()<<Q_FUNC_INFO<<"header "<<header;
                const int pos = header.trimmed().indexOf(":");
                if (pos > 0) {
                    QString key = header.mid(0, pos).simplified();
                    QString value = header.mid(pos+1, header.length()-pos-1).simplified();
                    if (key == "Location" && !value.isEmpty()) {
                        reqUrl = value;
                    }
                    if (key == "Content-Length") {
                        fileSize = value.toLongLong();
                        qDebug()<<Q_FUNC_INFO<< "value "<<value<<" file size is "<<size;
                    }
                }
            }
            return size;
        });
        m_reply->perform();
        NetworkError e = m_reply->error();
        bool success = (e == NetworkNoError);
        if (!success) {
            QString str = m_reply->errorString();
            FREE_REPLY;
            FREE_BLOCK;
            qDebug()<<Q_FUNC_INFO<<"get file size error "<<str;
            m_dispatch->dispatchFileSize(-1);
        }
        FREE_REPLY;
        FREE_BLOCK;
        m_dlRequest->setDownloadUrl(reqUrl.toString());
        m_dispatch->dispatchFileSize(fileSize);
    }
    if (networkMgr) {
        networkMgr->deleteLater();
        networkMgr = nullptr;
    }
}


} //YADownloader
