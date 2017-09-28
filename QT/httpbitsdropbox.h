#ifndef HTTPBITSDROPBOX_H
#define HTTPBITSDROPBOX_H

#include "httpbitsgeneric.h"

class HttpBitsDropbox : public HttpBitsGeneric
{
    Q_OBJECT
public:
    explicit HttpBitsDropbox(QObject *parent = 0);

protected:
    void generateSqlSchema();

protected slots:
    void handleSessionClosed(QUuid guidSession, QFileInfo infoFile);

};

#endif // HTTPBITSDROPBOX_H
