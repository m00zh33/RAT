#include "httphandlersecure.h"

HttpHandlerSecure::HttpHandlerSecure(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    Q_ASSERT(db.isOpen());
    if (isIdentified()) {
        QTimer::singleShot(0, this, SLOT(secure()));
    } else {
        this->renderLoginScreen();
    }
}

bool HttpHandlerSecure::isIdentified()
{
    QString strHead = QString(parser->getTcpSocket()
                         ->property("http-head").toByteArray());
    QMap<QString, QString> mapCookies;
    QRegExp rxCookie("\\nCookie: ([^=]+)=([^; ]+)(?:;\\s*([^=]+)=([^;"
                   "\\r\\n]+))*", Qt::CaseInsensitive, QRegExp::RegExp2);

    int pos = 0, pair = 1;
    while ( (pos = rxCookie.indexIn(strHead, pos)) && pos != -1 ) {
        while (!rxCookie.cap(pair).isEmpty()) {
            mapCookies.insert(rxCookie.cap(pair).toLower(),
                              rxCookie.cap(pair+1).trimmed());
            pair += 2;
        }
        pos += rxCookie.matchedLength();
    }
    strHead.clear();
    //ok, got cookie, or not
    if (mapCookies.value("id","").length() != 38) {
        return false;
    }

    this->generateSqlSchemaIdentification();

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare("UPDATE identified SET access = DATETIME() WHERE "
                     "cookie = ?");
    sqlQuery.addBindValue(mapCookies.value("id"));
    if (!sqlQuery.exec()) return false;
    return (sqlQuery.numRowsAffected() > 0);
}

void HttpHandlerSecure::renderLoginScreen()
{
    QUrl url = parser->getUrl();

    this->generateSqlSchemaIdentification();
    this->parsePostToUrl(&url);

    mapHeaders.insert("Content-Type", "text/html; charset=utf-8");

    if (url.hasQueryItem("otp")) {
        QSqlQuery sqlOtp(db);
        sqlOtp.prepare("DELETE FROM otp WHERE code = ?");
        sqlOtp.addBindValue(url.queryItemValue("otp"));
        if (sqlOtp.exec() && sqlOtp.numRowsAffected() > 0) {
            QString strGuid = QUuid::createUuid().toString();
            sqlOtp.clear();
            sqlOtp.prepare("INSERT INTO identified (cookie, access) VALUES "
                           "(?, DATETIME())");
            sqlOtp.addBindValue(strGuid);
            sqlOtp.exec();
            url.removeQueryItem("otp");
#ifdef _STUNNEL_HACK
            url.setScheme("https");
#endif
            QByteArray barResponse =
                "<html><head><title>Please wait...</title><meta http-equiv="
                "\"Refresh\" content=\"0; url=" + url.toEncoded() + "\" />"
                "</head><body>Please follow this <a href=\""
                + url.toEncoded() + "\">link</a></body></html>";
            mapHeaders.insert("Set-Cookie",
                              QString("id=%1; path=/cgi-bin/; domain=%2")
                              .arg(strGuid).arg(url.host()).toAscii());
            this->writeHeader(barResponse.size());
            this->writeContent(barResponse);
            emit HandlerEnd();
            return;
        }
    }
    mapHeaders.insert("Set-Cookie",
                      QString("id=; domain=%1; expires="
                              "Thu, 01-Jan-70 00:00:01 GMT")
                      .arg(url.host()).toAscii());
    QFile fileLogin("htstatic/login.html");
    if (!fileLogin.exists() || !fileLogin.open(QIODevice::ReadOnly)) {
        fileLogin.setFileName("htstatic/404.html");
        if (!fileLogin.exists() || !fileLogin.open(QIODevice::ReadOnly)) {
            return this->error500();
        }
        this->writeHeader(fileLogin.size(), "404 Not Found");
    } else {

        this->writeHeader(fileLogin.size());
    }
    this->writeContent(fileLogin.readAll());
    fileLogin.close();
    emit HandlerEnd();
}

void HttpHandlerSecure::parsePostToUrl(QUrl *url)
{
    if (!parser->property("HttpParserPost").toBool()) return;
    QString strUrl = url->toString();
    strUrl.append((url->hasQuery() ? "&" : "?"));
    strUrl.append( static_cast<HttpParserPost *>(parser)->barPostData );
    url->setUrl(strUrl);
}

void HttpHandlerSecure::secure() {
    this->error500();
}

void HttpHandlerSecure::generateSqlSchemaIdentification()
{
    Q_ASSERT(db.isOpen());
    QSqlQuery query(db);
    if (!db.tables().contains("identified")) {
        query.prepare("CREATE TABLE identified (cookie, access)");
        query.exec();
    }
    if (!db.tables().contains("otp")) {
        query.clear();
        query.prepare("CREATE TABLE otp (code, created)");
        query.exec();
    }
}
