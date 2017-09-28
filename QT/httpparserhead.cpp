#include "httpparserhead.h"

HttpParserHead::HttpParserHead(QObject *parent, QTcpSocket *tcpSocket) :
    HttpParserGet(parent, tcpSocket)
{
    this->boolHeadersOnly = true;
}

void HttpParserHead::SendResponseA(QByteArray barLine,
                                   QByteArray barResponse,
                                   QByteArray barEncoding)
{
    QByteArray barHeaders;
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    barResponse.clear();
    barHeaders = barLine + "\r\n";
    barHeaders.append(tcpSocket->property("keep-alive").toBool()
                       ? "keep-alive" : "close");
    barHeaders.append(
        "Content-Type: text/html; charset="+barEncoding+"\r\n"
        "Date: " + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                "ddd, dd MMM yyyy hh:mm:ss").toAscii()
        +" GMT\r\nServer: GFE/2.0\r\n\r\n");
    this->tcpSocket->write(barHeaders);

}
