#include "fileio.h"

void getKeySession(u32 *outKey)
{
  outKey[0] = 785243751u;
  outKey[1] = 4172533573u;
  outKey[2] = 2749264945u;
  outKey[3] = 3068397350u;
  char l1[256], l2[256];
  GetVolumeInformation("C:\\", l1,
    (DWORD)sizeof(l1), &outKey[4], 0, 0, l2,
    (DWORD)sizeof(l2));
  outKey[5] = 3004706886u;
  outKey[6] = 20075361u;
  outKey[7] = 674264759u;
}

wstring readEncryptedFile(const wstring& filePath, u32 *key)
{
  FILE *  pFile;
  char    buffer[ENC_BLOCK_SIZE];
  wstring wBuffer;
  size_t  i;
  
  _wfopen_s( &pFile, filePath.c_str(), L"rb" );

  if (pFile == NULL) return wstring(L"");

  while ( ! feof( pFile ) ) {
    fread ( buffer, sizeof(char), ENC_BLOCK_SIZE, pFile );
    unRUPT( buffer, ENC_BLOCK_SIZE, key, 8, i);
    wBuffer.append((wchar_t *)buffer, ENC_BLOCK_SIZE / sizeof(wchar_t));
  }

  wBuffer.resize(wBuffer.find_first_of(L'\0') + 1);

  fclose( pFile );

  return wBuffer;
}

bool writeEncryptedFile(const wstring& filePath, const wstring& fileContent, u32 *key)
{
  FILE * pFile;
  char buffer[ENC_BLOCK_SIZE];
  const wchar_t *pContent = fileContent.c_str();
  size_t intContentLength = fileContent.length() * sizeof(wchar_t);
  size_t i;

  _wfopen_s( &pFile, filePath.c_str(), L"wb" );

  if (pFile == NULL) return false;

  for (;;) {
    i = ENC_BLOCK_SIZE;
    if (intContentLength < ENC_BLOCK_SIZE) {
      i = intContentLength;
      memset( buffer, 0, ENC_BLOCK_SIZE );
    }
    memcpy ( buffer, pContent, i );

    enRUPT(buffer, ENC_BLOCK_SIZE, key, 8, i);

    if (fwrite( buffer, 1, ENC_BLOCK_SIZE, pFile ) < ENC_BLOCK_SIZE) {
      fclose ( pFile );
      return false;
    }


    if (intContentLength <= ENC_BLOCK_SIZE) break;
    pContent += ENC_BLOCK_SIZE;
    intContentLength -= ENC_BLOCK_SIZE;
  }

  fclose( pFile );

  return true;
}

bool appendEncryptedFileW(FILE *pFile, wstring &fileContent, u32 *key)
{
  char buffer[ENC_BLOCK_SIZE];
  const wchar_t *pContent = fileContent.c_str();
  size_t intContentLength = fileContent.length() * sizeof(wchar_t);
  size_t i;

  if (pFile == NULL) return false;
  if (intContentLength < ENC_BLOCK_SIZE) return true;  

  while (intContentLength >= ENC_BLOCK_SIZE) {
    i = ENC_BLOCK_SIZE;
    memcpy ( buffer, pContent, i );
    enRUPT(buffer, ENC_BLOCK_SIZE, key, 8, i);
    if (fwrite( buffer, 1, ENC_BLOCK_SIZE, pFile ) < ENC_BLOCK_SIZE) {
      fclose ( pFile );
      return false;
    }
    pContent += ENC_BLOCK_SIZE;
    intContentLength -= ENC_BLOCK_SIZE;
  }

  if (intContentLength > 0) {
    intContentLength /= sizeof(wchar_t);
    fileContent = fileContent.substr(fileContent.length() - intContentLength);
  }

  return true;
}

bool appendEncryptedFileC(FILE *pFile, wstring &fileContent, u32 *key)
{
  char buffer[ENC_BLOCK_SIZE];
  char *pContent, *pContentFree;
  size_t intContentLength;
  size_t i;
  int utf8split = 0;

  if (fileContent.length() > 1 && fileContent.at(0) == 0) {
    utf8split = 1;
  }

  if (pFile == NULL) return false;
  if (fileContent.length() - utf8split < ENC_BLOCK_SIZE) return true;  

  pContent = (char *) malloc(fileContent.length() * sizeof(wchar_t));
  pContentFree = pContent;

  intContentLength = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  fileContent.c_str() + utf8split,
    fileContent.length() - 1,
	  pContent,
	  fileContent.length() * sizeof(wchar_t),
	  NULL,
	  NULL
  );

  if (intContentLength < ENC_BLOCK_SIZE) {
    free(pContentFree);
    return true;
  }

  fileContent.clear();

  pContent += utf8split;

  while (intContentLength >= ENC_BLOCK_SIZE) {
    i = ENC_BLOCK_SIZE;
    memcpy ( buffer, pContent, i );
    enRUPT(buffer, ENC_BLOCK_SIZE, key, 8, i);
    if (fwrite( buffer, 1, ENC_BLOCK_SIZE, pFile ) < ENC_BLOCK_SIZE) {
      fclose ( pFile );
      return false;
    }
    pContent += ENC_BLOCK_SIZE;
    intContentLength -= ENC_BLOCK_SIZE;
  }

  if (intContentLength > 0) {
    wchar_t pContentRet[ENC_BLOCK_SIZE];

    utf8split = MultiByteToWideChar(
	    CP_UTF8,
	    0,
      pContent - 1,
      2,
	    pContentRet,
      _countof(pContentRet)
    );
    if (utf8split == 1) {
      pContent--;
      fileContent.append(L"\0");
    }
    intContentLength = MultiByteToWideChar(
	    CP_UTF8,
	    0,
      pContent,
      intContentLength,
      pContentRet,
	    ENC_BLOCK_SIZE
    );
    fileContent.append(pContentRet, intContentLength);
  }

  free (pContentFree);

  return true;
}

wstring getNextLine (wstring &wRemaining)
{
  wstring wReturn;
  wstring::size_type iPos1;
  iPos1 = wRemaining.find_first_of(L'\n');
  if (iPos1 == 0) {
    wRemaining.erase(0, 1);
    return wReturn;
  }
  if (iPos1 == wstring::npos) {
    wReturn = wRemaining;
    wRemaining.resize(0);
    return wReturn;
  }
  wReturn = wRemaining.substr( 0, iPos1 );
  wRemaining.erase( 0, iPos1 );
  return wReturn;
}

string convertToString(const wstring &source)
{
  int intContentLength;
  char *pContent = (char *) malloc(source.length() * sizeof(wchar_t));
  std::string ret;

  intContentLength = WideCharToMultiByte(
	  CP_UTF8,
	  0,
	  source.c_str(),
    source.length(),
	  pContent,
	  source.length() * sizeof(wchar_t),
	  NULL,
	  NULL
  );

  ret.append(pContent, intContentLength);

  free (pContent);

  return ret;
}