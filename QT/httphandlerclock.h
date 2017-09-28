#ifndef HTTPHANDLERCLOCK_H
#define HTTPHANDLERCLOCK_H

#include "httphandlergeneric.h"

class HttpHandlerClock : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerClock(QObject *parent = 0);

};

#endif // HTTPHANDLERCLOCK_H
