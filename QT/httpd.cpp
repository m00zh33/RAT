#include "httpd.h"

httpd::httpd()
{
    if (!this->tcpServer.listen(P_BIND, P_PORT))
    {
        this->deleteLater();
        return;
    }
    connect(&this->tcpServer, SIGNAL(newConnection()),
            this, SLOT(requestNew()));
}

void httpd::requestNew()
{
    QTcpServer *tcpServerSender = static_cast<QTcpServer *>(sender());
    if (!tcpServerSender->hasPendingConnections()) {
        return;
    }
    QTcpSocket *tcpSocket = tcpServerSender->nextPendingConnection();
    quint16 intCount = mapRequests.value(
            tcpSocket->peerAddress().toString(), 0);
    if (tcpSocket->peerAddress().isNull()
        || !tcpSocket->isValid()
        || intCount >= P_MAX_FROM_IP)
    {
        tcpSocket->abort();
        tcpSocket->deleteLater();
        return;
    }
    intCount++;
    mapRequests.insert(tcpSocket->peerAddress().toString(), intCount);

    connect(tcpSocket, SIGNAL(disconnected()),
            tcpSocket, SLOT(deleteLater()));
    connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(requestDataArrived()));
    connect(tcpSocket, SIGNAL(disconnected()),
            this, SLOT(requestClosed()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(requestError(QAbstractSocket::SocketError)));

    QTimer *tmrTimeout = new QTimer(tcpSocket);
    tcpSocket->setProperty("keep-alive-requests",0);
    tmrTimeout->setSingleShot(true);
    tmrTimeout->setInterval(P_TIMEOUT_REQUEST);
    connect(tmrTimeout, SIGNAL(timeout()), this, SLOT(requestTimeout()));
    tmrTimeout->start();
    mapTimeouts.insert(tcpSocket, tmrTimeout);
}

void httpd::requestClosed()
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(sender());
    this->requestClosed(tcpSocket);
}

void httpd::requestClosed(QTcpSocket *tcpSocket) {
    if (!tcpSocket || !tcpSocket->isValid()) {
        tcpSocket->deleteLater();
        return;
    }
    quint16 intCount = mapRequests.value(
            tcpSocket->peerAddress().toString(), 0);
    if (intCount) {
        intCount--;
        if (intCount) {
            mapRequests.insert(tcpSocket->peerAddress().toString(), intCount);
        } else {
            mapRequests.remove(tcpSocket->peerAddress().toString());
        }
    }
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    } else {
        tcpSocket->abort();
        tcpSocket->deleteLater();
    }
}

void httpd::requestDataArrived()
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(sender());
    this->requestDataArrived(tcpSocket);
}

void httpd::requestDataArrived(QTcpSocket *tcpSocket)
{
    QByteArray barHead, barTmp;
    uint intHttpParser, intHttpState;
    HttpParserGeneric *ptrParser = 0;

    if (!tcpSocket->bytesAvailable()) return;

    intHttpState = tcpSocket->property("http-state").toUInt();

    if (intHttpState == kStateTransitional) return;

    if (intHttpState != kStateBody) {
        bool boolSaveHead = false, boolHead = true;

        barHead = tcpSocket->property("http-head").toByteArray();
        do {
            switch (intHttpState)
            {
                case kStateNone:
                    tcpSocket->setProperty("http-state", kStateHead);
                    intHttpState = kStateHead;
                    tcpSocket->setProperty("http-parser", kParseNone);
                case kStateHead:
                    barTmp = tcpSocket->readLine(0);
                    if (barTmp == "\r\n" || barTmp == "\n") {
                        tcpSocket->setProperty("http-state", kStateBody);
                        intHttpState = kStateBody;
                        QTimer *tmrTimeout = mapTimeouts.value(tcpSocket, 0);
                        if (tmrTimeout) {
                            tmrTimeout->stop();
                            tmrTimeout->deleteLater();
                        }
                        mapTimeouts.remove(tcpSocket);
                        continue;
                    }
                    barHead.append(barTmp);
                    boolSaveHead = true;
                    break;
                case kStateBody:
                    boolHead = false;
                    break;
                default:
                    return;
            }
        } while (boolHead && tcpSocket->bytesAvailable());
        if (boolSaveHead) tcpSocket->setProperty("http-head",barHead);
    }
    if (intHttpState != kStateBody) return;

    intHttpParser = tcpSocket->property("http-parser").toUInt();
    if (intHttpParser == kParseNone && !this->mapParser.contains(tcpSocket)) {
        //run header parser and update intHttpParser
        intHttpParser = this->parseHead(tcpSocket);
        if (intHttpParser) {
            tcpSocket->setProperty("http-parser", intHttpParser);
        }
    } else {
        //throw that into the appropriate body parse
        ptrParser = this->mapParser.value(tcpSocket);
        //push data to ptrParser
        return;
    }
    //create new parser and map it

    if (tcpSocket->property("keep-alive-requests").toUInt() >= P_KEEPALIVE_MAX)
        tcpSocket->setProperty("keep-alive", false);

    switch (intHttpParser) {
        case kParseNone:
            //throw 400 bad request and close connection
            ptrParser = new HttpParserGeneric(this, tcpSocket);
            break;
        case kParseGet:
            ptrParser = static_cast <HttpParserGeneric *>
                        (new HttpParserGet(this, tcpSocket));
            break;
        case kParseHead:
            ptrParser = static_cast <HttpParserGeneric *>
                        (new HttpParserHead(this, tcpSocket));
            break;
        case kParsePost:
            ptrParser = static_cast <HttpParserGeneric *>
                        (new HttpParserPost(this, tcpSocket));
            break;
        case kParseBitsPost:
            ptrParser = static_cast <HttpParserGeneric *>
                        (new HttpParserBitsPost(this, tcpSocket));
            break;
        case kParseOptions:
            ptrParser = static_cast <HttpParserGeneric *>
                    (new HttpParserOptions(this, tcpSocket));
            break;
        case kParseProfind:
            ptrParser = static_cast <HttpParserGeneric *>
                    (new HttpParserProfind(this, tcpSocket));
            break;
    }
    this->mapParser.insert(tcpSocket, ptrParser);
    connect(ptrParser, SIGNAL(TerminateParser()), SLOT(parserTermination()));
    QTimer::singleShot(0, ptrParser, SLOT(GenerateResponse()));
}

int httpd::parseHead(QTcpSocket *tcpSocket)
{
    QByteArray barHead = tcpSocket->property("http-head").toByteArray();
    QList<QByteArray> lines = barHead.replace("\r\n","\n").split('\n');
    QList<QByteArray> elements = lines.takeFirst().split(' ');
    QUrl urlRequest;
    QByteArray strLine;
    quint8 intRetType;
    qint16 intSepPos;
    if (elements.count() < 3) return kParseNone;
    if (elements.at(2) == "HTTP/1.0") {
        tcpSocket->setProperty("http-protocol",10);
        tcpSocket->setProperty("keep-alive",false);
    } else if (elements.at(2) == "HTTP/1.1") {
        tcpSocket->setProperty("http-protocol",11);
        tcpSocket->setProperty("keep-alive",true);
    } else {
        return kParseNone;
    }
    if (elements.at(1).at(0) == '/') {
        urlRequest.setEncodedUrl(
                QByteArray(tcpSocket->property("QSslSocket").toBool()
                    ? "https" : "http") + "://localhost" + elements.at(1));
        foreach (strLine, lines) {
            if (strLine == "") break;
            intSepPos = strLine.indexOf(": ");
            if (intSepPos == -1) continue;
            if (strLine.left(intSepPos).toLower() == "host") {
                strLine = strLine.mid(intSepPos+2);
                intSepPos = strLine.indexOf(":");
                if (intSepPos == -1) {
                    urlRequest.setEncodedHost(strLine);
                } else {
                    urlRequest.setEncodedHost(strLine.left(intSepPos));
                    urlRequest.setPort(strLine.mid(intSepPos+1).toInt());
                }
            } else if (strLine.left(intSepPos).toLower() == "connection") {
                if (strLine.mid(intSepPos+2).toLower() == "keep-alive") {
                    tcpSocket->setProperty("keep-alive",true);
                } else if (strLine.mid(intSepPos+2).toLower() == "close") {
                    tcpSocket->setProperty("keep-alive",false);
                }
            }

        }
    } else {
        urlRequest.setEncodedUrl(elements.at(1));
        urlRequest.setScheme(tcpSocket->property("QSslSocket").toBool()
                             ? "https" : "http");
    }
    if (!urlRequest.isValid()) return kParseNone;

    if (elements.first() == "GET") {
        intRetType = kParseGet;
    } else if (elements.first() == "POST") {
        intRetType = kParsePost;
    } else if (elements.first() == "HEAD") {
        intRetType = kParseHead;
    } else if (elements.first() == "BITS_POST") {
        intRetType = kParseBitsPost;
    } else if (elements.first() == "OPTIONS") {
        intRetType = kParseOptions;
    } else if (elements.first() == "PROPFIND") {
        intRetType = kParseProfind;
    } else {
        intRetType = kParseNone;
    }
    if (intRetType)
        tcpSocket->setProperty("url", urlRequest);
        return intRetType;
}

void httpd::requestError(QAbstractSocket::SocketError)
{
    QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(sender());
    if (!tcpSocket) return;
    quint16 intCount = mapRequests.value(
            tcpSocket->peerAddress().toString(), 0);
    if (intCount) {
        intCount--;
        if (intCount) {
            mapRequests.insert(tcpSocket->peerAddress().toString(), intCount);
        } else {
            mapRequests.remove(tcpSocket->peerAddress().toString());
        }
    }
    tcpSocket->abort();
    tcpSocket->deleteLater();
}

void httpd::requestTimeout()
{
    QTimer *tmrTimeout = static_cast<QTimer *>(sender());
    QTcpSocket *tcpSocket = static_cast<QTcpSocket *>(tmrTimeout->parent());
    tcpSocket->setProperty("http-state", kStateMeta);
    this->requestClosed(tcpSocket);
    tmrTimeout->deleteLater();
}

void httpd::parserTermination()
{
    HttpParserGeneric *parser = static_cast<HttpParserGeneric *>(sender());
    QTcpSocket *tcpSocket = NULL;
    tcpSocket = parser->getTcpSocket();
    parser->deleteLater();
    this->mapParser.remove(tcpSocket);
    if (!tcpSocket || !tcpSocket->isValid()
      || tcpSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    if (!tcpSocket->property("keep-alive").toBool()) {
        return;
    }

    quint16 intKACount = tcpSocket->property("keep-alive-requests").toUInt();
    intKACount++;
    tcpSocket->setProperty("keep-alive-requests", intKACount);
    if (intKACount > P_KEEPALIVE_MAX) {
        this->requestClosed(tcpSocket);
        return;
    }

    tcpSocket->setProperty("http-state", kStateHead);

    tcpSocket->setProperty("http-head", "");
    tcpSocket->setProperty("keep-alive", false);
    tcpSocket->setProperty("url", QUrl());
    tcpSocket->setProperty("http-protocol", 0);
    tcpSocket->setProperty("http-parser", 0);


    QTimer *tmrTimeout = new QTimer(tcpSocket);
    tmrTimeout->setSingleShot(true);
    tmrTimeout->setInterval(P_KEEPALIVE_TIMEOUT);
    connect(tmrTimeout, SIGNAL(timeout()), this, SLOT(requestTimeout()));
    tmrTimeout->start();
    mapTimeouts.insert(tcpSocket, tmrTimeout);

    if (tcpSocket->bytesAvailable()) {
        this->requestDataArrived(tcpSocket);
    }
}
