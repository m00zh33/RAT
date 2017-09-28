#ifndef HTTPD_H
#define HTTPD_H

#include "common.h"

#include <QtCore>
#include <QtNetwork>

#include "httpparsergeneric.h"
#include "httpparserget.h"
#include "httpparserhead.h"
#include "httpparserpost.h"
#include "httpparserbitspost.h"
#include "httpparseroptions.h"
#include "httpparserprofind.h"

class httpd : public QObject
{
    Q_OBJECT
public:
    httpd();
protected:
    int  parseHead(QTcpSocket *tcpSocket);
    void requestDataArrived(QTcpSocket *tcpSocket);
    void requestClosed(QTcpSocket *tcpSocket);

    QTcpServer tcpServer;
    QMap<QTcpSocket *, HttpParserGeneric *> mapParser;
    QMap<QString, quint16> mapRequests;
    QMap<QTcpSocket *, QTimer *> mapTimeouts;

protected slots:
    void requestNew();
    void requestClosed();
    void requestDataArrived();
    void requestError(QAbstractSocket::SocketError);
    void requestTimeout();
    void parserTermination();
};

#endif // HTTPD_H
