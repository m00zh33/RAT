#include "httpbitsgeneric.h"

HttpBitsGeneric::HttpBitsGeneric(QObject *parent) :
    QObject(parent)
{
    db = this->setupSql();
}

QSqlDatabase HttpBitsGeneric::setupSql()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid()) {
        db.close();
        db = QSqlDatabase::addDatabase("QSQLITE");
        Q_ASSERT(db.isValid());
        db.setDatabaseName("database.sqlite3");
        Q_ASSERT(db.databaseName() == "database.sqlite3");
        if (!db.open()) {
            Q_ASSERT(db.lastError().type() == QSqlError::NoError);
            return db;
        }
    } else if (!db.isOpen()) {
        db.close();
        db.setDatabaseName("database.sqlite3");
        if (!db.open()) {
            Q_ASSERT(db.lastError().type() == QSqlError::NoError);
            return db;
        }
    }
    Q_ASSERT(db.isOpen());
    return db;
}

void HttpBitsGeneric::setUrl(QUrl url)
{
    this->url = url;
}

void HttpBitsGeneric::setParser(HttpParserBitsPost *parser)
{
    this->parser = parser;
}

bool HttpBitsGeneric::isUrlValid(QUrl url, HttpParserBitsPost *parser)
{
    QSqlDatabase db = HttpBitsGeneric::setupSql();

    QSqlQuery sql(db);
    HttpBitsGeneric *bits;

    if (url.host() != P_HOSTNAME) return false;
    if (!url.hasQueryItem("id") || url.queryItemValue("id").length() != 38)
        return false;

    sql.prepare("SELECT 1 FROM sessions WHERE code LIKE ?");
    sql.addBindValue(url.queryItemValue("id").replace('%',"")
                     .replace('_',""));

    if (!sql.exec() || !sql.next()) return false;

    if (url.path() == "/cgi-bin/drop") {
        if (!url.hasQueryItem("name")) return false;
        bits = new HttpBitsDropbox();
    } else if (url.path() == "/cgi-bin/dir") {
        bits = new HttpBitsDir();
    } else {
        return false;
    }
    bits->setParser(parser);
    bits->setUrl(url);
    connect(parser, SIGNAL(SessionClosed(QUuid,QFileInfo)),
            bits, SLOT(handleSessionClosed(QUuid,QFileInfo)));
    return true;
}


void HttpBitsGeneric::handleSessionClosed(QUuid guidSession,
                                          QFileInfo infoFile)
{
    guidSession = guidSession;
    infoFile = infoFile;
}
