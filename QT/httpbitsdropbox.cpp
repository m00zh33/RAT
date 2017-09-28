#include "httpbitsdropbox.h"

HttpBitsDropbox::HttpBitsDropbox(QObject *parent) :
    HttpBitsGeneric(parent)
{
}

void HttpBitsDropbox::handleSessionClosed(QUuid guidSession,
                                          QFileInfo infoFile)
{
    this->generateSqlSchema();
    QSqlQuery sql(db);

    sql.prepare("INSERT INTO dropbox VALUES( ?, DATETIME(), ?, ?, ? )");
    sql.addBindValue(url.queryItemValue("id"));
    sql.addBindValue(url.queryItemValue("name"));
    sql.addBindValue(guidSession.toString());
    sql.addBindValue(infoFile.size());
    sql.exec();
    this->deleteLater();
}

void HttpBitsDropbox::generateSqlSchema()
{
    QSqlQuery query(db);
    if (!db.tables().contains("dropbox")) {
        query.prepare("CREATE TABLE dropbox "
                      "(session, created, name, fileguid, size)");
        query.exec();
    }
}
