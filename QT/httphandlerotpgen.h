#ifndef HTTPHANDLEROTPGEN_H
#define HTTPHANDLEROTPGEN_H

#include "httphandlergeneric.h"

class HttpHandlerOtpgen : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerOtpgen(QObject *parent = 0);

protected:
    void generateSqlSchema();
    QChar randomChar();

};

#endif // HTTPHANDLEROTPGEN_H
