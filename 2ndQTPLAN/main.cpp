#include <QtCore/QCoreApplication>

#include "enc.h"
#include "com.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    com *c = new com();
    c=c;
    return a.exec();
}
