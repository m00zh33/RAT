#include "httphandlersecuredir.h"

HttpHandlerSecureDir::HttpHandlerSecureDir(QObject *parent) :
    HttpHandlerSecure(parent)
{
}

void HttpHandlerSecureDir::secure()
{
    if (!this->setupSql()) {
        Q_ASSERT(db.isOpen() && !db.isOpenError());
        return this->error500();
    }
    if (!db.tables().contains("dirdump")) {
        QSqlQuery sqlCreate(db);
        sqlCreate.prepare("CREATE TABLES dirdump (guid, state)");
        sqlCreate.exec();
        Q_ASSERT(sqlCreate.lastError().number() == -1);
    }

    mapHeaders.insert("Content-Type", "text/html; charset=utf-8");
    url = this->parser->getUrl();

    if (!url.hasQueryItem("i") || !url.hasQueryItem("go")
      || url.queryItemValue("i").length() != 38) {
        return this->error500();
    }
    id = url.queryItemValue("i");

    QSqlQuery sql(db);
    sql.prepare("SELECT state FROM dirdump WHERE guid = ?");
    sql.addBindValue(id);
    sql.exec();
    if (!sql.next()) {
        this->showRequest();
        return;
    }
    if (sql.value(0).toInt() < 2) {
        this->showStatus();
        return;
    }
    dbFiles = QSqlDatabase::addDatabase("QSQLITE", id);
    dbFiles.setDatabaseName(QString(P_FOLDER_SESSIONS).append(id)
                            .append(".sql"));
    dbFiles.open();

    QString go = url.queryItemValue("go");
    if (go == "drive") {
        showDrives();
    } else if (go == "path") {
        showFolder();
    }

    dbFiles.close();
    QSqlDatabase::removeDatabase(id);
}

void HttpHandlerSecureDir::showRequest() {
    QFile file("htstatic/dirrequest.html");

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return this->error500();
    }

    QString strContent = QString::fromUtf8(file.readAll()).arg(0).arg(0)
            .arg(id);
    file.close();

    this->writeHeader(strContent.toUtf8().length());
    this->writeContent(strContent.toUtf8());
    emit HandlerEnd();
}

void HttpHandlerSecureDir::showStatus() {
    QFile file("htstatic/dirstatus.html");

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return this->error500();
    }

    QString strContent = QString::fromUtf8(file.readAll());
    file.close();

    this->writeHeader(strContent.toUtf8().length());
    this->writeContent(strContent.toUtf8());
    emit HandlerEnd();
}

void HttpHandlerSecureDir::showDrives() {
    QFile file("htstatic/dirdrives.html");

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return this->error500();
    }
    QStringList lstContent = QString::fromUtf8(file.readAll())
            .split("%!%");
    file.close();
    if (lstContent.count() < 3) {
        return this->error500();
    }

    QByteArray barContent = lstContent.at(0).arg(0).arg(0).arg(id).toUtf8();

    QSqlQuery sql(dbFiles);
    sql.prepare("SELECT mountPath, type FROM drives LIMIT 100");
    sql.exec();
    const QStringList typeToName = QStringList() << "bla0" << "bla1"
                                                 << "Removable" << "Fixed"
                                                 << "Network" << "bla5"
                                                 << "bla6" << "bla7";
    while (sql.next()) {
        QString buff = lstContent.at(1);
        QString path = sql.value(0).toString();
        quint16 type = sql.value(1).toInt();
        QString drivetype = typeToName.at(type);
        barContent.append(buff.arg(path).arg(path).arg(id)
                          .arg(QString::number(type)).arg(drivetype)
                          .toUtf8());
    }
    barContent.append(lstContent.at(2).toUtf8());

    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}

void HttpHandlerSecureDir::showFolder() {
    QFile file("htstatic/dirpath.html");

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return this->error500();
    }
    const QString path = url.queryItemValue("path");
    QStringList lstContent = QString::fromUtf8(file.readAll()).split("%!%");
    file.close();
    QString uppath = path;
    QString rootpath = uppath.left(2);
    if (uppath.indexOf('\\') == -1) {
        uppath = rootpath;
    } else {
        uppath = uppath.left(uppath.lastIndexOf('\\'));
    }
    QByteArray barContent = lstContent.at(0).arg(id).arg(uppath)
            .arg(rootpath).arg(path).toUtf8();
    QString strOrderby = url.hasQueryItem("order")
            ? url.queryItemValue("order") : "";
    QString orderby = "name ASC";
    QRegExp rxField("^(name|type|size|created|modified|accessed) "
                    "(asc|desc)$");
    if (-1 != rxField.indexIn(strOrderby)) {
        orderby = strOrderby;
    }
    QSqlQuery sql(dbFiles);
    sql.prepare("SELECT path, type, name, size, created, modified, accessed"
                " FROM files WHERE path = ? ORDER BY type ASC, " + orderby
                + " LIMIT 10000");
    sql.addBindValue(path);
    sql.exec();
    QString buffer;
    const QString dropbox("https://" P_HOSTNAME "/cgi-bin/drop?id=%1&name=");
    while (sql.next()) {
        buffer.clear();
        QString newpath = sql.value(0).toString() + "\\"
                + sql.value(2).toString();
        if (sql.value(1).toString() == "d") {
            if (sql.value(2).toString() == path)
                continue; //C: hack
            buffer = lstContent.at(1).arg(id)
                .arg(sql.value(2).toString()) //name
                .arg(convertPercent(newpath)); //path

        } else if (sql.value(1).toString() == "f") {
            QString size = convertSize(sql.value(3).toUInt());
            buffer = lstContent.at(2).arg(id) // 1
                    .arg(sql.value(2).toString()) // 2 name
                    .arg(size) // 3 size
                    .arg(convertDate(sql.value(5).toDateTime())) //4modified
                    .arg(convertDate(sql.value(6).toDateTime())) //5accessed
                    .arg(convertDate(sql.value(4).toDateTime())); //6created
            if (newpath.indexOf(" ")) {
                newpath.prepend("\"");
                newpath.append("\"");
            }
            buffer = buffer.replace("%NPATH", convertPercent(newpath))
                    .replace("%DBX", convertPercent(dropbox.arg(id)));
        }
        barContent.append(buffer.toUtf8());
    }

    barContent.append(lstContent.at(3).arg(id)
                      .arg(convertPercent("\""+path+"\\\"")).toUtf8());

    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}

QString HttpHandlerSecureDir::convertSize(quint32 size) {
    QString result = "<span title='" + QString::number(size) + "'>";
    if (size < 1024) {
        result.append(QString::number(size) + " bytes");
        if (size == 1) result.chop(1);
    } else {
        size = (0.5 + size) / 1024;
        if (size < 1024) {
            result.append(QString::number(size) + "kb");
        } else {
            size = (0.5 + size) / 1024;
            if (size < 1024) {
                result.append(QString::number(size) + "mb");
            } else {
                size = (0.5 + size) / 1024;
                result.append(QString::number(size) + "gb");
            }
        }
    }
    result.append("</span>");
    return result;
}

QString HttpHandlerSecureDir::convertDate(QDateTime inDate) {
    const QDateTime now = QDateTime::currentDateTime();
    inDate.setTimeSpec(Qt::UTC);
    QString result = "<span title='"
            + inDate.toString("dd.MM.yyyy hh:mm:ss") + "'>%1</span>";
    qint32 secsToNow = inDate.secsTo(now);
    if (secsToNow < 60) {
        return result.arg(QString::number(secsToNow) + "s ago");
    }
    secsToNow /= 60;
    if (secsToNow < 60) {
        return result.arg(QString::number(secsToNow) + "m ago");
    }
    secsToNow /= 60;
    if (secsToNow < 24) {
        return result.arg(QString::number(secsToNow) + "h ago");
    }
    secsToNow /= 24;
    if (secsToNow < 31) {
        return result.arg(QString::number(secsToNow) + "d ago");
    }
    if (secsToNow < 365) {
        return result.arg(inDate.toString("dd.MM"));
    }
    return result.arg(inDate.toString("dd.MM.yyyy"));
}

QString HttpHandlerSecureDir::convertPercent(const QString& in) {
    QByteArray tmp = in.toUtf8().toPercentEncoding();
    QString result = QString::fromAscii(tmp);
    return result;
}
