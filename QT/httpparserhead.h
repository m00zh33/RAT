#ifndef HTTPPARSERHEAD_H
#define HTTPPARSERHEAD_H

#include "httpparserget.h"

class HttpParserHead : public HttpParserGet
{
    Q_OBJECT
public:
    explicit HttpParserHead(QObject *parent, QTcpSocket *tcpSocket);

protected:
    void SendResponseA(QByteArray barLine,
                           QByteArray barResponse,
                           QByteArray barEncoding);
};

#endif // HTTPPARSERHEAD_H
