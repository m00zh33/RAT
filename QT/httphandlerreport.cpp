#include "httphandlerreport.h"

HttpHandlerReport::HttpHandlerReport(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    QSqlQuery sql(db);
    QUrl url = parser->getUrl();

    if (!url.hasQueryItem("id") || url.queryItemValue("id").length() != 38
      || !url.hasQueryItem("cmd") || !url.hasQueryItem("entry") ) {
        this->error500();
        return;
    }

    mapHeaders.insert("Content-Type","text/plain");

    this->generateSqlSchema();

    sql.prepare("SELECT 1 FROM sessions WHERE code LIKE ?");
    sql.addBindValue(url.queryItemValue("id").replace('%',"")
                     .replace('_',""));

    if (sql.exec() && sql.next()) {
        if (!parser->boolHeadersOnly) {
            sql.clear();
            sql.prepare("INSERT INTO reports VALUES (?, DATETIME(), ?, ?, 0)");
            sql.addBindValue(url.queryItemValue("id"));
            sql.addBindValue(url.queryItemValue("cmd"));
            sql.addBindValue(url.queryItemValue("entry"));
            sql.exec();
        }
    }

    this->writeHeader(1);
    this->writeContent(" ");
    emit HandlerEnd();
}

void HttpHandlerReport::generateSqlSchema()
{
    if (!db.tables().contains("reports")) {
        QSqlQuery query("CREATE TABLE reports "
                        "(session, time, cmd, entry, reviewed)", db);
        query.exec();
    }
}
