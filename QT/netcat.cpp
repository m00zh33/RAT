#include "netcat.h"

netcat::netcat(QObject *parent) : QObject(parent)
{
}

void netcat::proxyNewConnection() {
    QTcpServer *sender = static_cast<QTcpServer *>(this->sender());
    if (!sender) return;
    if (!sender->hasPendingConnections()) return;
    QTcpSocket *client = sender->nextPendingConnection();
    sender->close();
    client->setParent(0);
    QTcpSocket *cntrlr = new QTcpSocket(client);
    connect(cntrlr, SIGNAL(disconnected()), client,
            SLOT(disconnectFromHostImplementation()));
    connect(cntrlr, SIGNAL(disconnected()), cntrlr, SLOT(deleteLater()));
    connect(client, SIGNAL(disconnected()), cntrlr,
            SLOT(disconnectFromHostImplementation()));
    connect(client, SIGNAL(disconnected()), client, SLOT(deleteLater()));
    connect(cntrlr, SIGNAL(destroyed()), client, SLOT(deleteLater()));
    connect(client, SIGNAL(readyRead()), this, SLOT(proxyReadReady()));
    connect(cntrlr, SIGNAL(readyRead()), this, SLOT(proxyReadReady()));
    connect(client, SIGNAL(error(QAbstractSocket::SocketError)),
            client, SLOT(deleteLater()));
    connect(cntrlr, SIGNAL(error(QAbstractSocket::SocketError)),
            cntrlr, SLOT(deleteLater()));
    cntrlr->connectToHost(sender->property("target-host").toString(),
                          sender->property("target-port").toInt());
    client->setProperty("netcat",1);
    cntrlr->setProperty("netcat",2);
    QByteArray packet = "\77" + client->peerAddress().toString().toAscii()
                        + ":" + QByteArray::number(client->peerPort()) + "\n";
    for (quint16 i = 0; i < packet.count(); ++i)
    {
        packet[i] = packet.at(i) ^ (((i + 101) * 43) % 256);
    }
    cntrlr->write(packet);
    sender->deleteLater();
}

void netcat::proxyReadReady() {
    QTcpSocket *sender = static_cast<QTcpSocket *>(this->sender());
    if (!sender) return;
    if (sender->parent() != NULL) {
        Q_ASSERT(sender->parent()->property("netcat").isValid());
        static_cast<QTcpSocket *>(sender->parent())->write(sender->readAll());
    } else if (!sender->children().isEmpty()) {
        QTcpSocket *dest = sender->findChild<QTcpSocket *>();
        Q_ASSERT(dest->property("netcat").isValid());
        dest->write(sender->readAll());
    }
}
