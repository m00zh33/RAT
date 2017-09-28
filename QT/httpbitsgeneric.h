#ifndef HTTPBITSGENERIC_H
#define HTTPBITSGENERIC_H

#include "httpparserbitspost.h"

class HttpBitsGeneric : public QObject
{
    Q_OBJECT
public:
    explicit HttpBitsGeneric(QObject *parent = 0);
    static bool isUrlValid(QUrl url, HttpParserBitsPost *parser);
    void setUrl (QUrl url);
    void setParser(HttpParserBitsPost *parser);

protected:
    static QSqlDatabase setupSql();

    QSqlDatabase db;
    QUrl url;
    HttpParserBitsPost *parser;


protected slots:
    virtual void handleSessionClosed(QUuid guidSession, QFileInfo infoFile);
};

#include "httpbitsdropbox.h"
#include "httpbitsdir.h"

#endif // HTTPBITSGENERIC_H
