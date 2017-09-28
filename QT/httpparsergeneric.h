#ifndef HTTPPARSERGENERIC_H
#define HTTPPARSERGENERIC_H

#include "common.h"

#include <QtCore>
#include <QtNetwork>

class HttpParserGeneric : public QObject
{
    Q_OBJECT
public:
    HttpParserGeneric( QObject * parent, QTcpSocket * tcpSocket );
    ~HttpParserGeneric();
    QTcpSocket * getTcpSocket();
    virtual QUrl getUrl();

    bool boolHeadersOnly;
protected:
    void SendResponse(QByteArray barLine, QString strResponse);
    virtual void SendResponseA(QByteArray barLine,
                       QByteArray barResponse,
                       QByteArray barEncoding);

    QTcpSocket *tcpSocket;
    QTimer tmrTimeout;
protected slots:
    virtual void GenerateResponse();
    virtual void ResponseEnd();
    void Timeout();
signals:
    void TerminateParser();
};

#endif // HTTPPARSERGENERIC_H
