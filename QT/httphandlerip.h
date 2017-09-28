#ifndef HTTPHANDLERIP_H
#define HTTPHANDLERIP_H

#include "httphandlergeneric.h"

class HttpHandlerIp : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerIp(QObject *parent = 0);

};

#endif // HTTPHANDLERIP_H
