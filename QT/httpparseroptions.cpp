#include "httpparseroptions.h"

HttpParserOptions::HttpParserOptions( QObject * parent, QTcpSocket * tcpSocket )
    : HttpParserGeneric(parent, tcpSocket)
{

}

void HttpParserOptions::GenerateResponse()
{
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    QByteArray headers = "HTTP/1.";
    tmrTimeout.stop();
    headers.append(this->tcpSocket->property("http-protocol").toUInt() == 11
                   ? "1" : "0");
    headers.append(" 200 OK");
    headers.append("\r\nDate: " + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                   "ddd, dd MMM yyyy hh:mm:ss").toAscii());
    headers.append("  GMT\r\nServer: gws\r\nContent-Length: 0"
                   "\r\nAccept-Ranges: none\r\nDASL: <DAV:sql>"
                   "\r\nDAV: 1, 2"
                   "\r\nPublic: OPTIONS, TRACE, GET, HEAD, DELETE, PUT, POST, "
                   "COPY, MOVE, MKCOL, PROPFIND, PROPPATCH, LOCK, UNLOCK, "
                   "SEARCH"
                   "\r\nAllow: OPTIONS, TRACE, GET, HEAD, COPY, "
                   "PROPFIND, SEARCH, LOCK, UNLOCK"
                   "\r\nCache-Control: private"
                   "\r\nConnection: ");
    headers.append(tcpSocket->property("keep-alive").toBool()
                   ? "keep-alive" : "close");
    headers.append("\r\n\r\n");
    this->tcpSocket->write(headers);
    if (tcpSocket->property("keep-alive").toBool()) {
        tcpSocket->setProperty("http-state", kStateTransitional);
    } else {
        tcpSocket->setProperty("http-state", kStateMeta);
        tcpSocket->disconnectFromHost();
    }
    emit TerminateParser();
}
