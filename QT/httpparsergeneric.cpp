#include "httpparsergeneric.h"

HttpParserGeneric::HttpParserGeneric(
        QObject * parent, QTcpSocket * tcpSocket )
{
    connect(parent, SIGNAL(destroyed()),
            this, SLOT(deleteLater()));
    this->tcpSocket = tcpSocket;
    connect(tcpSocket, SIGNAL(destroyed()),
            this, SLOT(deleteLater()));
    tmrTimeout.setInterval(P_TIMEOUT_READ);
    tmrTimeout.setSingleShot(true);
    connect(&tmrTimeout, SIGNAL(timeout()), this, SLOT(Timeout()));
    connect(this, SIGNAL(TerminateParser()), &tmrTimeout, SLOT(stop()));
    tmrTimeout.start();
    this->setProperty("HttpParserGeneric",true);
}

HttpParserGeneric::~HttpParserGeneric()
{
    tmrTimeout.stop();
}

void HttpParserGeneric::GenerateResponse()
{
    static QByteArray barLine = "HTTP/1.0 400 Bad Request";
    QFile fileError400;
    fileError400.setFileName("htstatic/400.html");
    if (fileError400.exists()
      && fileError400.open(QIODevice::ReadOnly | QIODevice::Text)) {
        this->SendResponse(barLine, fileError400.readAll());
        fileError400.close();
    } else {
        this->SendResponse(barLine, barLine);
    }
    this->tcpSocket->disconnectFromHost();
    emit TerminateParser();
    this->tcpSocket->setProperty("http-state", kStateMeta);
}

void HttpParserGeneric::SendResponse(QByteArray barLine, QString strResponse)
{
    this->SendResponseA(barLine, strResponse.toUtf8(), "UTF-8");
}

void HttpParserGeneric::SendResponseA(QByteArray barLine,
                                      QByteArray barResponse,
                                      QByteArray barEncoding)
{
    QByteArray barHeaders;
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    barHeaders = barLine + "\r\n";
    barHeaders.append("Connection: ");
    barHeaders.append(tcpSocket->property("keep-alive").toBool()
                       ? "keep-alive" : "close");
    barHeaders.append("\r\n");
    if (tcpSocket->property("keep-alive").toBool()) {
        barHeaders.append("Keep-Alive: timeout="
                   + QByteArray::number(P_KEEPALIVE_TIMEOUT/1000) + ", max="
                   + QByteArray::number(P_KEEPALIVE_MAX) + "\r\n");
    }
    barHeaders.append(
        "Content-Type: text/html; charset="+barEncoding+"\r\n"
        "Content-Length: " + QByteArray::number(barResponse.length()) +
        "\r\nDate: "
        + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                            "ddd, dd MMM yyyy hh:mm:ss").toAscii()
        +" GMT\r\nServer: GFE/2.0\r\n\r\n");
    this->tcpSocket->write(barHeaders);
    this->tcpSocket->write(barResponse);
}

QTcpSocket * HttpParserGeneric::getTcpSocket()
{
    return this->tcpSocket;
}

QUrl HttpParserGeneric::getUrl() {
    return QUrl();
}


void HttpParserGeneric::Timeout()
{
    this->tcpSocket->setProperty("http-state", kStateMeta);
    this->tcpSocket->setProperty("keep-alive", false);
    tcpSocket->disconnectFromHost();
    emit TerminateParser();
}

void HttpParserGeneric::ResponseEnd() {}
