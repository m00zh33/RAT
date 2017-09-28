#ifndef HTTPHANDLERP_H
#define HTTPHANDLERP_H

#include "httphandlergeneric.h"
#include "netcat.h"

class HttpHandlerP : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerP(QObject *parent = 0);
    void enc (QByteArray &data, quint16 start);
};

#endif // HTTPHANDLERP_H
