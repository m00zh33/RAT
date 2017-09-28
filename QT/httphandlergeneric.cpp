#include "httphandlergeneric.h"

HttpHandlerGeneric::HttpHandlerGeneric(QObject *parent) :
    QObject(parent)
{
    db = QSqlDatabase::database();
    if (!this->setupSql()) {
        this->deleteLater();
        return;
    }

    Q_ASSERT(parent->property("HttpParserGeneric").toBool());
    if (!parent->property("HttpParserGeneric").toBool()) {
        this->deleteLater();
        return;
    }
    this->setParser(static_cast<HttpParserGeneric *>(parent));

    this->populateHeaders();
}

HttpHandlerGeneric::~HttpHandlerGeneric()
{    
}

bool HttpHandlerGeneric::setupSql()
{
    if (!db.isValid()) {
        db.close();
        db = QSqlDatabase::addDatabase("QSQLITE");
        Q_ASSERT(db.isValid());
        db.setDatabaseName("database.sqlite3");
        Q_ASSERT(db.databaseName() == "database.sqlite3");
        if (!db.open()) {
            Q_ASSERT(db.lastError().type() == QSqlError::NoError);
            return false;
        }
    } else if (!db.isOpen()) {
        db.close();
        db.setDatabaseName("database.sqlite3");
        if (!db.open()) {
            Q_ASSERT(db.lastError().type() == QSqlError::NoError);
            return false;
        }
    }
    return true;
}

void HttpHandlerGeneric::setParser(HttpParserGeneric *parser)
{
    this->parser = parser;
    connect(this, SIGNAL(HandlerEnd()), parser, SLOT(ResponseEnd()));
}

void HttpHandlerGeneric::populateHeaders()
{
    bool boolKeepAlive = parser->getTcpSocket()
                         ->property("keep-alive").toBool();
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;

    mapHeaders.insert("Connection", (boolKeepAlive ? "keep-alive" : "close"));
    if (boolKeepAlive) {
        mapHeaders.insert("Keep-Alive", "timeout="
                          +  QByteArray::number(P_KEEPALIVE_TIMEOUT/1000)
                          + ", max=" + QByteArray::number(P_KEEPALIVE_MAX));
    }
    mapHeaders.insert("Date",
                      lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                   "ddd, dd MMM yyyy hh:mm:ss").toAscii()
                      + " GMT");
    mapHeaders.insert("Server", "gws");
    mapHeaders.insert("X-XSS-Protection", "1; mode=block");
    mapHeaders.insert("X-Content-Type-Options", "nosniff");
}

void HttpHandlerGeneric::writeHeader(quint64 intContentLength,
                                     QByteArray barFirstLine) {
    Q_ASSERT(parser->getTcpSocket());
    QByteArray barResponse = "HTTP/1.";
    if (parser->getTcpSocket()->property("http-protocol").toUInt() == 11) {
        barResponse.append("1");
    } else {
        barResponse.append("0");
    }
    barResponse.append(" "+barFirstLine+"\r\n");

    if (intContentLength || parser->boolHeadersOnly) {
        mapHeaders.insert("Content-Length",
                          QByteArray::number(intContentLength));
    }

    QHashIterator<QByteArray, QByteArray> i(mapHeaders);
    while (i.hasNext()) {
        i.next();
        barResponse.append(i.key() + ": " + i.value() + "\r\n");
    }
    barResponse.append("\r\n");
    parser->getTcpSocket()->write(barResponse);
}

void HttpHandlerGeneric::writeContent(QByteArray barContent)
{
    if (parser->boolHeadersOnly) return;
    parser->getTcpSocket()->write(barContent);
}

void HttpHandlerGeneric::error500()
{
    QByteArray barContent = "500 Internal Server Error";
    this->writeHeader(barContent.length(), barContent);
    this->writeContent(barContent);
    emit HandlerEnd();
}
