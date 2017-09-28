#include "httphandlerp.h"

HttpHandlerP::HttpHandlerP(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    static netcat *nc = new netcat();
    QByteArray barContent = parser->getTcpSocket()->property("http-head")
                            .toByteArray().split('\n').at(0).split(' ')
                            .at(1).split('?').at(1).mid(2);
    barContent = QByteArray::fromPercentEncoding(barContent);
    enc(barContent, 0);
    if (barContent.left(5) == "1000;") {
        barContent = barContent.mid(5);
        QSqlQuery sqlQuery(db);
        if (!db.tables().contains("pytheas1")) {
            sqlQuery.prepare("CREATE TABLE pytheas1 (proxyport, pid)");
            sqlQuery.exec();
        }
        if (barContent == "0") {
            sqlQuery.prepare("SELECT _ROWID_, proxyport FROM pytheas1 LIMIT 1");
            barContent = "\x07";
            if (sqlQuery.exec() && sqlQuery.next()
                && sqlQuery.value(0).isValid()) {
                quint16 proxyport = sqlQuery.value(1).toInt();
                barContent = "\1" +
                             QByteArray::fromRawData((const char*)&proxyport,2);
                barContent.append(P_HOSTNAME);
                enc(barContent, 0);
                barContent.prepend("\x08");
                QString rowid = sqlQuery.value(0).toString();
                sqlQuery.clear();
                sqlQuery.prepare("DELETE FROM pytheas1 WHERE _ROWID_ = ?");
                sqlQuery.addBindValue(rowid);
                sqlQuery.exec();
            }
        } else {
            enc(barContent, 0);
            if (barContent[0] == '\1') {
                quint16 proxyport = (* (quint16 *)barContent.mid(1,2).data() );
                QTcpServer *tcpProxyServer;
                tcpProxyServer = new QTcpServer(nc);
                QTimer *tmrKillStale = new QTimer(tcpProxyServer);
                tmrKillStale->singleShot(900000, tcpProxyServer,
                                         SLOT(deleteLater()));
#ifdef _MSC_VER
                tmrKillStale = tmrKillStale;
#endif
                tcpProxyServer->setProperty("target-host", barContent.mid(3));
                tcpProxyServer->setProperty("target-port", proxyport);
                tcpProxyServer->setMaxPendingConnections(1);
                connect(tcpProxyServer, SIGNAL(newConnection()), nc,
                        SLOT(proxyNewConnection()));
                barContent = (tcpProxyServer->listen() ? "\7" : "\0");
                sqlQuery.prepare("INSERT INTO pytheas1 VALUES(?, 0);");
                proxyport = tcpProxyServer->serverPort();
                sqlQuery.addBindValue(proxyport);
                sqlQuery.exec();
            } else {
                barContent = "";
            }
        }
    }
    mapHeaders.insert("Content-Type","text/plain");
    barContent = barContent.toBase64();
    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}

void HttpHandlerP::enc (QByteArray &data, quint16 start)
{
    for (quint16 i = start; i < data.count(); ++i)
    {
        data[i] = data.at(i) ^ (((i + 101) * 43) % 256);
    }
}
