#ifndef HTTPHANDLERGENERIC_H
#define HTTPHANDLERGENERIC_H

#include <QObject>
#include <QtSql>
#include <QMap>
#include "httpparsergeneric.h"

class HttpHandlerGeneric : public QObject
{
    Q_OBJECT
public:
    explicit HttpHandlerGeneric(QObject *parent = 0);
    ~HttpHandlerGeneric();
    void setParser(HttpParserGeneric *parser);

protected:
    void populateHeaders();
    void writeHeader(quint64 intContentLength,
                     QByteArray barFirstLine = "200 OK");
    void writeContent(QByteArray barContent);
    void error500();
    bool setupSql();

    QSqlDatabase db;
    HttpParserGeneric *parser;
    QHash <QByteArray, QByteArray> mapHeaders;
signals:
    void HandlerEnd();
};

#endif // HTTPHANDLERGENERIC_H
