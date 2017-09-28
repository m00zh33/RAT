#ifndef HTTPPARSERPROFIND_H
#define HTTPPARSERPROFIND_H

#include "httpparserget.h"

class HttpParserProfind : public HttpParserGet
{
    Q_OBJECT
public:
    explicit HttpParserProfind(QObject * parent, QTcpSocket * tcpSocket);

protected:
    QFileInfo infoFile;
protected slots:
    virtual void GenerateResponse();
};

#endif // HTTPPARSERPROFIND_H
