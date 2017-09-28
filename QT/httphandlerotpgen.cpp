#include "httphandlerotpgen.h"

HttpHandlerOtpgen::HttpHandlerOtpgen(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    QSqlQuery sql(db);
    QUrl url = parser->getUrl();
    QStringList creds;
    QByteArray barResponse;
    QString strCode;
    quint16 i;

    if (!url.hasQueryItem("cred") || url.queryItemValue("cred").length() != 38
      || parser->boolHeadersOnly) {
        this->error500();
        return;
    }

    mapHeaders.insert("Content-Type","text/html");

    creds.append("{hi8aof6o-noc1-x2k7-5c9x-zx8f80t4jtld}"); // my
    creds.append("{6jsi84qp-z1vi-xt14-nl6q-cdhbq3w7qrmt}"); // psdr

    if (creds.contains(url.queryItemValue("cred"))) {
        sql.prepare("INSERT INTO otp VALUES (?, DATETIME())");
        for (i = 0; i < 6; i++) {
            strCode.append(this->randomChar());
        }
        sql.addBindValue(strCode);
        sql.exec();
        barResponse = "<h1>" + strCode.toAscii() + "        </h1>";

    } else {
        barResponse = "<h1>Access Denied.</h1>";
    }
    this->writeHeader(barResponse.length());
    this->writeContent(barResponse);
    emit HandlerEnd();
}

void HttpHandlerOtpgen::generateSqlSchema()
{
    QSqlQuery query(db);

    if (!db.tables().contains("otp")) {
        query.prepare("CREATE TABLE otp (code, created)");
        query.exec();
    }
}

QChar HttpHandlerOtpgen::randomChar()
{
    unsigned int ret = qrand() % 36;
    if (ret < 26) ret += 'a';
    else ret += '0'-26;
    return QChar (ret);
}
