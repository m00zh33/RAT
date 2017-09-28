#ifndef ENC_H
#define ENC_H

#include <QtCore/QCoreApplication>
#include <QMutex>

void senc(QByteArray &data, quint16 start);
QByteArray mkdata(QByteArray data, quint8 slot);
bool dcdata(QByteArray &data, QByteArray &ret, quint8 &slot);
void rstinseq();

#endif
