#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QtGui/QMainWindow>
#include <QtNetwork>
#include <QStandardItemModel>
#include "qtwin.h"
#include "portmapper.h"

namespace Ui
{
    class controller;
}

class controller : public QMainWindow
{
    Q_OBJECT

public:
    controller(QWidget *parent = 0);
    ~controller();
    quint8 findFreeSlot();
    bool socketDataComplete(quint16 minSize, QTcpSocket *&socket);
    void log_add(QString entry);
    bool portMapAdd(QHostAddress bindIp, quint16 bindPort,
                    QHostAddress destIp, quint16 destPort);
    bool portMapDel(QString bindIp, quint16 bindPort);
    void portClear();
    void portPopulate(QTableView *lstTable);

protected:
    QTcpServer *sckController;
    QTcpServer *sckSocks5;
    QByteArray barData;
    bool isConnected;
    QVector<bool> lstSlotsFree;
    QVector<QTcpSocket*> lstSlots;
    quint8 intLastFree;
    QNetworkAccessManager *downloader;
    QByteArray barMyIP;
    QList<QTcpServer*> lstPortMap;
    QPointer<portmapper> portmapperDialog;

private:
    Ui::controller *ui;

private slots:
    void on_cmdPortMapper_clicked();
    void on_cmdUpdate_clicked();
    void on_commandLinkButton_2_clicked();
    void on_commandLinkButton_clicked();
    void on_pushSocksPort_clicked();
    void on_pusControllerPort_clicked();
    void sckController_readyRead();
    void sckSocks_newConnection();
    void sckController_newConnection();
    void sckController_disconnect();
    void sckSocks_disconnect();
    void sckSocks_readyRead();
    void downloadMyIp(QNetworkReply* reply);
    void downloadReqCon(QNetworkReply* reply);
    void downloadNext();
    void sckMapping_newConnection();
};

#endif // CONTROLLER_H
