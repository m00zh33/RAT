#include "httphandlersecurecmd.h"

HttpHandlerSecureCmd::HttpHandlerSecureCmd(QObject *parent) :
        HttpHandlerSecure(parent) {}

void HttpHandlerSecureCmd::secure()
{
    if (!this->setupSql()) {
        Q_ASSERT(db.isOpen() && !db.isOpenError());
        return this->error500();
    }

    mapHeaders.insert("Content-Type", "text/html; charset=utf-8");
    url = this->parser->getUrl();

    if (!url.hasQueryItem("i")
      || url.queryItemValue("i").length() != 38) {
        return this->error500();
    }

    if (url.hasQueryItem("update")) {
        if (url.queryItemValue("update") == "time") {
            this->updateTime();
        } else if (url.queryItemValue("update") == "queue") {
            this->updateQueue();
        }
    } else if (url.hasQueryItem("kill")) {
        this->kill();
    } else if (url.hasQueryItem("cmd")) {
        this->cmd();
    } else {
        this->overview();
    }
}

void HttpHandlerSecureCmd::overview()
{
    QString strContent;
    QDateTime date, now = QDateTime::currentDateTime();
    QFile file("htstatic/cmd.html");
    qint64 intSecs;

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return this->error500();
    }
    /*
     * %1 name
     * %2 serial
     * %3 code
     * %4 last-access
     * %5 next-access
     */
    QSqlQuery sql(db);
    sql.setForwardOnly(true);
    sql.prepare("SELECT name, serial, code, access FROM sessions WHERE "
                "code LIKE ?");
    sql.addBindValue(url.queryItemValue("i"));
    if (!sql.exec() || !sql.next()) {
        file.close();
        url.removeQueryItem("i");
        url.setPath("/cgi-bin/list");
#ifdef _STUNNEL_HACK
        url.setScheme("https");
#endif
        mapHeaders.insert("Location", url.toEncoded());
        this->writeHeader(0, "302 Found");
        emit HandlerEnd();
        return;
    }

    strContent = QString::fromUtf8(file.readAll());
    file.close();

    strContent = strContent.arg(sql.value(0).toString())
                 .arg(sql.value(1).toString())
                 .arg(sql.value(2).toString());
    date = sql.value(3).toDateTime();
    date.setTimeSpec(Qt::UTC);
    intSecs = date.secsTo(now);
    if (intSecs == 1) {
        strContent = strContent.arg(QString("1 second ago"));
    } else if (intSecs < 60) {
        strContent = strContent.arg(QString("%1 seconds ago")
                           .arg(QString::number(intSecs)));
    } else if (intSecs < 120) {
        strContent = strContent.arg(QString("1 minute ago"));
    } else if (intSecs < 3600) {
        strContent = strContent.arg(QString("%1 minutes ago")
                           .arg(QString::number(intSecs/60)));
    } else if (intSecs < 7200) {
        strContent = strContent.arg(QString("1 hour ago"));
    } else if (intSecs < 86400) {
        strContent = strContent.arg(QString("%1 hours ago")
                           .arg(QString::number(intSecs/3600)));
    } else if (intSecs < 172800) {
        strContent = strContent.arg(QString("1 day ago"));
    } else {
        intSecs = date.daysTo(now);
        strContent = strContent.arg(QString("%1 days ago")
                           .arg(QString::number(intSecs)));
    }
    date = date.addSecs(60*15);
    intSecs = now.secsTo(date);
    if (intSecs < 0) {
        strContent = strContent.arg("maybe never");
    } else if (intSecs < 60) {
        strContent = strContent.arg(QString("in %1 seconds")
                       .arg(QString::number(intSecs)));
    } else {
        strContent = strContent.arg(QString("in %1 minutes")
                           .arg(QString::number((intSecs/60)+1)));
    }

    this->writeHeader(strContent.toUtf8().length());
    this->writeContent(strContent.toUtf8());
    emit HandlerEnd();
}

void HttpHandlerSecureCmd::updateTime()
{
    QByteArray barContent;
    QDateTime date, date2, now = QDateTime::currentDateTime();
    qint64 intSecs, intDate2;
    QSqlQuery sql(db);
    sql.setForwardOnly(true);
    sql.prepare("SELECT access FROM sessions WHERE code LIKE ?");
    sql.addBindValue(url.queryItemValue("i"));
    if (!sql.exec() || !sql.next()) {
        this->writeHeader(0, "204 No Content");
        emit HandlerEnd();
        return;
    }

    date = sql.value(0).toDateTime();
    date.setTimeSpec(Qt::UTC);
    intSecs = date.secsTo(now);

    barContent.append("Last seen: ");

    if (intSecs == 1) {
       barContent.append("1 second ago");
    } else if (intSecs < 60) {
        barContent.append(QString("%1 seconds ago")
                           .arg(QString::number(intSecs)).toUtf8());
    } else if (intSecs < 120) {
        barContent.append("1 minute ago");
    } else if (intSecs < 3600) {
        barContent.append(QString("%1 minutes ago")
                           .arg(QString::number(intSecs/60))
                           .toUtf8());
    } else if (intSecs < 7200) {
        barContent.append("1 hour ago");
    } else if (intSecs < 86400) {
        barContent.append(QString("%1 hours ago")
                           .arg(QString::number(intSecs/3600))
                           .toUtf8());
    } else if (intSecs < 172800) {
        barContent.append("1 day ago");
    } else {
        intSecs = date.daysTo(now);
        barContent.append(QString("%1 days ago")
                           .arg(QString::number(intSecs)).toUtf8());
    }

    sql.clear();
    sql.prepare("SELECT lastcmd FROM sessions WHERE code LIKE ?");
    sql.addBindValue(url.queryItemValue("i").replace('%',"")
                     .replace('_',""));
    sql.exec();
    barContent.append(", Next command sync: ");
    if (sql.next() && !sql.value(0).isNull()) {
        date2 = sql.value(0).toDateTime();
        date2.setTimeSpec(Qt::UTC);
        intDate2 = date2.secsTo(now);
        if (intDate2 > 60*60) {
            date = date.addSecs(60*15);
        } else if (intDate2 > 60*15) {
            date = date.addSecs(60*10);
        } else if (intDate2 > 60*5) {
            date = date.addSecs(60*5);
        } else {
            date = date.addSecs(60*1);
        }
    } else {
        date = date.addSecs(60*15);
    }
    intSecs = now.secsTo(date);
    if (intSecs < 0) {
        barContent.append("maybe never");
    } else if (intSecs < 60) {
        barContent.append(QString("in %1 seconds")
                       .arg(QString::number(intSecs)).toUtf8());
    } else {
        barContent.append(QString("in %1 minutes")
                           .arg(QString::number((intSecs/60)+1))
                           .toUtf8());
    }

    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}

void HttpHandlerSecureCmd::updateQueue()
{
    QByteArray barContent;
    QString strContent, strClean;
    QDateTime date, now = QDateTime::currentDateTime();
    qint64 intSecs;
    QSqlQuery sql(db);

    sql.setForwardOnly(true);
    sql.prepare("SELECT _ROWID_, time, cmd, params FROM ops WHERE session "
                "LIKE ? ORDER BY time DESC LIMIT 100");
    sql.addBindValue(url.queryItemValue("i"));
    sql.exec();
    barContent = "<b>Outbox:</b><ul class=\"outbox\">";
    while (sql.next()) {
        barContent.append("<li><b>");
        strContent = sql.value(2).toString();
        strContent.replace('<',"&lt;").replace('>',"&gt;")
                .replace('&',"&amp;");
        barContent.append(strContent.toUtf8());
        barContent.append("</b> ");
        strContent = sql.value(3).toString();
        strContent.replace('<',"&lt;").replace('>',"&gt;")
                .replace('&',"&amp;");
        barContent.append(strContent.toUtf8());

        date = sql.value(1).toDateTime();
        date.setTimeSpec(Qt::UTC);
        intSecs = date.secsTo(now);
        strContent = " (%1 %2 ago) ";
        if (intSecs > 60) {
            barContent.append(strContent.arg(QString::number(intSecs/60))
                              .arg("minutes").toUtf8());
        } else {
            barContent.append(strContent.arg(QString::number(intSecs))
                              .arg("seconds").toUtf8());
        }
        barContent.append("<a href=\"cmd?kill="
                          + QByteArray::number(sql.value(0).toUInt()) + "&i="
                          + url.queryItemValue("i") + "\">kill</a> ");
        barContent.append("</li>");
    }

    barContent.append("</ul><b>Reports:</b><ul class=\"reports\">");
    sql.clear();
    sql.prepare("SELECT _ROWID_, time, cmd, entry, reviewed FROM reports "
                "WHERE session LIKE ? ORDER BY time DESC LIMIT 10");
    sql.addBindValue(url.queryItemValue("i"));
    sql.setForwardOnly(true);
    sql.exec();
    while (sql.next()) {
        barContent.append("<li><b>"+sql.value(2).toString().toUtf8()+"</b> ");
        barContent.append(sql.value(3).toString().toUtf8());
        date = sql.value(1).toDateTime();
        date.setTimeSpec(Qt::UTC);
        intSecs = date.secsTo(now);
        strContent = " (%1 %2 ago) </li>";
        if (intSecs > 60) {
            barContent.append(strContent.arg(QString::number(intSecs/60))
                              .arg("minutes").toUtf8());
        } else {
            barContent.append(strContent.arg(QString::number(intSecs))
                              .arg("seconds").toUtf8());
        }
        if (sql.value(4).toInt()) continue;
        if (strClean.isEmpty()) {
            strClean = "UPDATE reports SET reviewed = DATETIME() WHERE "
                      "_ROWID_ IN (" + QString::number(sql.value(0).toUInt());
        } else {
            strClean.append(", " + QString::number(sql.value(0).toUInt()));
        }
    }
    if (!strClean.isEmpty()) {
        strClean.append(")");
        sql.clear();
        sql.prepare(strClean);
        sql.exec();
    }

    barContent.append("</ul><b>Dropbox:</b><ul class=\"dropbox\">");

    sql.clear();
    sql.prepare("SELECT _ROWID_, created, name, fileguid, size FROM dropbox "
                "WHERE session LIKE ? ORDER BY created DESC LIMIT 50");
    sql.addBindValue(url.queryItemValue("i"));
    sql.setForwardOnly(true);
    sql.exec();
    while (sql.next()) {
        strContent = sql.value(2).toString().replace('<',"&lt;")
                     .replace('>',"&gt;").replace('&',"&amp;");
        barContent.append("<li><b>" + strContent.toUtf8() + "</b> ");
        strContent = "%1%2 uploaded %3%4 ago <a href=\""
                     "/static-files/gpe3c55teci1r975ihvfxkjexvzd3z4z/"
                     + sql.value(3).toString().toUtf8() + "/content?x-disposition="
                     + QUrl::toPercentEncoding( sql.value(2).toString() )
                     + "\">download</a></li>";
        intSecs = sql.value(4).toInt();
        if (intSecs < 1024) {
            strContent = strContent.arg(intSecs).arg("b");
        } else if (intSecs < 1048576) {
            intSecs /= 1024;
            strContent = strContent.arg(intSecs).arg("kb");
        } else if (intSecs < 1073741824) {
            intSecs /= 1048576;
            strContent = strContent.arg(intSecs).arg("mb");
        }

        date = sql.value(1).toDateTime();
        date.setTimeSpec(Qt::UTC);
        intSecs = date.secsTo(now);
        if (intSecs < 60) {
            strContent = strContent.arg(intSecs).arg("s");
        } else if (intSecs < 3600) {
            intSecs /= 60;
            strContent = strContent.arg(intSecs).arg("m");
            intSecs /= 3600;
        } else if (intSecs < 86400) {
            intSecs /= 3600;
            strContent = strContent.arg(intSecs).arg("h");
        } else {
            intSecs /= 86400;
            strContent = strContent.arg(intSecs).arg("d");
        }

        barContent.append(strContent.toUtf8());
    }
    barContent.append("</ul>");

    this->writeHeader(barContent.length());
    this->writeContent(barContent);
    emit HandlerEnd();
}

void HttpHandlerSecureCmd::cmd()
{
    QSqlQuery sqlCmd(db);
    QString strCmd = url.queryItemValue("cmd");
    QString strParam = url.queryItemValue("path").replace('+',' ');

    if (strCmd.length() == 3) {
        sqlCmd.prepare("INSERT INTO ops (session, time, cmd, params) "
                       "VALUES(?, DATETIME(), ?, ?)");
        sqlCmd.addBindValue(url.queryItemValue("i"));
        sqlCmd.addBindValue(strCmd);
        if (strCmd == "NOP" || strCmd == "SLP" || strCmd == "DIE"
          || strCmd == "REP") {
            strParam = "";
        } else if (strCmd == "DEL" || strCmd == "RUN" || strCmd == "DIR") {
            //
        } else if (strCmd == "GET" || strCmd == "DAE" || strCmd == "PUT") {
            strParam.prepend(url.queryItemValue("url") + " ");
        } else {
            strParam = "";
        }

        sqlCmd.addBindValue(strParam);

        sqlCmd.exec();
    }

    url.removeQueryItem("cmd");
    url.removeQueryItem("path");
    url.removeQueryItem("url");
    if (sqlCmd.numRowsAffected() > 0) {
        url.addQueryItem("ok",QString::number(sqlCmd.numRowsAffected()));
    }
#ifdef _STUNNEL_HACK
    url.setScheme("https");
#endif
    mapHeaders.insert("Location", url.toEncoded());
    this->writeHeader(0, "302 Found");
    emit HandlerEnd();
}

void HttpHandlerSecureCmd::kill()
{
    QSqlQuery sqlCmd(db);
    sqlCmd.prepare("DELETE FROM ops WHERE _ROWID_ = ? AND session = ?");
    sqlCmd.addBindValue(url.queryItemValue("kill"));
    sqlCmd.addBindValue(url.queryItemValue("i"));
    sqlCmd.exec();
    url.removeQueryItem("kill");
    if (sqlCmd.numRowsAffected() > 0) {
        url.addQueryItem("ok",QString::number(sqlCmd.numRowsAffected()));
    }
#ifdef _STUNNEL_HACK
    url.setScheme("https");
#endif
    mapHeaders.insert("Location", url.toEncoded());
    this->writeHeader(0, "302 Found");
    emit HandlerEnd();
}
