#ifndef HTTPHANDLERSECUREDIR_H
#define HTTPHANDLERSECUREDIR_H

#include "httphandlersecure.h"

class HttpHandlerSecureDir : public HttpHandlerSecure
{
    Q_OBJECT
public:
    explicit HttpHandlerSecureDir(QObject *parent = 0);

protected:
    void showRequest();
    void showStatus();
    void showDrives();
    void showFolder();

    QUrl url;
    quint8 state;
    QSqlDatabase dbFiles;
    QString id;

    QString convertSize(quint32 size);
    QString convertDate(QDateTime inDate);
    QString convertPercent(const QString& in);
protected slots:
    void secure();

};

#endif // HTTPHANDLERSECUREDIR_H
