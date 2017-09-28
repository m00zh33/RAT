#include "httpbitsdir.h"
#include <QProcess>

HttpBitsDir::HttpBitsDir(QObject *parent) :
    HttpBitsGeneric(parent)
{
}

void HttpBitsDir::handleSessionClosed(QUuid guidSession,
                                      QFileInfo infoFile)
{
    myInfo = infoFile;
    myGuid = guidSession;
    if (url.queryItemValue("id").length() != 38) return;

    

    myId = QUuid(url.queryItemValue("id")); //TODO: this is wrong value

    //filename will be infoFile.filePath()
    //lets unlzma it
    QProcess *procLzma = new QProcess(this);

    connect(procLzma, SIGNAL(finished(int)),
            this, SLOT(handleLzmaClosed(int)));

    QFile::rename(infoFile.filePath(), infoFile.filePath() + ".txt.lzma");

#ifdef Q_OS_WIN32
    procLzma->start("lzma", QStringList() << "d" << infoFile.filePath()
                    + ".txt.lzma" << infoFile.filePath() + ".txt",
                    QIODevice::ReadWrite );
#else
    procLzma->start("lzma", QStringList() << "-d" << infoFile.filePath()
                    + ".txt.lzma", QIODevice::ReadWrite);
#endif
}

void HttpBitsDir::handleLzmaClosed(int errcode)
{
    QFile fileDirdump;
    QFile::rename(myInfo.filePath() + ".txt",
                  QString(P_FOLDER_SESSIONS + myId.toString() + ".txt"));
    fileDirdump.setFileName(
                QString(P_FOLDER_SESSIONS + myId.toString() + ".txt"));

    if (errcode > 0) {
        QFile::remove(fileDirdump.fileName().toLocal8Bit());
        return;
    }

    if (!db.isOpen() || db.isOpenError()) {
        db = this->setupSql();
        Q_ASSERT(db.isValid() && db.isOpen() && !db.isOpenError());
    }

    fileDirdump.open(QIODevice::ReadOnly | QIODevice::Text);
    Q_ASSERT(fileDirdump.isOpen());  
    QSqlDatabase dbFiles = QSqlDatabase::addDatabase("QSQLITE", myId.toString());
    dbFiles.setDatabaseName(QString(P_FOLDER_SESSIONS).append(myId.toString())
                            .append(".sql"));
    dbFiles.open();

    QSqlQuery sqlCreate(db);
    if (!db.tables().contains("dirdump")) {
        sqlCreate.prepare("CREATE TABLE dirdump (guid, state)");
        sqlCreate.exec();
        Q_ASSERT(sqlCreate.lastError().number() == -1);
    }
    sqlCreate.prepare("DELETE FROM dirdump WHERE guid = ?");
    sqlCreate.addBindValue(myId.toString());
    sqlCreate.exec();

    sqlCreate.prepare("INSERT INTO dirdump VALUES(?, 1)");
    sqlCreate.addBindValue(myId.toString());
    sqlCreate.exec();

    QSqlQuery sql(dbFiles);
    if (!dbFiles.tables().contains("drives")) {
        sql.prepare("CREATE TABLE drives (mountPath, type)");
        sql.exec();
    }

    if (!dbFiles.tables().contains("files")) {
        sql.prepare("CREATE TABLE files (path, type, name, size, created, "
                    "modified, accessed)");
        sql.exec();
    }

    if (fileDirdump.peek(1) == "[") {
        sql.prepare("DELETE FROM drives");
        sql.exec();
        sql.prepare("DELETE FROM files");
        sql.exec();
        sql.prepare("DROP INDEX IF EXISTS idx_path");
        sql.exec();
        dbFiles.transaction();
        fileDirdump.read(1); //skip "["
        while (!fileDirdump.atEnd() && fileDirdump.peek(1) == "{") {
            HttpBitsDir::handleRoot(fileDirdump, dbFiles);
        }
        dbFiles.commit();
        sql.prepare("CREATE INDEX idx_path ON files(path)");
        sql.exec();
    } else if (fileDirdump.peek(5) == "{\"r\":") {
        // we need to override a tree for a certein point
        QString root;
        QByteArray tmp;
        sql.prepare("DELETE FROM files WHERE path LIKE ?");
        fileDirdump.read(5);
        if (fileDirdump.peek(4) != "null") {
            readJsonString(fileDirdump, tmp);
            root = QString::fromUtf8(tmp);
            root.append("%");
            sql.addBindValue(root);
            sql.exec();
            root.chop(1);
            while (root.right(1) == "\\") root.chop(1);
            dbFiles.transaction();
            if (fileDirdump.read(5) == ",\"c\":") {
                handlePath(fileDirdump, dbFiles, root);
                dbFiles.commit();
            }
        }
    }

    sqlCreate.prepare("UPDATE dirdump SET state = 2 WHERE guid = ?");
    sqlCreate.addBindValue(myId.toString());
    sqlCreate.exec();

    dbFiles.close();
    QSqlDatabase::removeDatabase(myId.toString());
    fileDirdump.close();
    fileDirdump.remove();

    this->deleteLater();
}

void HttpBitsDir::handleRoot(QFile& fileDirdump, QSqlDatabase& db) {
    QString mountpoint, tmp = "";
    quint16 drivetype;
    bool ok;

    if (fileDirdump.read(6) != "{\"r\":\"") return;
    mountpoint = fileDirdump.read(3);
    while (fileDirdump.peek(1) != "\"") {
        mountpoint.append(fileDirdump.read(1));
    }
    if (mountpoint.right(1) == "\\") mountpoint.chop(1);
    if (fileDirdump.read(6) != "\",\"t\":") return;
    tmp.clear();
    {
        QChar chr = fileDirdump.peek(1).at(0);
        while (chr >= '0' && chr <= '9') {
            tmp.append(fileDirdump.read(1));
            chr = fileDirdump.peek(1).at(0);
        }
    }
    drivetype = tmp.toUInt(&ok);
    if (!ok && tmp != "0") return;
    if (fileDirdump.read(5) != ",\"c\":") return;
    handlePath(fileDirdump, db, mountpoint);
    tmp = fileDirdump.read(1);
    Q_ASSERT(tmp == "}");
    if (tmp != "}") return;

    QSqlQuery sql(db);
    sql.prepare("INSERT INTO drives VALUES(?, ?)");
    sql.addBindValue(mountpoint);
    sql.addBindValue(drivetype);
    sql.exec();   
}

void HttpBitsDir::handlePath(QFile& fileDirdump, QSqlDatabase& db, QString path) {
    QByteArray tmp;
    QString name;
    QString newpath;

    if (fileDirdump.read(5) != "{\"d\":") return;
    if (fileDirdump.peek(4) == "null") {
        newpath = name = path;
        fileDirdump.read(4);
    } else {
        readJsonString(fileDirdump, tmp);
        if (tmp.isEmpty()) return;
        name = QString::fromUtf8(tmp);
        newpath = path + "\\" + name;
    }
    if (fileDirdump.read(6) != ",\"c\":[") return;

    do {
        if (fileDirdump.peek(5) == "{\"f\":") {
            handleFile(fileDirdump, db, newpath);
        } else if (fileDirdump.peek(5) == "{\"d\":") {
            handlePath(fileDirdump, db, newpath);
        } else if (fileDirdump.peek(4) == "null") {
            fileDirdump.read(4);
        } else {
            return;
        }
    } while (fileDirdump.read(1) == ",");
    tmp = fileDirdump.read(1);
    Q_ASSERT(tmp == "}");
    if (tmp != "}") return;

    QSqlQuery sql(db);
    sql.prepare("INSERT INTO files VALUES(?, \"d\", ?, NULL, NULL, NULL, NULL)");
    sql.addBindValue(path);
    sql.addBindValue(name);
    sql.exec();
}

void HttpBitsDir::handleFile(QFile& fileDirdump, QSqlDatabase& db, QString path) {
    QByteArray tmp;
    QString name;
    qint64 size = 0;
    QDateTime created, modified, accessed;

    qApp->processEvents();

    if (fileDirdump.read(5) != "{\"f\":") return;
    readJsonString(fileDirdump, tmp);
    if (tmp.isEmpty()) return;
    name = QString::fromUtf8(tmp);
    if (fileDirdump.read(5) != ",\"s\":") return;
    forever {
        QChar chr = fileDirdump.read(1).at(0);
        if (chr >= '0' && chr <= '9') {
          size = size * 10 + chr.toAscii() - '0';
        } else {
            break;
        }
    }
    if (fileDirdump.read(4) != "\"r\":") return;
    readJsonDate(fileDirdump, created); // ,
    if (fileDirdump.read(4) != "\"m\":") return;
    readJsonDate(fileDirdump, modified); // ,
    if (fileDirdump.read(4) != "\"x\":") return;
    readJsonDate(fileDirdump, accessed); // }

    QSqlQuery sql(db);
    sql.prepare("INSERT INTO files VALUES(?, \"f\", ?, ?, ?, ?, ?)");
    sql.addBindValue(path);
    sql.addBindValue(name);
    sql.addBindValue(size);
    sql.addBindValue(created);
    sql.addBindValue(modified);
    sql.addBindValue(accessed);
    sql.exec();
}

void HttpBitsDir::readJsonString(QFile &fileDirdump, QByteArray &buffer) {
    buffer.clear();
    if (fileDirdump.read(1) != "\"") return;
    while (!fileDirdump.atEnd()) {
        buffer.append(fileDirdump.read(1));
        if (buffer.right(1) == "\"") {
            buffer.chop(1);
            return;
        }
    }
}

void HttpBitsDir::readJsonDate(QFile &fileDirdump, QDateTime &date) {
    QChar chr;
    qint8 sign = 1;
    quint64 seconds = 0;
    forever {
        chr = fileDirdump.read(1).at(0);
        if (chr == '-') {
            if (seconds) break;
            sign = -1;
            continue;
        }
        if (chr >= '0' && chr <= '9') {
            seconds = seconds * 10 + chr.toAscii() - '0';
        } else {
            break;
        }
    }
    seconds *= 1000 * sign;
    date = QDateTime::fromTime_t(0).addMSecs(seconds);
}
