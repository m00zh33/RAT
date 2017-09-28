#include "encryptio.h"

void getSessionKey (u32 *key)
{
    key[0] = 785243751u;
    key[1] = 4172533573u;
    key[2] = 2749264945u;
    key[3] = 3068397350u;
    key[5] = 3004706886u;
    key[6] = 20075361u;
    key[7] = 674264759u;
}

QByteArray encryptString (QString input, u32 *key)
{
    char         buffer[ENC_BLOCK_SIZE];
    const char   *pInput = (char *)input.utf16();
    size_t       intInputLength = input.length() * sizeof(ushort);
    QByteArray   barBuffer;
    size_t       i;

    forever {
        i = ENC_BLOCK_SIZE;
        if (intInputLength < ENC_BLOCK_SIZE) {
            i = intInputLength;
            memset(buffer, 0, ENC_BLOCK_SIZE);
        }
        memcpy ( buffer, pInput, i );
        enRUPT(buffer, ENC_BLOCK_SIZE, key, 8, i) //;
        barBuffer.append( buffer, ENC_BLOCK_SIZE );

        if (intInputLength <= ENC_BLOCK_SIZE) break;
        pInput += ENC_BLOCK_SIZE;
        intInputLength -= ENC_BLOCK_SIZE;
    }

    return barBuffer;
}

QString decryptString (QByteArray input, u32 *key)
{
    char       buffer[ENC_BLOCK_SIZE];
    const char *chrInput = input.constData();
    size_t     intInputLength = input.length();
    QString    strBuffer;
    size_t     i, j;

    forever {
        i = (intInputLength < ENC_BLOCK_SIZE ? intInputLength
            : ENC_BLOCK_SIZE);
        memcpy ( buffer, chrInput, i );

        unRUPT( buffer, ENC_BLOCK_SIZE, key, 8, j) //;

        strBuffer.append(QString::fromUtf16((ushort *) buffer,
                                            ENC_BLOCK_SIZE / sizeof(ushort)));

        if (intInputLength <= ENC_BLOCK_SIZE) break;
        chrInput += ENC_BLOCK_SIZE;
        intInputLength -= ENC_BLOCK_SIZE;
    }

    while (strBuffer.right(1) == '\0') {
        strBuffer.chop(1);
    }

    return strBuffer;
}
