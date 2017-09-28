#ifndef HTTPPARSERGET_H
#define HTTPPARSERGET_H

#include "httpparsergeneric.h"

class HttpParserGet : public HttpParserGeneric
{
    Q_OBJECT
public:
    explicit HttpParserGet( QObject * parent, QTcpSocket * tcpSocket );
    QUrl getUrl();
protected:
    void ErrorCommon(quint16 intErrCode, QByteArray barErrText);
    void ErrorNotFound();
    void ErrorAccessDenied();
    void ErrorBadParameters();
    void ErrorInvalidRange();
    void FoundElsewhere(QString strLocation);
    void FoundElsewherePermenant(QString strLocation);
    void FoundElsewhereCommon(bool boolPermanent, QString strLocation);
    void GenerateStaticFile();
    void ParseHeaders();
    void SendFile();
    bool ParseDynamicContent();

    QUrl urlRequested;
    QMap <QString, QString> mapHeaders;
    QFile fileRequested;
    QTimer tmrSendFilePart;
    quint32 intBytesRemaining;
protected slots:
    virtual void GenerateResponse();
    void ResponseEnd();
    void SendFilePart();
};

#include "httphandlerclock.h"
#include "httphandlerip.h"
#include "httphandlerp.h"
#include "httphandlersession.h"
#include "httphandlernoop.h"
#include "httphandlerreport.h"
#include "httphandlerotpgen.h"
#include "httphandlersecurelist.h"
#include "httphandlersecurecmd.h"
#include "httphandlersecuredir.h"

#endif // HTTPPARSERGET_H
