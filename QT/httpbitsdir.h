#ifndef HTTPBITSDIR_H
#define HTTPBITSDIR_H

#include "httpbitsgeneric.h"

class HttpBitsDir : public HttpBitsGeneric
{
    Q_OBJECT
public:
    explicit HttpBitsDir(QObject *parent = 0);

    static void handleRoot(QFile&, QSqlDatabase&);
    static void handlePath(QFile&, QSqlDatabase&, QString);
    static void handleFile(QFile&, QSqlDatabase&, QString);
    static void readJsonString(QFile&, QByteArray&);
    static void readJsonDate(QFile&, QDateTime&);

signals:

protected:
    QFileInfo myInfo;
    QUuid myGuid;
    QUuid myId;

protected slots:
    void handleSessionClosed(QUuid guidSession, QFileInfo infoFile);
    void handleLzmaClosed(int errcode);

};

#endif // HTTPBITSDIR_H
