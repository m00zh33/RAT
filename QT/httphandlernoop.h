#ifndef HTTPHANDLERNOOP_H
#define HTTPHANDLERNOOP_H

#include "httphandlergeneric.h"
#include "encryptio.h"

class HttpHandlerNoop : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerNoop(QObject *parent = 0);

protected:
    void generateSqlSchema();

};

#endif // HTTPHANDLERNOOP_H
