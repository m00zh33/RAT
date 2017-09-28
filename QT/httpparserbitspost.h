#ifndef HTTPPARSERBITSPOST_H
#define HTTPPARSERBITSPOST_H

#include "httpparserpost.h"

class HttpParserBitsPost : public HttpParserPost
{
    Q_OBJECT
public:
    explicit HttpParserBitsPost(QObject *parent, QTcpSocket *tcpSocket);

protected:
    void HandlePing();
    void HandleCreateSession();
    void HandleFragment();
    void HandleCloseSession();
    void HandleCancelSession();
    bool HandleEndSession(bool boolIgnoreSize);

    void SendResponseA(QByteArray barLine,
                       QByteArray barResponse,
                       QByteArray barEncoding);
    QByteArray GenerateCommonHeaders(quint16 intHttpCode,
                                     QByteArray barHttpDesc,
                                     QByteArray barBitsType,
                                     quint64 intContentLength);

    QUuid guidSession;
protected slots:
    virtual void GenerateResponse();
    void ReadPost();

signals:
    void SessionClosed(QUuid guidSession, QFileInfo infoFile);
};

#include "httpbitsgeneric.h"

#endif // HTTPPARSERBITSPOST_H
