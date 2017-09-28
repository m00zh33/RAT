#ifndef HTTPHANDLERSECURELIST_H
#define HTTPHANDLERSECURELIST_H

#include "httphandlersecure.h"

class HttpHandlerSecureList : public HttpHandlerSecure
{
    Q_OBJECT
public:
    explicit HttpHandlerSecureList(QObject *parent = 0);

protected:
    void renderAjaxList();
    void renderHtmlList();

    QUrl url;
    quint16 intLimit;

protected slots:
    void secure();

};

#endif // HTTPHANDLERSECURELIST_H
