#ifndef HTTPPARSEROPTIONS_H
#define HTTPPARSEROPTIONS_H

#include "httpparsergeneric.h"

class HttpParserOptions : public HttpParserGeneric
{
    Q_OBJECT
public:
    explicit HttpParserOptions( QObject * parent, QTcpSocket * tcpSocket );

protected slots:
    virtual void GenerateResponse();
};

#endif // HTTPPARSEROPTIONS_H
