#ifndef HTTPPARSERPOST_H
#define HTTPPARSERPOST_H

#include "httpparserget.h"

class HttpParserPost : public HttpParserGet
{
    Q_OBJECT
public:
    explicit HttpParserPost(QObject *parent, QTcpSocket *tcpSocket);

    QByteArray barPostData;
protected slots:
    virtual void GenerateResponse();
    virtual void ReadPost();

};

#endif // HTTPPARSERPOST_H
