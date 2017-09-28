#ifndef HTTPHANDLERREPORT_H
#define HTTPHANDLERREPORT_H

#include "httphandlergeneric.h"

class HttpHandlerReport : public HttpHandlerGeneric
{
    Q_OBJECT
public:
    explicit HttpHandlerReport(QObject *parent = 0);

protected:
    void generateSqlSchema();

};

#endif // HTTPHANDLERREPORT_H
