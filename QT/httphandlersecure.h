#ifndef HTTPHANDLERSECURE_H
#define HTTPHANDLERSECURE_H

#include <QFile>
#include <QRegExp>
#include "httphandlergeneric.h"

class HttpHandlerSecure : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerSecure(QObject *parent = 0);

protected:
    bool isIdentified();
    void renderLoginScreen();
    void generateSqlSchemaIdentification();
    void parsePostToUrl(QUrl *url);

protected slots:
    virtual void secure();

};

#include "httpparserpost.h"

#endif // HTTPHANDLERSECURE_H
