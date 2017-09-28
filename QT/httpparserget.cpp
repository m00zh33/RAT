#include "httpparserget.h"

HttpParserGet::HttpParserGet( QObject * parent, QTcpSocket * tcpSocket ) :
    HttpParserGeneric(parent, tcpSocket)
{
    boolHeadersOnly = false;
}

void HttpParserGet::GenerateResponse()
{
    this->ParseHeaders();
    this->urlRequested = this->tcpSocket->property("url").toUrl();

    if (!this->ParseDynamicContent()) {
        this->GenerateStaticFile();
    }
}

bool HttpParserGet::ParseDynamicContent()
{
    this->fileRequested.setFileName(P_FOLDER_ROOT + urlRequested.host()
                                    + urlRequested.path());
    if (urlRequested.path().left(9) == "/cgi-bin/") {
            QString script = urlRequested.path().mid(9);
        if (!fileRequested.exists()) {
            this->ErrorNotFound();
            return true;
        }
        if (script == "clock") {
            new HttpHandlerClock(this);
        } else if (script == "ip") {
            new HttpHandlerIp(this);
        } else if (script == "p") {
            new HttpHandlerP(this);
        } else if (script == "session") {
            new HttpHandlerSession(this);
        } else if (script == "list") {
            new HttpHandlerSecureList(this);
        } else if (script == "cmd") {
            new HttpHandlerSecureCmd(this);
        } else if (script == "dir") {
            new HttpHandlerSecureDir(this);
        } else if (script == "noop") {
            new HttpHandlerNoop(this);
        } else if (script == "report") {
            new HttpHandlerReport(this);
        } else if (script == "otpgen") {
            new HttpHandlerOtpgen(this);
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

void HttpParserGet::GenerateStaticFile()
{
    QFileInfo infoFile(this->fileRequested);
    QString test1 = QDir::cleanPath(P_FOLDER_ROOT);
    QString test2 = QDir::cleanPath(infoFile.absolutePath());
    if (test2.left(test1.length()) != test1) {
        return this->ErrorAccessDenied();
    }
    if (infoFile.exists()) {
        if (infoFile.isFile() && fileRequested.open(QIODevice::ReadOnly)) {
            this->SendFile();
        } else if (infoFile.isDir()) {
            this->fileRequested.setFileName(P_FOLDER_ROOT
                                            + urlRequested.host()
                                            + urlRequested.path()
                                            + "/default.html");
            infoFile.setFile(fileRequested);
            if (infoFile.exists() && infoFile.isFile()) {
                if (fileRequested.open(QIODevice::ReadOnly)) {
                    this->SendFile();
                } else {
                    this->ErrorAccessDenied();
                }
            } else {
                this->ErrorNotFound();
            }
        } else {
            this->ErrorNotFound();
        }
    } else {
        this->ErrorNotFound();
    }
}

void HttpParserGet::ParseHeaders()
{
    QByteArray barHeaders = this->tcpSocket->property("http-head")
                            .toByteArray().replace("\r\n","\n");
    QStringList strLines = QString(barHeaders).split('\n');
    QString strLine;
    qint32 intPos;
    foreach (strLine, strLines) {
        if (strLine.isEmpty()) continue;
        intPos = strLine.indexOf(": ");
        if (intPos == -1) continue;
        this->mapHeaders.insert(strLine.left(intPos).toLower(),
                                strLine.mid(intPos + 2));
    }
}

void HttpParserGet::ErrorCommon(quint16 intErrCode, QByteArray barErrText)
{
    static QByteArray barErrCode = QByteArray::number(intErrCode);
    QByteArray barLine = "HTTP/1.";
    if (this->tcpSocket->property("http-protocol").toUInt() == 11)
        barLine.append("1");
    else
        barLine.append("0");
    barLine.append(" " + barErrCode + " " + barErrText);
    QFile fileError;
    fileError.setFileName("htstatic/"+barErrCode+".html");
    if (fileError.exists()
      && fileError.open(QIODevice::ReadOnly | QIODevice::Text)) {
        this->SendResponse(barLine, fileError.readAll());
        fileError.close();
    } else {
        this->SendResponse(barLine, barLine);
    }
    this->ResponseEnd();
}

void HttpParserGet::ErrorNotFound()
{
    this->ErrorCommon(404, "File Not Found");
}

void HttpParserGet::ErrorAccessDenied() {
    this->ErrorCommon(403, "Forbidden");
}

void HttpParserGet::ErrorBadParameters() {
    this->ErrorCommon(500, "Internals Server Error");
}

void HttpParserGet::ErrorInvalidRange() {
    this->ErrorCommon(416, "Requested Range Not Satisfiable");
}

void HttpParserGet::FoundElsewhere(QString strLocation) {
    this->FoundElsewhereCommon(false, strLocation);
}

void HttpParserGet::FoundElsewherePermenant(QString strLocation) {
    this->FoundElsewhereCommon(true, strLocation);
}

void HttpParserGet::FoundElsewhereCommon(bool boolPermanent,
                                         QString strLocation) {
    static QByteArray barErrCode =
            (boolPermanent ? "301 Moved Permanently" : "302 Found");
    QByteArray barLine = "HTTP/1.";
    if (this->tcpSocket->property("http-protocol").toUInt() == 11)
        barLine.append("1");
    else
        barLine.append("0");
    barLine.append(" " + barErrCode);
    barLine.append("\r\nLocation: " + QUrl::toPercentEncoding(strLocation));
    QFile fileError;
    fileError.setFileName("htstatic/"+barErrCode.left(3)+".html");
    if (fileError.exists()
      && fileError.open(QIODevice::ReadOnly | QIODevice::Text)) {
        this->SendResponse(barLine, fileError.readAll());
        fileError.close();
    } else {
        this->SendResponse(barLine, barLine);
    }
    this->ResponseEnd();
}

void HttpParserGet::ResponseEnd()
{
    if (tcpSocket->property("keep-alive").toBool()) {
        tcpSocket->setProperty("http-state", kStateTransitional);
    } else {
        tcpSocket->setProperty("http-state", kStateMeta);
        tcpSocket->disconnectFromHost();
    }
    emit TerminateParser();
}

void HttpParserGet::SendFile()
{
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QFileInfo infoFile(this->fileRequested);
    QByteArray headers = "HTTP/1.";
    QLocale lcl;
    intBytesRemaining = 0;
    infoFile.setCaching(true);
    tmrTimeout.stop();

    if (this->tcpSocket->property("http-protocol").toUInt() == 11)
        headers.append("1");
    else
        headers.append("0");
    if (this->mapHeaders.contains("range")) {
        QRegExp rxRange("^bytes=(\\d+)-(\\d*)/?(\\d*)$");
        if (-1 != rxRange.indexIn(this->mapHeaders.value("range"))) {
            quint64 intStart = 0, intEnd = 0, intSize = 0;
            intBytesRemaining = 1;
            if (!rxRange.cap(3).isEmpty())
                intSize = rxRange.cap(3).toUInt();
            if (!rxRange.cap(2).isEmpty())
                intEnd = rxRange.cap(2).toUInt();
            if (!rxRange.cap(1).isEmpty())
                intStart = rxRange.cap(1).toUInt();
            else
                intBytesRemaining = 0;
            if (!intBytesRemaining) {
            } else if (intSize && intSize != (quint64)infoFile.size()) {
                intBytesRemaining = 0;
            } else if (intEnd && intEnd >= (quint64)infoFile.size()) {
                intBytesRemaining = 0;
            } else if (intStart >= (quint64)infoFile.size()) {
                intBytesRemaining = 0;
            } else if (intEnd && intStart >= intEnd) {
                intBytesRemaining = 0;
            }
            if (intBytesRemaining) {
                if (intEnd > 0) {
                    intBytesRemaining = intEnd + 1;
                } else {
                    intBytesRemaining = infoFile.size();
                }
                intBytesRemaining -= intStart;
                if (!intEnd) intEnd = infoFile.size() - 1;
                if (!intSize) intSize = infoFile.size();
                if (fileRequested.seek(intStart)) {
                    headers.append(" 206 Partial Content");
                    headers.append("\r\nContent-Range: bytes "
                        + QByteArray::number(intStart) + "-"
                        + QByteArray::number(intEnd) + "/"
                        + QByteArray::number(intSize));
                } else {
                    intBytesRemaining = 0;
                }
            }
            if (!intBytesRemaining) {
                this->fileRequested.close();
                return this->ErrorInvalidRange();
            }
        }
    }
    if (intBytesRemaining == 0) {
        intBytesRemaining = infoFile.size();
        headers.append(" 200 OK");
    }
    headers.append("\r\nLast-Modified: ");
    headers.append(lcl.toString(infoFile.lastModified().toTimeSpec(Qt::UTC),
                   "ddd, dd MMM yyyy hh:mm:ss").toAscii());
    headers.append(" GMT");
    if (infoFile.suffix().isEmpty()
      && urlRequested.hasQueryItem("x-disposition")) {
        headers.append("\r\nContent-Disposition: attachment; filename="
                       + urlRequested.queryItemValue("x-disposition"));
    } else {
        headers.append("\r\nContent-Type: ");
        if (infoFile.suffix() == "html" || infoFile.suffix() == "htm"
          || infoFile.suffix().isEmpty()) {
            headers.append("text/html");
        } else if (infoFile.suffix() == "jpg"
                   || infoFile.suffix() == "jpeg") {
            headers.append("image/jpeg");
        } else if (infoFile.suffix() == "png") {
            headers.append("image/png");
        } else if (infoFile.suffix() == "txt") {
            headers.append("text/plain");
        } else if (infoFile.suffix() == "js") {
            headers.append("application/x-javascript");
        } else if (infoFile.suffix() == "css") {
            headers.append("text/css");
        } else if (infoFile.suffix() == "zip") {
            headers.append("application/zip");
        } else {
            headers.append("application/octet-stream");
        }
    }
    headers.append("\r\nDate: " + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                   "ddd, dd MMM yyyy hh:mm:ss").toAscii());
    headers.append(" GMT\r\nServer: gws\r\n");
    headers.append("X-XSS-Protection: 1; mode=block\r\n");
    if (tcpSocket->property("keep-alive").toBool()) {
        headers.append("Keep-Alive: timeout="
                   + QByteArray::number(P_KEEPALIVE_TIMEOUT/1000) + ", max="
                   + QByteArray::number(P_KEEPALIVE_MAX) + "\r\n");
    }
    headers.append("Connection: ");
    headers.append(tcpSocket->property("keep-alive").toBool() ? "keep-alive"
        : "close");
    headers.append("\r\n");
    headers.append("Content-Length: "+QByteArray::number(intBytesRemaining));
    headers.append("\r\nAccept-Ranges: bytes\r\n\r\n");
    this->tcpSocket->write(headers);
    headers.clear();
    if (boolHeadersOnly) {
        this->fileRequested.close();
        return this->ResponseEnd();
    }
    tmrSendFilePart.setInterval(1000);
    connect(&tmrSendFilePart, SIGNAL(timeout()), this, SLOT(SendFilePart()));
    tmrSendFilePart.start();
    this->SendFilePart();
}

void HttpParserGet::SendFilePart()
{

    if (intBytesRemaining < P_TRANSFER_RATE) {
        tcpSocket->write(fileRequested.read(intBytesRemaining));
        intBytesRemaining = 0;
        fileRequested.close();
        ResponseEnd();
    } else {
        tcpSocket->write(fileRequested.read(P_TRANSFER_RATE));
        if (tcpSocket->bytesToWrite() && !tcpSocket->flush()) {
            tmrSendFilePart.setInterval(tmrSendFilePart.interval() + 1000);
        }
        intBytesRemaining -= P_TRANSFER_RATE;
        if (!fileRequested.bytesAvailable()
          || tmrSendFilePart.interval() > 30000) {
            tcpSocket->disconnectFromHost();
            tmrSendFilePart.stop();
        }
    }
}

QUrl HttpParserGet::getUrl()
{
    return this->urlRequested;
}
