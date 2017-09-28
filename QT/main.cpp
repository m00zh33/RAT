#include <QtCore/QCoreApplication>
#include <QtSql>
#include <iostream>
#include "https.h"

void test_dir();

int main(int argc, char *argv[])
{
    //QT_REQUIRE_VERSION (argc, argv, "4.7.0");

    QCoreApplication a(argc, argv);

    QLocale::setDefault(QLocale::English);

    srand(QDateTime().toTime_t());

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        std::cerr << "Error, SQLITE not loaded; Try:" << std::endl
                << "export QT_PLUGIN_PATH=/usr/local/Trolltech/Qt-4.7.0/plugins"
                << std::endl;
        exit(1);
    }

    {
        QDir maker;
        maker.mkdir(P_FOLDER_ROOT);
        maker.mkdir(P_FOLDER_SESSIONS);
        maker.mkdir(P_FOLDER_BITS_SESSIONS);
    }

    new https();

    return a.exec();
}
