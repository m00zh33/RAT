#ifndef D_FILEIO_H_
#define D_FILEIO_H_

#include "EnRUPT.h"
#include <string>
#include <cstdio>
#include <windows.h>

#define ENC_BLOCK_SIZE (32)
#define MAX_U32        (sizeof(u32) * 256 - 1)

using namespace std;

wstring readEncryptedFile(const wstring& filePath, u32 *key);
bool writeEncryptedFile(const wstring& filePath, const wstring& fileContent, u32 *key);
bool appendEncryptedFileW(FILE *pFile, wstring &fileContent, u32 *key);
bool appendEncryptedFileC(FILE *pFile, wstring &fileContent, u32 *key);

string convertToString(wstring &source);
wstring getNextLine (wstring &wRemaining);

void getKeySession(u32 *outKey);

#endif
