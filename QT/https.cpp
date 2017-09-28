#include "https.h"

https::https() : httpd()
{
    if (!this->sslServer.listen(P_SSL_BIND, P_SSL_PORT))
    {
        this->deleteLater();
        return;
    }
    connect(&this->sslServer, SIGNAL(newConnection()),
            this, SLOT(requestNew()));
}
