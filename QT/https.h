#ifndef HTTPS_H
#define HTTPS_H

#include "httpd.h"
#include "qsslserver.h"

class https : public httpd
{
    Q_OBJECT
public:
    https();
protected:

    QSslServer sslServer;
};

#endif // HTTPS_H
