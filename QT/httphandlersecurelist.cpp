#include "httphandlersecurelist.h"

HttpHandlerSecureList::HttpHandlerSecureList(QObject *parent) :
        HttpHandlerSecure(parent) {}

void HttpHandlerSecureList::secure()
{
    if (!this->setupSql()) {
        Q_ASSERT(db.isOpen() && !db.isOpenError());
        return this->error500();
    }

    mapHeaders.insert("Content-Type", "text/html; charset=utf-8");
    url = this->parser->getUrl();
    intLimit = 100;
    if (url.hasQueryItem("limit")) {
        intLimit = url.queryItemValue("limit").toInt();
        if (!intLimit || intLimit > 1000) intLimit = 100;
    }

    if (!url.hasQueryItem("output")) {
        renderHtmlList();
    } else {
        renderAjaxList();
    }

}

void HttpHandlerSecureList::renderAjaxList()
{
    QSqlQuery sqlList(db);
    QByteArray barResponse;
    QString strEncode;
    QDateTime date, now = QDateTime::currentDateTime();
    quint64 intSecs;

    sqlList.prepare("SELECT name, serial, code, access FROM "
                    "sessions ORDER BY access DESC LIMIT "
                    + QString::number(intLimit));
    sqlList.setForwardOnly(true);
    sqlList.exec();

    barResponse.append("<ul>\n");
    while (sqlList.next()) {
        date = sqlList.value(3).toDateTime();
        date.setTimeSpec(Qt::UTC);
        barResponse.append("<li> ");
        strEncode = sqlList.value(0).toString() + " "
                    + sqlList.value(1).toString() + " ";
        intSecs = date.secsTo(now);
        if (intSecs < 60) {
            strEncode.append("(just now)");
        } else if (intSecs < 3600) {
            strEncode.append(QString("(%1 minutes ago)")
                               .arg(QString::number(intSecs/60)));
        } else if (intSecs < 86400) {
            strEncode.append(QString("(%1 hours ago)")
                               .arg(QString::number(intSecs/3600)));
        } else {
            intSecs = date.daysTo(now);
            strEncode.append(QString("(%1 days ago)")
                               .arg(QString::number(intSecs)));
        }
        strEncode.replace('<',"&lt;").replace('>',"&gt;")
                .replace('\'',"&#039;").replace('"',"&quot;")
                .replace('&',"&amp;");
        barResponse.append(strEncode.toUtf8());
        barResponse.append(" <a href=\"cmd?i=");
        barResponse.append(sqlList.value(2).toByteArray());
        barResponse.append("\">control</a> </li>\n");
    }
    barResponse.append("</ul>\n");
    this->writeHeader(barResponse.length(), "200 OK");
    this->writeContent(barResponse);
    emit HandlerEnd();
}

void HttpHandlerSecureList::renderHtmlList()
{
    QFile file("htstatic/list.html");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return this->error500();
    }
    this->writeHeader(file.size());
    this->writeContent(file.readAll());
    file.close();
    emit HandlerEnd();
}
