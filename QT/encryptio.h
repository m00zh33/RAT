#ifndef ENCRYPTIO_H
#define ENCRYPTIO_H

#include "EnRUPT.h"
#include <QString>
#include <stdlib.h>

#define ENC_BLOCK_SIZE (32)

void getSessionKey (u32 *key);

QByteArray encryptString (QString input, u32 *key);
QString decryptString (QByteArray input, u32 *key);


#endif // ENCRYPTIO_H
