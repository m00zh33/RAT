#ifndef HTTPHANDLERSECURECMD_H
#define HTTPHANDLERSECURECMD_H

#include "httphandlersecure.h"

class HttpHandlerSecureCmd : public HttpHandlerSecure
{
    Q_OBJECT
public:
    explicit HttpHandlerSecureCmd(QObject *parent = 0);


protected:
    QUrl url;
    void overview();
    void updateTime();
    void updateQueue();
    void cmd();
    void kill();

protected slots:
    void secure();

};

#endif // HTTPHANDLERSECURECMD_H
