#ifndef QSSLSERVER_H
#define QSSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QFile>
#include <QStringList>
#include "common.h"

class QSslServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit QSslServer(QObject *parent = 0);
    void incomingConnection(int socket);
    virtual bool hasPendingConnections() const;
    virtual QTcpSocket *nextPendingConnection();
    virtual QSslSocket *nextPendingSslConnection();
public slots:
    void close_();

protected:
    QList <QSslSocket *>pendingConnections;

protected slots:
    void ready();

};

#endif // QSSLSERVER_H
