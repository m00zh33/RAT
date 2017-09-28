#include "httphandlerip.h"

HttpHandlerIp::HttpHandlerIp(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    QByteArray barContent = parser->getTcpSocket()
                            ->peerAddress().toString().toAscii();
    mapHeaders.insert("Content-Type","text/plain");
    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}
