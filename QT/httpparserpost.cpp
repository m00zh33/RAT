#include "httpparserpost.h"

HttpParserPost::HttpParserPost(QObject *parent, QTcpSocket *tcpSocket) :
    HttpParserGet(parent, tcpSocket)
{
    this->setProperty("HttpParserPost", true);
}

void HttpParserPost::GenerateResponse()
{
    this->ParseHeaders();

    this->urlRequested = this->tcpSocket->property("url").toUrl();

    if (mapHeaders.value("content-length", "0").toInt() < 1) {
        //invalid request
        tcpSocket->setProperty("keep-alive", false);
        this->ErrorBadParameters();
    }

    intBytesRemaining = mapHeaders.value("content-length").toUInt();
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(ReadPost()));
    if (tcpSocket->bytesAvailable()) {
        QTimer::singleShot(0, this, SLOT(ReadPost()));
    }

}

void HttpParserPost::ReadPost()
{
    if (!tcpSocket->bytesAvailable()) return;
    if (tcpSocket->bytesAvailable() >= intBytesRemaining) {
        barPostData.append(tcpSocket->read(intBytesRemaining));
        intBytesRemaining = 0;
        disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(ReadPost()));
        fileRequested.close(); //was it even open?
        if (!this->ParseDynamicContent()) {
            this->ErrorNotFound();
            emit ResponseEnd();
        }
    } else {
        intBytesRemaining -= tcpSocket->bytesAvailable();
        barPostData.append(tcpSocket->readAll());
    }
}
