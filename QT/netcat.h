#ifndef NETCAT_H
#define NETCAT_H

#include <QTcpSocket>
#include <QTcpServer>

class netcat : public QObject
{
    Q_OBJECT
public:
    explicit netcat(QObject *parent = 0);

protected slots:
    void proxyNewConnection();
    void proxyReadReady();

};

#endif // NETCAT_H
