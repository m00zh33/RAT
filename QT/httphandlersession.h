#ifndef HTTPHANDLERSESSION_H
#define HTTPHANDLERSESSION_H

#include "httphandlergeneric.h"
#include <QUuid>
#include "encryptio.h"

class HttpHandlerSession : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerSession(QObject *parent = 0);

protected:
    void generateSqlSchema();

};

#endif // HTTPHANDLERSESSION_H
