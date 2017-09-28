#include "httphandlersession.h"

HttpHandlerSession::HttpHandlerSession(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    QByteArray barContent;
    QUrl url = parser->getUrl();

    if (!url.hasQueryItem("name") || !url.hasQueryItem("serial")) {
        this->error500();
        return;
    }
    if (!db.tables().contains("sessions"))
        this->generateSqlSchema();
    QSqlQuery query(db);
    query.prepare("SELECT code FROM sessions WHERE name = ? AND serial = ?");
    query.addBindValue(url.queryItemValue("name"));
    query.addBindValue(url.queryItemValue("serial"));
    query.setForwardOnly(true);
    if (!query.exec()) {
        this->error500();
        return;
    }
    if (!query.next()) {
        barContent = QUuid::createUuid().toString().toAscii();
        query.clear();
        query.prepare("INSERT INTO sessions (name, serial, code, access) "
                      "VALUES (?, ?, ?, DATETIME())");
        query.addBindValue(url.queryItemValue("name"));
        query.addBindValue(url.queryItemValue("serial"));
        query.addBindValue(barContent);
        query.exec();
        query.prepare("INSERT INTO ops VALUES(?, DATETIME(), \"DIR\", "
                      "\"\")");
        query.addBindValue(barContent);
        query.exec();

    } else {
        barContent = query.value(0).toByteArray();
        query.clear();
        query.prepare("UPDATE sessions SET access = DATETIME() WHERE "
                      "name = ? AND serial = ?");
        query.addBindValue(url.queryItemValue("name"));
        query.addBindValue(url.queryItemValue("serial"));
        query.exec();
    }

    mapHeaders.insert("Content-Type","application/octet-stream");

    u32 key[8];
    getSessionKey(key);
    key[4] = url.queryItemValue("serial").toULong();
    barContent = encryptString(QString(barContent), key);
    memset(key,0,8);

    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}

void HttpHandlerSession::generateSqlSchema()
{
//    bool boolCommit = db.transaction();
    QSqlQuery query("CREATE TABLE IF NOT EXISTS sessions "
                    "(name, serial, code, access, lastcmd)", db);
    query.exec();
//    if (boolCommit) db.commit();
}
