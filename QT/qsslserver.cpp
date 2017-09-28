#include "qsslserver.h"

QSslServer::QSslServer(QObject *parent) :
    QTcpServer(parent)
{
    QFile fileCa(P_PEM_CHAIN);
    if (fileCa.open(QIODevice::ReadOnly)) {
        QString strCert;
        foreach (strCert, QString::fromAscii(fileCa.readAll()).split("\n\n")) {
            QSslSocket::addDefaultCaCertificate(
                    QSslCertificate(strCert.toAscii()));
        }
        fileCa.close();
    }
}

void QSslServer::incomingConnection(int socketDescriptor)
{
    QSslSocket *socket = new QSslSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    socket->setLocalCertificate(P_PEM_CERT);
    socket->setPrivateKey(P_PEM_KEY);
    socket->setProtocol(QSsl::SslV3);
    socket->setProperty("QSslSocket", true);

    if (socket->mode() != QSslSocket::UnencryptedMode) {
        socket->deleteLater();
        return;
    }
    socket->startServerEncryption();
    connect(this, SIGNAL(destroyed()), this, SLOT(close_()));
    connect(socket, SIGNAL(encrypted()), this, SLOT(ready()));

    this->pendingConnections.append(socket);
}

bool QSslServer::hasPendingConnections() const
{
    return !this->pendingConnections.isEmpty();
}

QTcpSocket *QSslServer::nextPendingConnection()
{
    return static_cast<QTcpSocket *>(this->pendingConnections.takeFirst());
}

QSslSocket *QSslServer::nextPendingSslConnection()
{
    return this->pendingConnections.takeFirst();
}

void QSslServer::close_()
{
    qDeleteAll(this->pendingConnections);
    this->pendingConnections.clear();
    this->close();
}

void QSslServer::ready()
{
    Q_ASSERT(sender()->property("QSslSocket").toBool());
    QSslSocket *socket = static_cast<QSslSocket *>(sender());
    if (socket->mode() == QSslSocket::SslServerMode) {
        disconnect(socket, SLOT(startServerEncryption()));
    }
}
