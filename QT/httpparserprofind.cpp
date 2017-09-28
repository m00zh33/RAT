#include "httpparserprofind.h"

HttpParserProfind::HttpParserProfind(QObject * parent, QTcpSocket * tcpSocket) :
    HttpParserGet(parent, tcpSocket)
{
}

void HttpParserProfind::GenerateResponse()
{
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    QByteArray headers = "HTTP/1.";
    tmrTimeout.stop();
    headers.append(this->tcpSocket->property("http-protocol").toUInt() == 11
                   ? "1" : "0");

    urlRequested = this->tcpSocket->property("url").toUrl();
    infoFile = QFileInfo(P_FOLDER_ROOT + urlRequested.host()
                         + urlRequested.path());
    QString test1 = QDir::cleanPath(P_FOLDER_ROOT);
    QString test2 = QDir::cleanPath(infoFile.absolutePath());
    if (test2.left(test1.length()) != test1) {
        return this->ErrorAccessDenied();
    }
    if (!infoFile.exists()) {
        headers.append(" 404 File Not Found");
    } else {
        headers.append(" 207 Multi-Status");
    }
    headers.append("\r\nDate: " + lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                   "ddd, dd MMM yyyy hh:mm:ss").toAscii());
    headers.append("  GMT\r\nServer: gws\r\nConnection: ");
    headers.append(tcpSocket->property("keep-alive").toBool()
                   ? "keep-alive" : "close");
    if (infoFile.exists()) {
        headers.append("\r\nContent-Location: ");
        headers.append(urlRequested.toEncoded());
        headers.append("\r\nContent-Type: text/xml"
                       "\r\nTransfer-Encoding: chunked");
    }
    headers.append("\r\n\r\n");
    this->tcpSocket->write(headers);
    headers.clear();

    if (infoFile.exists()) {
        QString strXml;
        strXml = "<?xml version=\"1.0\"?><a:multistatus xmlns:b=\""
                "urn:uuid:c2f41010-65b3-11d1-a29f-00aa00c14882/\" xmlns:c="
                "\"xml:\" xmlns:a=\"DAV:\"><a:response><a:href>";
        strXml.append(urlRequested.toEncoded());
        strXml.append("</a:href><a:propstat><a:status>HTTP/1.1 200 OK"
                      "</a:status><a:prop><a:getcontentlength b:dt=\"int\">");
        strXml.append(infoFile.isDir()
                      ? "0" :QString::number(infoFile.size()));
        strXml.append(
                "</a:getcontentlength><a:creationdate b:dt=\"dateTime.tz\">"
                + lcl.toString(infoFile.created().toTimeSpec(Qt::UTC),
                               "yyyy-MM-ddThh:mm:ss.zzzZ")
                //2008-11-18T01:09:55.490Z
                + "</a:creationdate><a:displayname>"
                + infoFile.fileName() + "</a:displayname>"
                //<a:getetag>"d97169f2d1d9cb1:8ed"</a:getetag>
                "<a:getlastmodified b:dt=\"dateTime.rfc1123\">"
                + lcl.toString(infoFile.lastModified().toTimeSpec(Qt::UTC),
                               "ddd, dd MMM yyyy hh:mm:ss")
                //Thu, 03 Mar 2011 18:36:44 GMT
                + " GMT</a:getlastmodified>\r\n");
        if (infoFile.isDir()) {
            strXml.append("<a:resourcetype><a:collection/>"
                "</a:resourcetype><a:supportedlock/><a:ishidden b:dt=\"boolean"
                "\">0</a:ishidden><a:iscollection b:dt=\"boolean\">1</a:"
                "iscollection><a:getcontenttype/></a:prop></a:propstat></a:"
                "response></a:multistatus>");
        } else {
            strXml.append(
                "<a:resourcetype/><a:supportedlock><a:lockentry><a:write/>"
                "<a:shared/></a:lockentry><a:lockentry><a:write/><a:exclusive/>"
                "</a:lockentry></a:supportedlock><a:ishidden b:dt=\"boolean\">"
                "0</a:ishidden><a:iscollection b:dt=\"boolean\">0"
                "</a:iscollection><a:getcontenttype/></a:prop></a:propstat>"
                "</a:response></a:multistatus>");
        }
        QByteArray barXml = strXml.toAscii();
        strXml.clear();
        this->tcpSocket->write(QByteArray::number(barXml.length(), 16)
                               + "\r\n" + barXml + "\r\n0\r\n\r\n");
    }

    ResponseEnd();
}
