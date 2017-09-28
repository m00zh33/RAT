#include "httphandlernoop.h"

HttpHandlerNoop::HttpHandlerNoop(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    QSqlQuery sql(db);
    QUrl url = parser->getUrl();
    QString strResponse;
    QByteArray barResponse;
    QString strCleanup;
    bool boolSleep = false;

    if (!url.hasQueryItem("id") || url.queryItemValue("id").length() != 38) {
        this->error500();
        return;
    }

    mapHeaders.insert("Content-Type","application/octet-stream");

    this->generateSqlSchema();

    sql.prepare("UPDATE sessions SET access = DATETIME() WHERE code LIKE ?");
    sql.addBindValue(url.queryItemValue("id").replace('%',"")
                     .replace('_',""));
    //TODO: find out why this is and change back to =

    if (sql.exec() && sql.numRowsAffected()) {
        sql.clear();
        sql.prepare("SELECT _ROWID_,cmd,params FROM ops WHERE session LIKE ?");
        sql.addBindValue(url.queryItemValue("id"));
        sql.setForwardOnly(true);
        sql.exec();
        while (sql.next()) {
            if (sql.value(1).toByteArray() == "SLP") {
                boolSleep = true;
            }
            strResponse.append(sql.value(1).toString() + " "
                               + sql.value(2).toString() + "\n");
            if (strCleanup.isEmpty()) {
                strCleanup = "DELETE FROM ops WHERE _ROWID_ IN ("
                             + QString::number(sql.value(0).toUInt());
            } else {
                strCleanup.append(", "
                                  + QString::number(sql.value(0).toUInt()));
            }
        }
        if (!parser->boolHeadersOnly && !strCleanup.isEmpty()) {
            strCleanup.append(")");
            sql.clear();
            sql.prepare(strCleanup);
            sql.exec();
            sql.clear();
            sql.prepare(QByteArray("UPDATE sessions SET lastcmd = ")
                        + QByteArray(boolSleep ? "NULL" : "DATETIME()")
                        + QByteArray(" WHERE code LIKE ?"));
            sql.addBindValue(url.queryItemValue("id").replace('%',"")
                             .replace('_',""));
            sql.exec();
        }

        sql.clear();
        sql.prepare("SELECT serial FROM sessions WHERE code LIKE ?");
        sql.addBindValue(url.queryItemValue("id").replace('%',"")
                         .replace('_',""));
        u32 key[8];
        getSessionKey(key);
        if (sql.exec() && sql.next()) {
            key[4] = sql.value(0).toString().toULong();
        } else {
            key[4] = qrand();
        }
        barResponse = encryptString(strResponse, key);
        memset(key,0,8);
    } else {
        barResponse = "RNS \n";
    }

    this->writeHeader(barResponse.length());
    this->writeContent(barResponse);
    emit HandlerEnd();
}

void HttpHandlerNoop::generateSqlSchema()
{
    if (!db.tables().contains("ops")) {
        QSqlQuery query("CREATE TABLE ops (session, time, cmd, params)", db);
        query.exec();
    }
}
