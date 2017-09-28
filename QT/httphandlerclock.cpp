#include "httphandlerclock.h"

HttpHandlerClock::HttpHandlerClock(QObject *parent) :
    HttpHandlerGeneric(parent)
{
    QDateTime dateNow(QDate::currentDate(), QTime::currentTime());
    QLocale lcl;
    QByteArray barContent = lcl.toString(dateNow.toTimeSpec(Qt::UTC),
                                         "ddd, dd MMM yyyy hh:mm:ss").toAscii();
    mapHeaders.insert("Content-Type","text/plain");
    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}
