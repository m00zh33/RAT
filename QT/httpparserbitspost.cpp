#include "httpparserbitspost.h"

HttpParserBitsPost::HttpParserBitsPost(QObject *parent, QTcpSocket *tcpSocket)
    : HttpParserPost(parent, tcpSocket)
{
    this->setProperty("HttpParserBitsPost",true);
}

void HttpParserBitsPost::GenerateResponse()
{
    bool boolInvalid = false;

    this->ParseHeaders();

    this->urlRequested = this->tcpSocket->property("url").toUrl();

    if (!HttpBitsGeneric::isUrlValid(this->urlRequested, this)) {
        return this->ErrorNotFound();
    }

    if (!boolInvalid && mapHeaders.value("bits-packet-type", "") != "") {
        QString strBitsPacketType = mapHeaders.value("bits-packet-type", "");
        if (strBitsPacketType == "Ping") {
            this->HandlePing();
        } else if (strBitsPacketType == "Create-Session") {
            this->HandleCreateSession();
        } else if (strBitsPacketType == "Fragment") {
            this->HandleFragment();
        } else if (strBitsPacketType == "Close-Session") {
           this->HandleCloseSession();
        } else if (strBitsPacketType == "Cancel-Session") {
            this->HandleCancelSession();
        } else {
            boolInvalid = true;
        }
    }

    if (boolInvalid) {
        tcpSocket->setProperty("keep-alive", false);
        this->ErrorBadParameters();
    }
}

void HttpParserBitsPost::HandlePing()
{
    QByteArray barResponse = this->GenerateCommonHeaders(200, "OK", "Ack", 0);
    barResponse.append("\r\n");
    this->tcpSocket->write(barResponse);
    this->ResponseEnd();
}

void HttpParserBitsPost::HandleCreateSession()
{
    QByteArray barResponse;
    QStringList lstProtocols =
            mapHeaders.value("bits-supported-protocols").split(' ');
    if (!lstProtocols.contains("{7df0354d-249b-430f-820d-3d2a9bef4931}")) {
        //BITS 1.5 Upload Protocol
        barResponse = this->GenerateCommonHeaders(500, "Internal Server Error",
                                                  "Ack", 0);
        barResponse.append("BITS-Error-Code: 0x80200005\r\n\r\n");
        //BG_E_PROTOCOL_NOT_AVAILABLE
        tcpSocket->write(barResponse);
        return this->ResponseEnd();
    }

    QFileInfo infoSession;
    do {
        guidSession = QUuid::createUuid();
        infoSession.setFile(P_FOLDER_BITS_SESSIONS + guidSession.toString());
    } while (infoSession.exists());

    QDir dirSessions(P_FOLDER_BITS_SESSIONS);
    if (!dirSessions.mkdir(guidSession.toString())) {
        //shit
        barResponse = this->GenerateCommonHeaders
                      (503, "Service Unavailable", "Ack", 0);
        barResponse.append("\r\n");
        this->tcpSocket->write(barResponse);
        return this->ResponseEnd();

    }
    dirSessions.cd(guidSession.toString());
    QFile fileSettings(dirSessions.filePath("settings"));
    fileSettings.open(QIODevice::WriteOnly | QIODevice::Text);
    fileSettings.write(tcpSocket->peerAddress().toString().toUtf8() + "\n");
    fileSettings.write(urlRequested.path().toUtf8() + "\n");
    fileSettings.close();
    barResponse = this->GenerateCommonHeaders(200, "OK", "Ack", 0);
    barResponse.append(
            "BITS-Protocol: {7df0354d-249b-430f-820d-3d2a9bef4931}\r\n"
            "BITS-Session-Id: "+guidSession.toString()+"\r\n"
            "Accept-Encoding: Identity\r\n"
            "\r\n"
                       );
    tcpSocket->write(barResponse);
    this->ResponseEnd();
}

void HttpParserBitsPost::HandleFragment()
{
    guidSession = QUuid(mapHeaders.value("bits-session-id","x"));
    if (guidSession.isNull()) {
        return this->ErrorBadParameters();
    }

    intBytesRemaining = mapHeaders.value("content-length", 0).toUInt();
    if (!intBytesRemaining || mapHeaders.value("content-range","") == "") {
        return this->ErrorBadParameters();
    }
    quint32 intStart, intEnd, intTotal;
    QRegExp rxRange("^bytes (\\d+)-(\\d+)/(\\d+)$");
    if (rxRange.indexIn(mapHeaders.value("content-range","")) == -1) {
        return this->ErrorBadParameters();
    }
    intStart = rxRange.cap(1).toUInt();
    intEnd   = rxRange.cap(2).toUInt();
    intTotal = rxRange.cap(3).toUInt();

    QFile fileSettings(P_FOLDER_BITS_SESSIONS + guidSession.toString()
                       + "/settings");
    if (!fileSettings.exists()) {
        //invalid session id
        return this->ErrorNotFound();
    }
    QFileInfo infoLock(P_FOLDER_BITS_SESSIONS + guidSession.toString()
                       + "/lock");
    if (infoLock.exists()) {
        return this->ErrorAccessDenied();
    }

    fileSettings.open(QIODevice::ReadWrite | QIODevice::Text);
    if (fileSettings.readLine() != tcpSocket->peerAddress().toString() + "\n"
        || fileSettings.readLine() != urlRequested.path().toUtf8() + "\n") {
        return this->ErrorAccessDenied();
    }
    if (fileSettings.atEnd()) {
        fileSettings.write(QByteArray::number(intTotal)+"\n");
        fileSettings.write(mapHeaders.value("content-name","").toUtf8()+"\n");
    } else if (fileSettings.readLine().trimmed().toUInt() != intTotal) {
        return this->ErrorInvalidRange();
    }
    fileSettings.close();

    intBytesRemaining = intEnd + 1 - intStart;

    fileRequested.setFileName(P_FOLDER_BITS_SESSIONS + guidSession.toString()
                              + "/content");
    fileRequested.open(QIODevice::ReadWrite);
    if (fileRequested.size() <= intEnd) {
        fileRequested.resize(intEnd+1);
    }

    fileRequested.seek(intStart);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(ReadPost()));
    if (tcpSocket->bytesAvailable()) {
        QTimer::singleShot(0, this, SLOT(ReadPost()));
    }

}

void HttpParserBitsPost::ReadPost()
{
    quint64 intBytesAvaliable = tcpSocket->bytesAvailable();
    if (!intBytesAvaliable) return;
    tmrTimeout.stop();
    if (intBytesAvaliable >= intBytesRemaining) {
        fileRequested.write(tcpSocket->read(intBytesRemaining));
        intBytesRemaining = 0;
        disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(ReadPost()));
        QByteArray barResponse = this->GenerateCommonHeaders(
                200, "OK", "Ack", 0);
        barResponse.append(
                "BITS-Session-Id: " + guidSession.toString() + "\r\n"
                "BITS-Received-Content-Range: "
                + QByteArray::number(fileRequested.pos()) + "\r\n"
                "\r\n");
        fileRequested.close();
        tcpSocket->write(barResponse);
        this->ResponseEnd();
    } else {
        intBytesRemaining -= intBytesAvaliable;
        fileRequested.write(tcpSocket->readAll());
        tmrTimeout.start();
    }
}

void HttpParserBitsPost::HandleCloseSession()
{
    if (this->HandleEndSession(false)) {
        emit SessionClosed(this->guidSession,
                           QFileInfo(this->fileRequested));
       //^<-- is this emited too late?
    }
}

void HttpParserBitsPost::HandleCancelSession()
{
    this->HandleEndSession(true);
}

bool HttpParserBitsPost::HandleEndSession(bool boolIgnoreSize)
{
    guidSession = QUuid(mapHeaders.value("bits-session-id","x"));
    if (guidSession.isNull()) {
        this->ErrorBadParameters();
        return false;
    }
    QFile fileSettings(P_FOLDER_BITS_SESSIONS + guidSession.toString()
                       + "/settings");
    if (!fileSettings.exists()) {
        //invalid session id
        this->ErrorNotFound();
        return false;
    }

    QFile fileLock(P_FOLDER_BITS_SESSIONS + guidSession.toString() + "/lock");
    if (fileLock.exists()) {
        this->ErrorAccessDenied();
        return false;
    }

    fileRequested.setFileName(P_FOLDER_BITS_SESSIONS + guidSession.toString()
                              + "/content");
    fileSettings.open(QIODevice::ReadOnly | QIODevice::Text);
    if (fileSettings.readLine() != tcpSocket->peerAddress().toString() + "\n"
      || fileSettings.readLine() != urlRequested.path().toUtf8() + "\n") {
        this->ErrorAccessDenied();
        return false;
    }
    if (fileSettings.readLine().trimmed().toUInt() != fileRequested.size()
      && !boolIgnoreSize) {
        //TODO: i don't know if i should really care about this
    }
//    QString strFilename = QString::fromUtf8(fileSettings.readLine().chop(1));
    fileSettings.close();

    fileLock.open(QIODevice::WriteOnly);
    fileLock.write(" ");
    fileLock.close();

    QByteArray barResponse = this->GenerateCommonHeaders(200, "OK", "Ack", 0);
    barResponse.append("BITS-Session-Id: " + guidSession.toString() + "\r\n"
                       "\r\n");
    tcpSocket->write(barResponse);
    this->ResponseEnd();

    return true;
}

void HttpParserBitsPost::SendResponseA(QByteArray barLine,
                                      QByteArray barResponse,
                                      QByteArray barEncoding)
{
    QByteArray barHeaders;
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    barHeaders = barLine + "\r\n";
    if (this->tcpSocket->property("keep-alive").toBool())
        barHeaders.append("Connection: keep-alive\r\n");
    barHeaders.append(
        "BITS-Error-Code: 0x80190190\r\n" //BG_E_HTTP_ERROR_400
        "BITS-Error-Context: 0x5\r\n"
        "Content-Type: text/html; charset="+barEncoding+"\r\n"
        "Content-Length: " + QByteArray::number(barResponse.length()) +
        "\r\nDate: "
        + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                            "ddd, dd MMM yyyy hh:mm:ss").toAscii()
        +" GMT\r\nServer: GFE/2.0\r\n\r\n");
    this->tcpSocket->write(barHeaders);
    this->tcpSocket->write(barResponse);
}

QByteArray HttpParserBitsPost::GenerateCommonHeaders(quint16 intHttpCode,
                                                    QByteArray barHttpDesc,
                                                    QByteArray barBitsType,
                                                    quint64 intContentLength)
{
    QByteArray barHeaders = "HTTP/1.";
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    if (this->tcpSocket->property("http-protocol").toUInt() == 11)
        barHeaders.append("1");
    else
        barHeaders.append("0");
    barHeaders.append(" " + QByteArray::number(intHttpCode) + " " +barHttpDesc
                      + "\r\n"
                      "BITS-Packet-Type: " + barBitsType + "\r\n"
                      "Content-Length: " + QByteArray::number(intContentLength)
                      + "\r\n"
                      "Date: " + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                       "ddd, dd MMM yyyy hh:mm:ss").toAscii()
                      + " GMT\r\n"
                      "Server: GFE/2.0\r\n");
    if (intHttpCode != 200 && intHttpCode != 201) {
        switch (intHttpCode) {
            case 400:
                barHeaders.append("BITS-Error-Code: 0x80190190\r\n");
//              BG_E_HTTP_ERROR_400 (0x80190190)
//              The server could not process the transfer request because the
//                syntax of the remote file name is invalid.
                break;
            case 401:
                barHeaders.append("BITS-Error-Code: 0x80190191\r\n");
//              BG_E_HTTP_ERROR_401 (0x80190191)
//              The user does not have permission to access the remote file.
//                The requested resource requires user authentication.
                break;
            case 404:
                barHeaders.append("BITS-Error-Code: 0x80190194\r\n");
//              BG_E_HTTP_ERROR_404 (0x80190194)
//              The requested URL does not exist on the server.
//              In IIS 7, this error can indicate That BITS uploads are not
//                enabled on the virtual directory (vdir) on the server.
//              That the upload size exceeds the maximum upload limit (for
//                details, see the BITSMaximumUploadSize IIS extension
//                property).
                break;
            case 407:
                barHeaders.append("BITS-Error-Code: 0x80190197\r\n");
//              BG_E_HTTP_ERROR_407 (0x80190197)
//              The user does not have permission to access the proxy.
//              The proxy requires user authentication.
                break;
            case 414:
                barHeaders.append("BITS-Error-Code: 0x8019019E\r\n");
//              BG_E_HTTP_ERROR_414 (0x8019019E)
//              The server cannot process the transfer request. The Uniform
//                Resource Identifier (URI) in the remote file name is longer
//                than the server can interpret.
                break;
            case 501:
                barHeaders.append("BITS-Error-Code: 0x801901F5\r\n");
//              BG_E_HTTP_ERROR_501 (0x801901F5)
//              The server does not support the functionality required to
//                fulfill the request. In IIS 6, this error indicates that BITS
//                uploads are not enabled on the virtual directory (vdir) on
//                the server.
                break;
            case 503:
                barHeaders.append("BITS-Error-Code: 0x801901F7\r\n");
//              BG_E_HTTP_ERROR_503 (0x801901F7)
//              The service is temporarily overloaded and cannot process the
//                request. Resume the job at a later time.
                break;
            case 504:
                barHeaders.append("BITS-Error-Code: 0x801901F8\r\n");
//              BG_E_HTTP_ERROR_504 (0x801901F8)
//              The transfer request timed out while waiting for a gateway.
//                Resume the job at a later time.
                break;
            case 505:
                barHeaders.append("BITS-Error-Code: 0x801901F9\r\n");
//              BG_E_HTTP_ERROR_505 (0x801901F9)
//              The server does not support the HTTP protocol version specified
//                in the remote file name.
                break;
        }
        barHeaders.append("BITS-Error-Context: 0x5\r\n");
            //BG_ERROR_CONTEXT_REMOTE_FILE
    }
    barHeaders.append("Connection: ");
    barHeaders.append(tcpSocket->property("keep-alive").toBool()
                       ? "keep-alive" : "close");
    barHeaders.append("\r\n");
    if (tcpSocket->property("keep-alive").toBool()) {
        barHeaders.append("Keep-Alive: timeout="
                   + QByteArray::number(P_KEEPALIVE_TIMEOUT/1000) + ", max="
                   + QByteArray::number(P_KEEPALIVE_MAX) + "\r\n");
    }

    return barHeaders;

}
